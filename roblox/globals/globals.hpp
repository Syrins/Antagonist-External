#pragma once

#include "../classes/classes.hpp"

#include <cstdint>
#include "../../utils/overlay/ckeybind/keybind.hpp"

struct globals
{
	static antagonist::roblox::instance_t datamodel;
	static antagonist::roblox::instance_t visualengine;
	static antagonist::roblox::instance_t players;
	static antagonist::roblox::instance_t camera;
	static int placeid;
//	static antagonist::roblox::instance_t camera;
	//static antagonist::roblox::instance_t pf;

	static bool esp;
	static bool box;
	static bool name_esp;
	static bool tracers;
	static bool healthinfo;
	static bool healthbar;
	static bool chams;

	static float box_color[3];
	static float name_color[3];
	static float tracers_color[3];

	static bool shake;
	static bool aimbot;
	static bool triggerbot;
	static bool sticky_aim;

	static bool prediction;
	static bool vsync;
	static bool streamproof;

	static bool autologin;

	static std::string game;

	static int aimpart;
	static int boxtype;

	static int x_offset;
	static int y_offset;

	static int max_dist;

	static float fov_roblox;

	static float smoothness_x;
	static float smoothness_y;

	static float prediction_x;
	static float prediction_y;

	static float shake_value;

	static float range;
	static float range_mult;

	static bool show_auth;

	static float sensitivity;
	static int fov;
	static bool fov_on;
	static bool disable_outside_fov;

	static bool healthcheck;
	static bool knock_check;
	static bool team_check;
	static bool team_check_esp;

	static CKeybind aimbotkey;
};