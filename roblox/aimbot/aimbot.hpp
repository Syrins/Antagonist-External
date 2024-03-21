#pragma once

#include "../classes/classes.hpp"

namespace antagonist
{
	namespace roblox
	{

		antagonist::roblox::instance_t get_closest_player_to_cursor();
		antagonist::roblox::vector2_t get_relative_player_to_pos(antagonist::roblox::vector2_t closest_player);
		void hook_aimbot();
	}
}