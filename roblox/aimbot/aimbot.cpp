#include "aimbot.hpp"

#include <random>
#include <Windows.h>
#include "../globals/globals.hpp"
#include "../../utils/xorstr/xorstr.hpp"

float calc_dist(antagonist::roblox::vector2_t first, antagonist::roblox::vector2_t sec)
{
	return sqrt((first.x - sec.x) * (first.x - sec.x) + (first.y - sec.y) * (first.y - sec.y));
}

antagonist::roblox::vector2_t add_vector_2(antagonist::roblox::vector2_t first, antagonist::roblox::vector2_t sec)
{
	return
	{
		first.x + sec.x,
		first.y + sec.y
	};
}

antagonist::roblox::vector3_t add_vector_3(antagonist::roblox::vector3_t first, antagonist::roblox::vector3_t sec)
{
	return
	{
		first.x + sec.x,
		first.y + sec.y,
		first.z + sec.z
	};
}

antagonist::roblox::vector3_t div_vector_3(antagonist::roblox::vector3_t first, antagonist::roblox::vector3_t sec)
{
	return
	{
		first.x / sec.x,
		first.y / sec.y,
		first.z / sec.z
	};
}

antagonist::roblox::vector2_t vector_sub(antagonist::roblox::vector2_t one, antagonist::roblox::vector2_t two)
{
	return { one.x - two.x, one.y - two.y };
}

antagonist::roblox::instance_t antagonist::roblox::get_closest_player_to_cursor()
{
	POINT cursor_point;
	GetCursorPos(&cursor_point);
	ScreenToClient(FindWindowA(0, XorStr("Roblox")), &cursor_point);

	vector2_t cursor =
	{
		static_cast<float>(cursor_point.x),
		static_cast<float>(cursor_point.y)
	};

	auto players = globals::players;

	antagonist::roblox::instance_t closest_player;

	int min_dist = 9e9;
	std::string teamname;

	for (auto player : players.children())
	{
		auto localplayer = players.get_local_player();
		if (player.self == localplayer.self)
			continue;

		auto character = player.get_model_instance();
		if (!character.self)
			continue;

		auto humanoid = character.find_first_child("Humanoid");
		if (!humanoid.self)
			continue;

		auto team = player.get_team();
		
		auto localplayer_team = localplayer.get_team();

		if (globals::team_check && (team.self == localplayer_team.self))
			continue;

		//if (humanoid.get_health() <= 0 && globals::healthcheck)
		//{
		//	continue;
		//}

		//if (humanoid.get_health() <= 4 && globals::knock_check)
		//{
		//	continue;
		//}

		antagonist::roblox::instance_t head;

		switch (globals::aimpart)
		{
		case 0:
			head = character.find_first_child("Head");
			break;
		case 1:
			head = character.find_first_child("UpperTorso");
			break;
		case 2:
			head = character.find_first_child("HumanoidRootPart");
			break;
		case 3:
			head = character.find_first_child("LowerTorso");
			break;
		}

		if (!head.self)
			continue;

		vector3_t head_pos_3d;

		if (globals::prediction)
		{
			vector3_t velocity = head.get_part_velocity();
			auto velocity_vec = div_vector_3(velocity, { globals::prediction_x, globals::prediction_y, globals::prediction_x });
			head_pos_3d = add_vector_3(head.get_part_pos(), velocity_vec);
		}

		else
		{
			head_pos_3d = head.get_part_pos();
		}

		//matrix3_t cframe = globals::camera.get_camera_cframe();
		//write<float>(cframe[2], 0.6);

		auto head_pos = antagonist::roblox::world_to_screen(head_pos_3d);

		float dist = calc_dist(head_pos, cursor);

		if ((dist < globals::fov) && (min_dist > dist))
		{
			closest_player = head;
			min_dist = dist;
		}
	}
	return closest_player;
}

antagonist::roblox::vector2_t antagonist::roblox::get_relative_player_to_pos(vector2_t closest_player)
{
	vector2_t ret = { 0, 0 };

	POINT cursor_point;
	GetCursorPos(&cursor_point);
	ScreenToClient(FindWindowA(0, XorStr("Roblox")), &cursor_point);
	vector2_t cursor_pos = { static_cast<float>(cursor_point.x),static_cast<float>(cursor_point.y) };

	ret.x = (closest_player.x - cursor_pos.x) * globals::sensitivity / globals::smoothness_x;
	ret.y = (closest_player.y - cursor_pos.y) * globals::sensitivity / globals::smoothness_y;

	return ret;
}

void run(antagonist::roblox::instance_t player)
{
	POINT cursor_pos;
	GetCursorPos(&cursor_pos);
	ScreenToClient(FindWindowA(0, "Roblox"), &cursor_pos);

	antagonist::roblox::vector3_t head_pos_3d;

	if (globals::prediction)
	{
		antagonist::roblox::vector3_t velocity = player.get_part_velocity();
		auto velocity_vec = div_vector_3(velocity, { globals::prediction_x, globals::prediction_y, globals::prediction_x });
		head_pos_3d = add_vector_3(player.get_part_pos(), velocity_vec);
	}
	else
	{
		head_pos_3d = player.get_part_pos();
	}

	auto head_pos = antagonist::roblox::world_to_screen(head_pos_3d);
	if (head_pos.x == -1)
	{
		return;
	}

	float dist = calc_dist(head_pos, { static_cast<float>(cursor_pos.x), static_cast<float>(cursor_pos.y) });

	antagonist::roblox::vector2_t relative = { 0, 0 };
	relative.x = (head_pos.x - cursor_pos.x) * globals::sensitivity / globals::smoothness_x;
	relative.y = (head_pos.y - cursor_pos.y) * globals::sensitivity / globals::smoothness_y;

	if (relative.x == -1 || relative.y == -1)
		return;

	if (globals::disable_outside_fov && (dist <= globals::fov))
	{
		INPUT input;
		input.mi.time = 0;
		input.type = INPUT_MOUSE;
		input.mi.mouseData = 0;
		input.mi.dx = (relative.x);
		input.mi.dy = (relative.y);
		input.mi.dwFlags = MOUSEEVENTF_MOVE;
		SendInput(1, &input, sizeof(input));
	}
	else
	{
		if (!globals::disable_outside_fov)
		{
			INPUT input;
			input.mi.time = 0;
			input.type = INPUT_MOUSE;
			input.mi.mouseData = 0;
			input.mi.dx = (relative.x);
			input.mi.dy = (relative.y);
			input.mi.dwFlags = MOUSEEVENTF_MOVE;
			SendInput(1, &input, sizeof(input));
		}
	}
}

antagonist::roblox::instance_t saved;
bool saveda = false;

void antagonist::roblox::hook_aimbot()
{
	while (true)
	{
		globals::aimbotkey.update();

		if (GetForegroundWindow() == FindWindowA(0, XorStr("Roblox")))
		{
			if (globals::aimbotkey.enabled && globals::aimbot)
			{
				antagonist::roblox::instance_t plr;

				if (globals::sticky_aim && saveda && saved.self != 0)
				{
					plr = saved;
				}
				else
				{
					plr = get_closest_player_to_cursor();
				}

				if (plr.self != 0 && plr.get_part_pos().y > 0.1f)
				{
					run(plr);
					saved = plr;
					saveda = true;
				}
				else
				{
					saveda = false;
				}
			}
			else
			{
				saveda = false;
			}
		}

		Sleep(1);
	}
}
