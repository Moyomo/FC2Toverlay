#include "Drawing.h"
#include "fc2.hpp"

LPCSTR Drawing::lpWindowName = "Overlay Performance";
ImVec2 Drawing::vWindowSize = { 350, 75 };

void Drawing::Draw()
{
	if (fc2::get_error() == FC2_TEAM_ERROR_NO_ERROR)
	{
		auto drawing = fc2::get_drawing();

		for (auto& i : drawing)
		{
			/**
			 * @brief get drawing canvas. this will allow us to draw in the background
			 */
			auto canvas = ImGui::GetBackgroundDrawList();

			/**
			 * @brief draw text. this would be better with ImGui::TextColored, but the current version of FC2KV does not support color text. once I add that, the example will be changed.
			 */
			if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_TEXT)
			{
				ImGui::Text("%s", i.text);
			}

			/**
			 * @brief draw line. nothing fancy going on here.
			 */
			else if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_LINE)
			{
				canvas->AddLine(
					ImVec2(i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT], i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP]),
					ImVec2(i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_RIGHT], i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_BOTTOM]),
					ImColor((int)i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_RED], i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_GREEN], i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_BLUE], i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA]),
					i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_THICKNESS]
				);
			}

			/**
			 * @brief draw boxes. worth noting that in imgui, imvec2 wants floating values, yet the dimensions are long. these should be casted.
			 */
			else if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_BOX || i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_BOX_FILLED)
			{
				const auto min = ImVec2(i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT], i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP]);
				const auto max = ImVec2(min.x + i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_RIGHT], min.y + i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_BOTTOM]);
				const auto clr = ImColor(static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_RED]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_GREEN]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_BLUE]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA]));

				if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_BOX)
				{
					canvas->AddRect(min, max, clr, static_cast<float>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_THICKNESS]));
				}
				else if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_BOX_FILLED)
				{
					canvas->AddRectFilled(min, max, clr, static_cast<float>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_THICKNESS]));
				}
			}
		}
	}

	ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Once);
	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGui::Begin(lpWindowName);
	{
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Text("Frametime: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	}
	ImGui::End();
}