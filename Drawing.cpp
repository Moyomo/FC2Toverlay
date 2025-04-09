#include "Drawing.hpp"
#include "UI.hpp"
#include "Config.hpp"

// define default values
std::chrono::steady_clock::time_point Drawing::errorTime = std::chrono::steady_clock::time_point();
bool Drawing::bDrawSettings = true;
ImGuiID Drawing::lastKeyLabelID = 0;
ImGuiKey Drawing::quitKey = ImGui_ImplWin32_KeyEventToImGuiKey(Config::iQuitKeycode, 0);

/**
 * @brief Check if settings window should get closed
 */
bool Drawing::IsSettingsWindowActive()
{
    return bDrawSettings == true;
}

/**
 * @brief Draw the settings window for the overlay
 */
void Drawing::DrawSettings()
{
    ImGui::SetNextWindowSize({ 240.0f, 200.0f }, ImGuiCond_Once);
    ImGui::SetNextWindowFocus();
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("Overlay settings", &bDrawSettings, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
    {
        // show Constellation connection status
        bool constellationConnected = Config::IsConstellationConnected();

        ImGui::Text("FC2 status:");
        ImGui::SameLine();
        if (constellationConnected)
            ImGui::TextColored({ 0.2f, 0.8f, 0.2f, 1.0f }, "Connected");
        else
        {
            ImGui::TextColored({ 0.9f, 0.1f, 0.0f, 1.0f }, "Not connected");
            ImGui::Text("Launch Constellation and restart\nthe overlay");
        }

        // draw overlay config options
        ImGui::Separator();
        if (!constellationConnected) ImGui::BeginDisabled();

        // streamproof setting
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Streamproof");
        ImGui::SameLine();
        HelpMarker("Don't show the overlay in all kinds of screen recordings (screenshots/streams/video capture)");
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 19.0f);
        ImGui::Checkbox("##Streamproof", &Config::bStreamProof);

        // target FPS setting
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Target FPS");
        ImGui::SameLine();
        HelpMarker("The amount of frames per second the overlay tries to run at");
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 90.0f);
        ImGui::PushItemWidth(90.0f);
        if (ImGui::InputInt("##Target FPS", &Config::iTargetFPS, 10, 50, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_ParseEmptyRefVal))
        {
            Config::iTargetFPS = std::min(1000, std::max(Config::iTargetFPS, 0));
            Config::targetFrametime = std::chrono::microseconds(Config::iTargetFPS == 0 ? 1 : 1000000 / Config::iTargetFPS);
        }

        // minimum random dimensions offset setting
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Size offset min");
        ImGui::SameLine();
        HelpMarker("The minimum value for the random size difference between the overlay and the target window. A negative offset will make the overlay larger than the target window");
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 90.0f);
        if (ImGui::InputInt("##Random dimensions min", &Config::iRandomOffsetMin, 1, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_ParseEmptyRefVal))
        {
            Config::iRandomOffsetMin = std::min(100, std::max(Config::iRandomOffsetMin, -100));
        }

        // maximum random dimensions offset setting
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Size offset max");
        ImGui::SameLine();
        HelpMarker("The maximum value for the random size difference between the overlay and the target window. A negative offset will make the overlay larger than the target window");
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 90.0f);
        if (ImGui::InputInt("##Random dimensions max", &Config::iRandomOffsetMax, 1, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_ParseEmptyRefVal))
        {
            Config::iRandomOffsetMax = std::min(100, std::max(Config::iRandomOffsetMax, -100));
        }
        ImGui::PopItemWidth();

        // custom overlay window name
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Window name");
        ImGui::SameLine();
        HelpMarker("Custom name for the overlay window and overlay window class");
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 110.0f);
        ImGui::SetNextItemWidth(110.0f);
        if (ImGui::InputText("##Window name", &Config::sWindowName, ImGuiInputTextFlags_CallbackCharFilter, Drawing::FilterChars))
        {
            // resize string if it's longer than 64 characters
            if (Config::sWindowName.length() > 64)
                Config::sWindowName.resize(64);
        }

        // custom quit hotkey
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Quit hotkey");
        ImGui::SameLine();
        HelpMarker("Custom hotkey for quickly closing the overlay (panic button)");
        if (Drawing::Hotkey("Quit hotkey", quitKey))
        {
            // translate the ImGuiKey to the matching virtual keycode and save it to the config
            Config::iQuitKeycode = Config::ImGuiKeyToVirtualKeycode(quitKey);
        }

        // autostart setting
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Autostart");
        ImGui::SameLine();
        HelpMarker("Skip this settings window in the future and directly launch the overlay with the saved config settings");
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 19.0f);
        ImGui::Checkbox("##Autostart", &Config::bAutostart);

        // debug setting
        if (!constellationConnected) ImGui::EndDisabled();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Debug mode");
        ImGui::SameLine();
        HelpMarker("Display additional information for debugging and troubleshooting in the settings window and on the overlay");
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 19.0f);
        ImGui::Checkbox("##Debug mode", &Config::bDebug);

        ImGui::Separator();
        if (!constellationConnected) ImGui::BeginDisabled();

        // config save and load buttons
        ImGui::Dummy({ 0.0f, 0.0f });
        if (ImGui::Button("Save config")) Config::SaveConfig();
        ImGui::SameLine();
        if (ImGui::Button("Load config")) Config::GetConfig();

        // button to start the overlay
        ImGui::Dummy({ 0.0f, 0.0f });
        ImGui::PushStyleColor(ImGuiCol_Button, { 0.25f, 0.21f, 0.47f, 1.0f });
        if (ImGui::Button("Start overlay"))
        {
            if (UI::SetTargetWindow())
            {
                Config::bCreateOverlay = true;
                Config::SetRandomDimensions();
                bDrawSettings = false;
            }
            else
                errorTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        }
        ImGui::PopStyleColor();

        // show error message when overlay creation fails
        if (std::chrono::steady_clock::now() < errorTime)
        {
            ImGui::Dummy({ 0.0f, 0.0f });
            ImGui::TextColored({ 0.9f, 0.1f, 0.0f, 1.0f }, "Can't find the target window!\nMake sure Constellation is\nfully calibrated and try again.");
        }
        if (!constellationConnected) ImGui::EndDisabled();

        // debug info
        if (Config::bDebug)
        {
            ImGui::Dummy({ 0.0f, 0.0f });
            ImGui::Separator();
            ImGui::TextColored({ 0.2f, 0.4f, 1.0f, 1.0f }, "Debug Info");
            ImGui::Text("Overlay version: 1.3.1"); // yes, this is stupid
            auto fc2tVersion = fc2::get_version();
            ImGui::Text("Used FC2T version: %i.%i", fc2tVersion.first, fc2tVersion.second);
            ImGui::Text("Current FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("UIAccess status: %d", (uint32_t)UI::dwUIAccessErr);
            ImGui::Text("Target handle: %d", (uint32_t)UI::hTargetWindow);
            ImGui::Text("Target process ID: %d", (uint32_t)UI::dTargetPID);
            auto frametime = Config::targetFrametime.count();
            ImGui::Text("Overlay target frametime:\n%llu microsecond%s", frametime, frametime == 1 ? "" : "s");
            ImGui::Text("Custom quit keycode: %d", Config::iQuitKeycode);
        }
    }
    ImGui::End();
}

/**
 * @brief Draw the overlay content
 */
void Drawing::DrawOverlay()
{
    if (fc2::get_error() == FC2_TEAM_ERROR_NO_ERROR)
    {
        // get drawing requests from FC2
        auto drawing = fc2::draw::get();

        // get the default font
        ImFont* font = ImGui::GetIO().Fonts->Fonts[0];

        // get drawing canvas to draw in the background
        const auto canvas = ImGui::GetBackgroundDrawList();

        // clip the drawing area to prevent drawing outside of the target window area
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        canvas->PushClipRect({ 0.0f - Config::iOffsetLeft, 0.0f - Config::iOffsetTop }, { displaySize.x + Config::iOffsetRight, displaySize.y + Config::iOffsetBottom });

        // loop over the drawing requests
        for (auto& [text, dimensions, style] : drawing)
        {
            // subtract random offsets from the drawing positions
            dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT] -= Config::iOffsetLeft;
            dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_RIGHT] -= Config::iOffsetLeft;
            dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP] -= Config::iOffsetTop;
            dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_BOTTOM] -= Config::iOffsetTop;

            // draw a drop shadow for the text and then the text itself on top of it
            if (style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_TEXT)
            {
                canvas->AddText(
                    font,
                    13.0f,
                    ImVec2(dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT] + 1.0f, dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP] + 1.0f),
                    ImColor(0, 0, 0, style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA]),
                    text
                );

                canvas->AddText(
                    font,
                    13.0f,
                    ImVec2(static_cast<float>(dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT]), static_cast<float>(dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP])),
                    ImColor(style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_RED], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_GREEN], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_BLUE], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA]),
                    text
                );
            }

            // draw a line
            else if (style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_LINE)
            {
                canvas->AddLine(
                    ImVec2(static_cast<float>(dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT]), static_cast<float>(dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP])),
                    ImVec2(static_cast<float>(dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_RIGHT]), static_cast<float>(dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_BOTTOM])),
                    ImColor(style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_RED], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_GREEN], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_BLUE], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA]),
                    static_cast<float>(style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_THICKNESS])
                );
            }

            // draw normal or filled boxes
            else if (style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_BOX || style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_BOX_FILLED)
            {
                const auto min = ImVec2(static_cast<float>(dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT]), static_cast<float>(dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP]));
                const auto max = ImVec2(min.x + dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_RIGHT] + Config::iOffsetLeft, min.y + dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_BOTTOM] + Config::iOffsetTop);
                const auto clr = ImColor(style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_RED], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_GREEN], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_BLUE], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA]);

                if (style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_BOX)
                {
                    canvas->AddRect(min, max, clr, NULL, NULL, static_cast<float>(style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_THICKNESS]));
                }
                else
                {
                    canvas->AddRectFilled(min, max, clr);
                }
            }

            // draw normal or filled circles
            else if (style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_CIRCLE || style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_CIRCLE_FILLED)
            {
                const auto pos = ImVec2(static_cast<float>(dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT]), static_cast<float>(dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP]));
                const auto clr = ImColor(style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_RED], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_GREEN], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_BLUE], style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA]);

                if (style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_CIRCLE)
                {
                    canvas->AddCircle(pos, static_cast<float>(style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_THICKNESS]), clr);
                }
                else
                {
                    canvas->AddCircleFilled(pos, static_cast<float>(style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_THICKNESS]), clr);
                }
            }
        }

        // remove the drawing area restriction
        canvas->PopClipRect();
    }

    if (Config::bDebug)
    {
        // draw red rectangle around overlay window client
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        auto canvas = ImGui::GetBackgroundDrawList();
        canvas->AddRect(ImVec2(0.0f, 0.0f), displaySize, ImColor(255, 0, 0, 180));

        // draw blue rectangle around target window client (if border is visible)
        canvas->AddRect(ImVec2(0.0f - Config::iOffsetLeft, 0.0f - Config::iOffsetTop), ImVec2(displaySize.x + Config::iOffsetRight, displaySize.y + Config::iOffsetBottom), ImColor(0, 0, 255, 180));

        // draw window with info about overlay performance
        ImGui::SetNextWindowSize({ 300.0f, 85.0f }, ImGuiCond_Once);
        ImGui::SetNextWindowPos({ 60.0f - Config::iOffsetLeft, 60.0f - Config::iOffsetTop }, ImGuiCond_Once);
        ImGui::SetNextWindowBgAlpha(0.6f);
        ImGui::Begin("Overlay debug window", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        {
            ImGui::Text("Frames per second: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Frametime: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
            ImGui::Text("Overlay size - X: %.0f Y: %.0f", displaySize.x, displaySize.y);
            ImGui::Text("Target window size - X: %.0f Y: %.0f", displaySize.x + Config::iOffsetLeft + Config::iOffsetRight, displaySize.y + Config::iOffsetTop + Config::iOffsetBottom);
            ImGui::Text("Offset Left: %d Offset Top: %d", Config::iOffsetLeft, Config::iOffsetTop);
            ImGui::Text("Offset Right: %d Offset Bottom: %d", Config::iOffsetRight, Config::iOffsetBottom);
        }
        ImGui::End();
    }
}

/**
 * @brief Filter input characters with specified rules
 * @param data ImGui callback data for the input text
 * @return 0 if the character is valid, otherwise 1
 */
int Drawing::FilterChars(ImGuiInputTextCallbackData* data)
{
    // check if character is a lowercase letter
    if (data->EventChar >= 0x61 && data->EventChar <= 0x7A)
        return 0;

    // check if character is an uppercase letter
    if (data->EventChar >= 0x41 && data->EventChar <= 0x5A)
        return 0;

    // check if character is a number
    if (data->EventChar >= 0x30 && data->EventChar <= 0x39)
        return 0;

    // check if character is a space
    if (data->EventChar == 0x20)
        return 0;

    return 1;
}

/**
 * @brief Draw question mark symbol that displays a help text when hovered
 * @param desc The description text that the help marker displays
 */
void Drawing::HelpMarker(const char* desc)
{
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

/**
 * @brief Button that lets you select a hotkey by pressing it
 * @param label Unique identifier of the button
 * @param selectedKey Variable that will contain the selected key
 * @return true if the hotkey was changed, otherwise false
 */
bool Drawing::Hotkey(const char* label, ImGuiKey& selectedKey)
{
    // get label id
    const ImGuiID labelID = ImGui::GetID(label);

    // check if button is currently waiting for user input
    bool canSetKey = lastKeyLabelID == labelID;

    // define text shown on the button
    std::string_view buttonText = canSetKey ? "..." : ImGui::IsNamedKey(selectedKey) ? ImGui::GetKeyName(selectedKey) : "Unknown";

    // calculate width of the button
    ImVec2 textSize = ImGui::CalcTextSize(buttonText.data());
    textSize.x += 10.0f;
    if (textSize.x < 50.0f) textSize.x = 50.0f;

    // Define button and click event
    ImGui::PushID(labelID);
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - textSize.x);
    if (ImGui::Button(buttonText.data(), { textSize.x, 19.0f }))
    {
        canSetKey = true;
        lastKeyLabelID = labelID;
    }
    ImGui::PopID();

    if (!canSetKey)
        return false;

    // check if the user clicked somewhere else
    if (!ImGui::IsItemHovered() && ImGui::GetIO().MouseClicked[0])
    {
        lastKeyLabelID = 0;
        return false;
    }

    // loop over all possible ImGuiKeys
    for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key = (ImGuiKey)(key + 1))
    {
        // check if current key is being pressed
        if (!ImGui::IsKeyDown(key))
            continue;

        // check if the keycode is valid and not a mouse click
        if (Config::ImGuiKeyToVirtualKeycode(key) < 3)
            continue;

        // save the new key
        selectedKey = key;

        lastKeyLabelID = 0;
        break;
    }

    return lastKeyLabelID == 0;
}