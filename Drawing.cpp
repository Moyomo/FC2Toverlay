#include "Drawing.hpp"

LPCSTR Drawing::lpWindowName = "Overlay Performance";
ImVec2 Drawing::vWindowSize = { 300, 85 };

void Drawing::Draw(BOOL bDebug)
{
    if (fc2::get_error() == FC2_TEAM_ERROR_NO_ERROR)
    {
        // get drawing requests from FC2
        auto drawing = fc2::get_drawing();

        // get the default font
        ImFont* font = ImGui::GetIO().Fonts->Fonts[0];

        // loop over the drawing requests
        for (auto& i : drawing)
        {
            // get drawing canvas to draw in the background
            auto canvas = ImGui::GetBackgroundDrawList();

            // draw a drop shadow for the text and then the text itself on top of it
            if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_TEXT)
            {
                canvas->AddText(
                    font,
                    13.0f,
                    ImVec2(i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT] + 1, i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP] + 1),
                    ImColor(0, 0, 0, static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA])),
                    i.text
                );

                canvas->AddText(
                    font,
                    13.0f,
                    ImVec2(i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT], i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP]),
                    ImColor(static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_RED]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_GREEN]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_BLUE]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA])),
                    i.text
                );
            }

            // draw a line
            else if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_LINE)
            {
                canvas->AddLine(
                    ImVec2(i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT], i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP]),
                    ImVec2(i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_RIGHT], i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_BOTTOM]),
                    ImColor((int)i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_RED], i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_GREEN], i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_BLUE], i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA]),
                    i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_THICKNESS]
                );
            }

            // draw normal or filled boxes
            else if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_BOX || i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_BOX_FILLED)
            {
                const auto min = ImVec2(i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT], i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP]);
                const auto max = ImVec2(min.x + i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_RIGHT], min.y + i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_BOTTOM]);
                const auto clr = ImColor(static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_RED]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_GREEN]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_BLUE]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA]));

                if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_BOX)
                {
                    canvas->AddRect(min, max, clr, NULL, NULL, static_cast<float>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_THICKNESS]));
                }
                else if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_BOX_FILLED)
                {
                    canvas->AddRectFilled(min, max, clr);
                }
            }
        }
    }

    if (bDebug)
    {
        // draw red rectangle around target window client
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        auto canvas = ImGui::GetBackgroundDrawList();
        canvas->AddRect(ImVec2(0, 0), displaySize, ImColor(255, 0, 0, 255));

        // draw window with info about overlay performance
        ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Once);
        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::Begin(lpWindowName);
        {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Frametime: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
            ImGui::Text("Display size - X: %.0f Y: %.0f", displaySize.x, displaySize.y);
        }
        ImGui::End();
    }
}