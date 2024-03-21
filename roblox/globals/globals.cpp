#include "globals.hpp"

antagonist::roblox::instance_t globals::datamodel{};
antagonist::roblox::instance_t globals::players{};
antagonist::roblox::instance_t globals::visualengine{};
antagonist::roblox::instance_t globals::camera{};
int globals::placeid = 0;
//antagonist::roblox::instance_t globals::camera{};
//antagonist::roblox::instance_t globals::pf{};

bool globals::sticky_aim = false;

bool globals::esp = false;
bool globals::box = false;
bool globals::name_esp = false;
bool globals::tracers = false;
bool globals::healthinfo = false;
bool globals::healthbar = true;
bool globals::chams = false;

float globals::box_color[3] = { 255, 255, 255 };
float globals::name_color[3] = { 255, 255, 255 };
float globals::tracers_color[3] = { 255, 255, 255 };

bool globals::prediction = false;
bool globals::vsync = true;
bool globals::streamproof = false;
bool globals::aimbot = false;
bool globals::autologin = false;
bool globals::shake = false;
bool globals::triggerbot = false;

std::string globals::game = "Universal";

int globals::aimpart = 0;
int globals::boxtype = 0;

int globals::x_offset = 0;
int globals::y_offset = 0;

int globals::max_dist = 10000;

float globals::fov_roblox = 70;

float globals::smoothness_x = 3;
float globals::smoothness_y = 3;

float globals::prediction_x = 3;
float globals::prediction_y = 3;

float globals::shake_value = 100;

float globals::range = 0;
float globals::range_mult = 0;

float globals::sensitivity = 0.5f;
int globals::fov = 100;
bool globals::fov_on = false;
bool globals::disable_outside_fov = false;
bool globals::show_auth = true;

bool globals::healthcheck = false;
bool globals::knock_check = false;
bool globals::team_check = false;
bool globals::team_check_esp = false;

CKeybind globals::aimbotkey{ "aimkey" };