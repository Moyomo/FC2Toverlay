#include "uiaccess.hpp"

void* kernel_module = LI_MODULE("Kernel32.dll").cached();
void* advapi_module = LI_MODULE("Advapi32.dll").cached();

static DWORD DuplicateWinloginToken(DWORD dwSessionId, DWORD dwDesiredAccess, PHANDLE phToken) {
	DWORD dwErr;
	PRIVILEGE_SET ps;

	ps.PrivilegeCount = 1;
	ps.Control = PRIVILEGE_SET_ALL_NECESSARY;

	if (!LI_FN(LookupPrivilegeValueW).in_cached(advapi_module)(NULL, SE_TCB_NAME, &ps.Privilege[0].Luid))
		return GetLastError();

	HANDLE hSnapshot = LI_FN(CreateToolhelp32Snapshot).in_cached(kernel_module)(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return GetLastError();

	BOOL bCont, bFound = FALSE;
	PROCESSENTRY32 pe;

	pe.dwSize = sizeof(pe);
	dwErr = ERROR_NOT_FOUND;

	for (bCont = LI_FN(Process32FirstW).in_cached(kernel_module)(hSnapshot, &pe); bCont; bCont = LI_FN(Process32NextW).in_cached(kernel_module)(hSnapshot, &pe)) {
		HANDLE hProcess;

		if (_tcsicmp(pe.szExeFile, TEXT("winlogon.exe")) != 0)
			continue;

		hProcess = LI_FN(OpenProcess).in_cached(kernel_module)(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
		if (!hProcess)
			continue;

		HANDLE hToken;
		DWORD dwRetLen, sid;

		if (LI_FN(OpenProcessToken).in_cached(advapi_module)(hProcess, TOKEN_QUERY | TOKEN_DUPLICATE, &hToken)) {
			BOOL fTcb;

			if (LI_FN(PrivilegeCheck).in_cached(advapi_module)(hToken, &ps, &fTcb) && fTcb) {
				if (LI_FN(GetTokenInformation).in_cached(advapi_module)(hToken, TokenSessionId, &sid, sizeof(sid), &dwRetLen) && sid == dwSessionId) {
					bFound = TRUE;
					if (LI_FN(DuplicateTokenEx).in_cached(advapi_module)(hToken, dwDesiredAccess, NULL, SecurityImpersonation, TokenImpersonation, phToken)) {
						dwErr = ERROR_SUCCESS;
					}
					else {
						dwErr = GetLastError();
					}
				}
			}
			LI_FN(CloseHandle).in_cached(kernel_module)(hToken);
		}
		LI_FN(CloseHandle).in_cached(kernel_module)(hProcess);

		if (bFound) break;
	}

	LI_FN(CloseHandle).in_cached(kernel_module)(hSnapshot);

	return dwErr;
}

static DWORD CreateUIAccessToken(PHANDLE phToken) {
	DWORD dwErr;
	HANDLE hTokenSelf;

	if (!LI_FN(OpenProcessToken).in_cached(advapi_module)(LI_FN(GetCurrentProcess).in_cached(kernel_module)(), TOKEN_QUERY | TOKEN_DUPLICATE, &hTokenSelf))
		return GetLastError();
	
	DWORD dwSessionId, dwRetLen;

	if (!LI_FN(GetTokenInformation).in_cached(advapi_module)(hTokenSelf, TokenSessionId, &dwSessionId, sizeof(dwSessionId), &dwRetLen)) {
		LI_FN(CloseHandle).in_cached(kernel_module)(hTokenSelf);
		return GetLastError();
	}

	HANDLE hTokenSystem;

	dwErr = DuplicateWinloginToken(dwSessionId, TOKEN_IMPERSONATE, &hTokenSystem);
	if (dwErr != ERROR_SUCCESS)
	{
		LI_FN(CloseHandle).in_cached(kernel_module)(hTokenSelf);
		return dwErr;
	}

	if (!LI_FN(SetThreadToken).in_cached(advapi_module)(NULL, hTokenSystem)) {
		LI_FN(CloseHandle).in_cached(kernel_module)(hTokenSystem);
		LI_FN(CloseHandle).in_cached(kernel_module)(hTokenSelf);
		return GetLastError();
	}

	if (!LI_FN(DuplicateTokenEx).in_cached(advapi_module)(hTokenSelf, TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_DEFAULT, NULL, SecurityAnonymous, TokenPrimary, phToken)) {
		LI_FN(RevertToSelf).in_cached(advapi_module)();
		LI_FN(CloseHandle).in_cached(kernel_module)(hTokenSystem);
		LI_FN(CloseHandle).in_cached(kernel_module)(hTokenSelf);
		return GetLastError();
	}

	BOOL bUIAccess = TRUE;

	if (!LI_FN(SetTokenInformation).in_cached(advapi_module)(*phToken, TokenUIAccess, &bUIAccess, sizeof(bUIAccess))) {
		dwErr = GetLastError();
		LI_FN(CloseHandle).in_cached(kernel_module)(*phToken);
	}

	LI_FN(RevertToSelf).in_cached(advapi_module)();
	LI_FN(CloseHandle).in_cached(kernel_module)(hTokenSystem);
	LI_FN(CloseHandle).in_cached(kernel_module)(hTokenSelf);

	return dwErr;
}

static BOOL CheckForUIAccess(DWORD* pdwErr, BOOL* pfUIAccess) {
	BOOL result = FALSE;
	HANDLE hToken;

	if (!LI_FN(OpenProcessToken).in_cached(advapi_module)(LI_FN(GetCurrentProcess).in_cached(kernel_module)(), TOKEN_QUERY, &hToken)) {
		*pdwErr = GetLastError();
		return FALSE;
	}

	DWORD dwRetLen;

	if (!LI_FN(GetTokenInformation).in_cached(advapi_module)(hToken, TokenUIAccess, pfUIAccess, sizeof(*pfUIAccess), &dwRetLen)) {
		*pdwErr = GetLastError();
		return FALSE;
	}

	LI_FN(CloseHandle).in_cached(kernel_module)(hToken);

	return TRUE;
}

DWORD PrepareForUIAccess() {
	DWORD dwErr;
	HANDLE hTokenUIAccess;
	BOOL fUIAccess;

	if (!CheckForUIAccess(&dwErr, &fUIAccess))
		return dwErr;

	if (fUIAccess)
		return ERROR_SUCCESS;

	dwErr = CreateUIAccessToken(&hTokenUIAccess);
	if (dwErr == ERROR_SUCCESS) {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		LI_FN(GetStartupInfoW).in_cached(kernel_module)(&si);
		if (CreateProcessAsUserW(hTokenUIAccess, NULL, LI_FN(GetCommandLineW).in_cached(kernel_module)(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
			LI_FN(CloseHandle).in_cached(kernel_module)(pi.hProcess), LI_FN(CloseHandle).in_cached(kernel_module)(pi.hThread);
			LI_FN(ExitProcess).in(kernel_module)(0);
		}
		else {
			dwErr = GetLastError();
		}

		LI_FN(CloseHandle).in_cached(kernel_module)(hTokenUIAccess);
	}

	return dwErr;
}