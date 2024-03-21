#define IMGUI_DEFINE_MATH_OPERATORS

#include "esp.hpp"

#include "../../utils/overlay/imgui/imgui.h"
#include "../globals/globals.hpp"

#include <Windows.h>
#include "../../utils/xorstr/xorstr.hpp"


float calc_dist_3d(antagonist::roblox::vector3_t first, antagonist::roblox::vector3_t sec)
{
	return sqrt((first.x - sec.x) * (first.x - sec.x) + (first.y - sec.y) * (first.y - sec.y) + (first.z - sec.z) * (first.z - sec.z));
}

antagonist::roblox::vector3_t vector_sub(antagonist::roblox::vector3_t one, antagonist::roblox::vector3_t two)
{
	return { one.x - two.x, one.y - two.y, one.z - two.z };
}

float vector3_mag(antagonist::roblox::vector3_t vector)
{
    return sqrtf((vector.x * vector.y) + (vector.y * vector.y) + (vector.z * vector.z));
}

void antagonist::roblox::hook_esp()
{
    auto players = globals::players;
    auto draw = ImGui::GetBackgroundDrawList();

    POINT cursor_pos;
    GetCursorPos(&cursor_pos);
    ScreenToClient(FindWindowA(0, "Roblox"), &cursor_pos);

    antagonist::roblox::vector3_t camerapos = globals::visualengine.get_camera_pos();

    if (globals::fov_on)
    {
        draw->AddCircle(ImVec2(cursor_pos.x, cursor_pos.y), globals::fov, ImColor(255, 255, 255));
    }

    if (globals::esp)
    {
        for (auto player : players.children())
        {
            // localplayer check
            auto localplayer = players.get_local_player();
            if (player.self == localplayer.self)
                continue;

            auto character = player.get_model_instance();
            if (!character.self)
                continue;

            auto humanoid = character.find_first_child("Humanoid");
            if (!humanoid.self)
                continue;

            auto head = character.find_first_child("Head");
            if (!head.self)
                continue;

            auto hrp = character.find_first_child("HumanoidRootPart");
            if (!hrp.self)
                continue;

            auto team = player.get_team();
           
            auto localplayer_team = localplayer.get_team();

            if (globals::team_check_esp && (team.self == localplayer_team.self))
                continue;

            if (humanoid.get_health() <= 0 && globals::healthcheck)
            {
                continue;
            }

            if (humanoid.get_health() <= 4 && globals::knock_check)
            {
                continue;
            }
            
            auto head_position_3d = head.get_part_pos();
            auto head_position = antagonist::roblox::world_to_screen(vector_sub(head_position_3d, { 0, 2, 0 }));

            auto hrp_pos = hrp.get_part_pos();

            auto leg_pos = antagonist::roblox::world_to_screen(vector_sub(head.get_part_pos(), { 0, 5, 0 }));
            if (leg_pos.x == -1)
                continue;

            float distance_from_camera = vector3_mag(vector_sub(camerapos, hrp_pos));

            auto headPosY = static_cast<float>(hrp_pos.y + 2.5);
            auto legPosY = static_cast<float>(hrp_pos.y - 3.5);

            auto top = antagonist::roblox::world_to_screen({ static_cast<float>(hrp_pos.x), headPosY, static_cast<float>(hrp_pos.z) });
            auto bottom = antagonist::roblox::world_to_screen({ static_cast<float>(hrp_pos.x), legPosY, static_cast<float>(hrp_pos.z) });

            float height = top.y - bottom.y;

            float width = (height / 2);
            width = width / 1.2;

            // tracers -> line
            if (globals::tracers)
            {
                draw->AddLine(ImVec2(head_position.x, head_position.y), ImVec2(static_cast<float>(cursor_pos.x), static_cast<float>(cursor_pos.y)), ImColor(255, 255, 255));
            }

            // name esp -> text
            if (globals::name_esp)
            {
                std::string playerName = player.name();

                for (int i = -1; i <= 1; ++i)
                {
                    for (int j = -1; j <= 1; ++j)
                    {
                        if (i != 0 || j != 0)
                        {
                            draw->AddText(ImVec2(ImVec2((top.x + bottom.x) / 2 - ImGui::CalcTextSize(playerName.c_str()).x / 2, top.y - 20).x + i, ImVec2((top.x + bottom.x) / 2 - ImGui::CalcTextSize(playerName.c_str()).x / 2, top.y - 20).y + j), ImColor(0, 0, 0, 255), playerName.c_str());
                        }
                    }
                }

                draw->AddText(ImVec2((top.x + bottom.x) / 2 - ImGui::CalcTextSize(playerName.c_str()).x / 2, top.y - 20), ImColor(255, 255, 255), playerName.c_str());
            }

            //health info -> text
            if (globals::healthinfo)
            {
                auto humanoid = character.find_first_child("Humanoid");
                if (humanoid.self)
                {
                    float health = humanoid.get_health();
                    int healthPercentage = static_cast<int>(health);

                    ImVec4 healthColor;

                    if (healthPercentage <= 100)
                        healthColor = ImColor(66, 245, 164);

                    if (healthPercentage <= 90)
                        healthColor = ImColor(66, 245, 111);

                    if (healthPercentage <= 70)
                        healthColor = ImColor(239, 245, 66);

                    if (healthPercentage <= 60)
                        healthColor = ImColor(245, 215, 66);

                    if (healthPercentage <= 50)
                        healthColor = ImColor(245, 176, 66);

                    if (healthPercentage <= 40)
                        healthColor = ImColor(245, 150, 66);

                    if (healthPercentage <= 30)
                        healthColor = ImColor(245, 123, 66);

                    if (healthPercentage <= 20)
                        healthColor = ImColor(245, 102, 66);

                    if (healthPercentage <= 10)
                        healthColor = ImColor(245, 66, 66);

                    std::string text = std::to_string(healthPercentage) + "%";

                    draw->AddText(ImVec2(head_position.x - width / 2, head_position.y + height + 2) + ImVec2(-1, -1), ImColor(0, 0, 0), text.c_str());
                    draw->AddText(ImVec2(head_position.x - width / 2, head_position.y + height + 2) + ImVec2(1, -1), ImColor(0, 0, 0), text.c_str());
                    draw->AddText(ImVec2(head_position.x - width / 2, head_position.y + height + 2) + ImVec2(-1, 1), ImColor(0, 0, 0), text.c_str());
                    draw->AddText(ImVec2(head_position.x - width / 2, head_position.y + height + 2) + ImVec2(1, 1), ImColor(0, 0, 0), text.c_str());

                    draw->AddText(ImVec2(head_position.x - width / 2, head_position.y + height + 2), (ImColor)healthColor, text.c_str());
                }
            }

            // box -> rectangle & circle (???)
            if (globals::box)
            {
                switch (globals::boxtype)
                {
                case 0:
                    draw->AddRect(ImVec2(bottom.x - width, top.y), ImVec2(top.x + width, bottom.y), ImColor(255, 255, 255));
                    break;
                case 1:
                {
                    // top left
                    draw->AddLine(ImVec2(top.x + width + 2, top.y), ImVec2(top.x + (width / 2) + 2, top.y), ImColor(255, 255, 255));
                    draw->AddLine(ImVec2(top.x + width + 2, top.y), ImVec2(top.x + width + 2, top.y - (height / 4)), ImColor(255, 255, 255));

                    // top right
                    draw->AddLine(ImVec2(bottom.x - width, top.y), ImVec2(bottom.x - (width / 2), top.y), ImColor(255, 255, 255));
                    draw->AddLine(ImVec2(bottom.x - width, top.y), ImVec2(bottom.x - width, top.y - (height / 4)), ImColor(255, 255, 255));

                    // bottom left
                    draw->AddLine(ImVec2(top.x + width + 2, bottom.y), ImVec2(top.x + (width / 2) + 2, bottom.y), ImColor(255, 255, 255));
                    draw->AddLine(ImVec2(top.x + width + 2, bottom.y), ImVec2(top.x + width + 2, bottom.y + (height / 4)), ImColor(255, 255, 255));

                    // bottom right
                    draw->AddLine(ImVec2(bottom.x - width, bottom.y), ImVec2(bottom.x - (width / 2), bottom.y), ImColor(255, 255, 255));
                    draw->AddLine(ImVec2(bottom.x - width, bottom.y), ImVec2(bottom.x - width, bottom.y + (height / 4)), ImColor(255, 255, 255));
                }
                    break;
                case 2:
                    break;
                }
            }
        }
    }
}