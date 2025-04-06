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
    ImGui::SetNextWindowSize({ 200, 200 }, ImGuiCond_Once);
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

        // streamproof setting
        ImGui::Checkbox("Streamproof", &Config::bStreamProof);

        // target FPS setting
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Target FPS");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
        if (ImGui::InputInt("##Target FPS", &Config::iTargetFPS, 10, 50, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_ParseEmptyRefVal))
        {
            Config::iTargetFPS = std::min(1000, std::max(Config::iTargetFPS, 0));
            Config::targetFrametime = std::chrono::microseconds(Config::iTargetFPS == 0 ? 1 : 1000000 / Config::iTargetFPS);
        }
        ImGui::PopItemWidth();

        // minimum random dimensions offset setting
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Random dimensions min");
        ImGui::SameLine();
        ImGui::PushItemWidth(90.0f);
        if (ImGui::InputInt("##Random dimensions min", &Config::iRandomOffsetMin, 1, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_ParseEmptyRefVal))
        {
            Config::iRandomOffsetMin = std::min(100, std::max(Config::iRandomOffsetMin, -100));
        }

        // maximum random dimensions offset setting
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Random dimensions max");
        ImGui::SameLine();
        if (ImGui::InputInt("##Random dimensions max", &Config::iRandomOffsetMax, 1, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_ParseEmptyRefVal))
        {
            Config::iRandomOffsetMax = std::min(100, std::max(Config::iRandomOffsetMax, -100));
        }
        ImGui::PopItemWidth();

        // autostart and debug settings
        ImGui::Checkbox("Autostart (skip this window)", &Config::bAutostart);
        if (!constellationConnected) ImGui::EndDisabled();
        ImGui::Checkbox("Debug mode", &Config::bDebug);

        ImGui::Separator();
        if (!constellationConnected) ImGui::BeginDisabled();

        // config save and load buttons
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
                Config::SetRandomDimensions();
                bDraw = false;
            }
            else
                errorTime = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        }

        // show error message when overlay creation fails
        if (std::chrono::steady_clock::now() < errorTime)
            ImGui::TextColored({ 0.9f, 0.1f, 0.0f, 1.0f }, "Can't find the target window! Make sure\nConstellation is fully calibrated and\ntry again.");
        if (!constellationConnected) ImGui::EndDisabled();

        // debug info
        if (Config::bDebug)
        {
            ImGui::Separator();
            ImGui::TextColored({ 0.2f, 0.4f, 1.0f, 1.0f }, "Debug Info");
            ImGui::Text("Overlay version: 1.3"); // yes, this is stupid
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

        // get drawing canvas to draw in the background
        const auto canvas = ImGui::GetBackgroundDrawList();

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