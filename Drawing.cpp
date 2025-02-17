#include "Drawing.hpp"
#include "UI.hpp"
#include "Config.hpp"

// define default values
std::chrono::steady_clock::time_point Drawing::errorTime = std::chrono::steady_clock::time_point();
bool Drawing::bDraw = true;

/**
 * @brief Check if window should get closed
 */
bool Drawing::IsActive()
{
    return bDraw == true;
}

/**
 * @brief Draw the settings window for the overlay
 */
void Drawing::DrawSettings()
{
    ImGui::SetNextWindowSize({ 300, 200 }, ImGuiCond_Once);
    ImGui::SetNextWindowFocus();
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::Begin("Overlay settings", &bDraw, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
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
            ImGui::Text("Launch Constellation and restart the overlay");
        }

        // draw overlay config options
        ImGui::Separator();
        if (!constellationConnected) ImGui::BeginDisabled();

        ImGui::Checkbox("Streamproof", &Config::bStreamProof);

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Target FPS");
        ImGui::SameLine();
        if (ImGui::InputInt(" ", &Config::iTargetFPS, 10, 50, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_ParseEmptyRefVal))
        {
            Config::iTargetFPS = std::min(1000, std::max(Config::iTargetFPS, 0));
            Config::targetFrametime = std::chrono::microseconds(Config::iTargetFPS == 0 ? 1 : 1000000 / Config::iTargetFPS);
        }
        ImGui::Checkbox("Autostart (skip this window)", &Config::bAutostart);
        if (!constellationConnected) ImGui::EndDisabled();
        ImGui::Checkbox("Debug mode", &Config::bDebug);

        ImGui::Separator();
        if (!constellationConnected) ImGui::BeginDisabled();

        // config buttons
        if (ImGui::Button("Save config")) Config::SaveConfig();
        ImGui::SameLine();
        if (ImGui::Button("Load config")) Config::GetConfig();

        // button to start the overlay
        ImGui::Dummy({ 0.0f, 1.0f });
        if (ImGui::Button("Start overlay"))
        {
            if (UI::SetTargetWindow())
            {
                Config::bCreateOverlay = true;
                bDraw = false;
            }
            else
                errorTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        }

        // show error message when overlay creation fails
        if (std::chrono::steady_clock::now() < errorTime)
            ImGui::TextColored({ 0.9, 0.1, 0.0, 1.0 }, "Can't find the target window! Make sure\nConstellation is fully calibrated\nand try again.");
        if (!constellationConnected) ImGui::EndDisabled();

        // debug info
        if (Config::bDebug)
        {
            ImGui::Separator();
            ImGui::TextColored({ 0.2, 0.4, 1.0, 1.0 }, "Debug Info");
            ImGui::Text("Overlay version: 1.2.2"); // yes, this is stupid
            auto fc2tVersion = fc2::get_version();
            ImGui::Text("Used FC2T version: %i.%i", fc2tVersion.first, fc2tVersion.second);
            ImGui::Text("Current FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("UIAccess status: %d", (uint32_t)UI::dwUIAccessErr);
            ImGui::Text("Target handle: %d", (uint32_t)UI::hTargetWindow);
            ImGui::Text("Target process ID: %d", (uint32_t)UI::dTargetPID);
            auto frametime = Config::targetFrametime.count();
            ImGui::Text("Overlay target frametime:\n%llu microseconds", frametime);
        }
    }
    ImGui::End();
}

/**
 * @brief Draw the overlay content
 */
void Drawing::Draw()
{
    if (fc2::get_error() == FC2_TEAM_ERROR_NO_ERROR)
    {
        // get drawing requests from FC2
        auto drawing = fc2::draw::get();

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
                else
                {
                    canvas->AddRectFilled(min, max, clr);
                }
            }

            // draw normal or filled circles
            else if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_CIRCLE || i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_CIRCLE_FILLED)
            {
                const auto pos = ImVec2(i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_LEFT], i.dimensions[FC2_TEAM_DRAW_DIMENSIONS::FC2_TEAM_DRAW_DIMENSIONS_TOP]);
                const auto clr = ImColor(static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_RED]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_GREEN]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_BLUE]), static_cast<int>(i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_ALPHA]));

                if (i.style[FC2_TEAM_DRAW_STYLE_TYPE] == FC2_TEAM_DRAW_TYPE_CIRCLE)
                {
                    canvas->AddCircle(pos, i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_THICKNESS], clr);
                }
                else
                {
                    canvas->AddCircleFilled(pos, i.style[FC2_TEAM_DRAW_STYLE::FC2_TEAM_DRAW_STYLE_THICKNESS], clr);
                }
            }
        }
    }

    if (Config::bDebug)
    {
        // draw red rectangle around target window client
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        auto canvas = ImGui::GetBackgroundDrawList();
        canvas->AddRect(ImVec2(0, 0), displaySize, ImColor(255, 0, 0, 255));

        // draw window with info about overlay performance
        ImGui::SetNextWindowSize({ 300, 85 }, ImGuiCond_Once);
        ImGui::SetNextWindowBgAlpha(0.7f);
        ImGui::Begin("Overlay Performance", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Frametime: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
            ImGui::Text("Display size - X: %.0f Y: %.0f", displaySize.x, displaySize.y);
        }
        ImGui::End();
    }
}