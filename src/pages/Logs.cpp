#include <Geode/Geode.hpp>
#include "../DevTools.hpp"
#include "../ImGui.hpp"

using namespace geode::prelude;

void DevTools::drawLogs() {

    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 win_size = ImGui::GetWindowSize();
    ImGui::GetWindowDrawList()->AddRectFilled(
        win_pos,
        ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y),
        IM_COL32(0, 0, 0, 255)
    );

    static auto lines = string::split("no logs..?", "a");
    static uintmax_t size;

    std::error_code err;
    uintmax_t current = std::filesystem::file_size(log::getCurrentLogPath(), err);
    if (size != current) {
        size = current;
        auto read = file::readString(log::getCurrentLogPath());
        lines = string::split(read.err().value_or(read.unwrapOrDefault()), "\n");
    }

    static ImVec2 lastcpos;
    static auto col = ImVec4{ 0.9f, 0.9f, 0.9f, 0.9f };
    static auto lcol = ImVec4{ 1.f,1.f,1.f,0.9f };
    for (auto line : lines) {
        if (std::string(line.c_str()).size() < 1) continue;
        if (!string::contains(line.c_str(), "]:")) {
            ImGui::SetCursorPosX(lastcpos.x);

            lastcpos = {
                ImGui::GetCursorPosX() < lastcpos.x ? lastcpos.x : ImGui::GetCursorPosX(),
                ImGui::GetCursorPosY()
            };

            ImGui::PushStyleColor(ImGuiCol_Text, col);
            auto p2overlay = false;
        render_textP2__:
            ImGui::TextWrapped("%s", line.c_str());
            ImGui::PopStyleColor();
            if (!p2overlay) {
                ImGui::SetCursorPos(lastcpos);
                ImGui::PushStyleColor(ImGuiCol_Text, lcol);
                p2overlay = true;
                goto render_textP2__;
            }

            continue;
        }
        auto parts = string::split(line, "[");
        std::string p1 = parts[0].c_str();

        if (string::contains(p1, "DEBUG"))  col = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };
        if (string::contains(p1, "INFO"))   col = ImVec4{ 0.5f, 1.0f, 1.0f, 1.0f };
        if (string::contains(p1, "WARN"))   col = ImVec4{ 1.0f, 1.0f, 0.65f, 1.0f };
        if (string::contains(p1, "ERROR"))  col = ImVec4{ 1.0f, 0.3f, 0.3f, 1.0f };

        lcol = ImVec4{ 1.0f, 1.0f, 1.0f, 0.86f };
        if (string::contains(p1, "DEBUG"))  lcol = ImVec4{ 1.f,1.f,1.f, 0.35f };

        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextWrapped("%s", std::string(p1.begin(), p1.end() - 1).c_str());
        ImGui::PopStyleColor();

        ImGui::SameLine();
        lastcpos = { 
            ImGui::GetCursorPosX() < lastcpos.x ? lastcpos.x : ImGui::GetCursorPosX(), 
            ImGui::GetCursorPosY() 
        };
        ImGui::SetCursorPos(lastcpos);

        ImGui::PushStyleColor(
            ImGuiCol_Text, string::contains(p1, "INFO") ? ImVec4{ 1.f, 1.f, 1.f, 0.35f } : col
        );
        auto p2overlay = false;
    render_textP2:
        ImGui::TextWrapped("[%s", string::join({ parts.begin() + 1, parts.end() }, "[").c_str());
        ImGui::PopStyleColor();
        if (!p2overlay) {
            ImGui::SetCursorPos(lastcpos);
            ImGui::PushStyleColor(ImGuiCol_Text, lcol);
            p2overlay = true;
            goto render_textP2;
        }
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(0.1f);

}
