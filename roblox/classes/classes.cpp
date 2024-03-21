#include "classes.hpp"

#include "../../utils/datamodel/datamodel.hpp"
#include "../../utils/overlay/overlay.hpp"
#include "../driver/driver_impl.hpp"
#include "../globals/globals.hpp"
#include "../aimbot/aimbot.hpp"

#include "../../mapper/kernel_ctx/kernel_ctx.h"
#include "../../mapper/drv_image/drv_image.h"
#include "../../mapper/raw_driver.hpp"
#include "../../mapper/driver_data.hpp"

#include "../../utils/xorstr/xorstr.hpp"
#include "../../utils/json/json.hpp"

#include "curl/curl.h"

#include <fstream>
#include <thread>
#include <numbers>
#include <unordered_set>

//below gpt to get roblox ver

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
	size_t totalSize = size * nmemb;
	output->append(static_cast<char*>(contents), totalSize);
	return totalSize;
}

std::string GetRobloxVersion() {
	CURL* curl;
	CURLcode res;

	// Initialize libcurl
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	std::string response;

	if (curl) {
		// Set the URL
		curl_easy_setopt(curl, CURLOPT_URL, XorStr("https://clientsettingscdn.roblox.com/v2/client-version/WindowsPlayer"));

		// Set the callback function to handle the response
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		// Perform the request
		res = curl_easy_perform(curl);

		// Check for errors
		if (res != CURLE_OK) {
			fprintf(stderr, XorStr("curl_easy_perform() failed: %s\n"), curl_easy_strerror(res));
		}

		// Cleanup
		curl_easy_cleanup(curl);
	}

	// Cleanup libcurl
	curl_global_cleanup();

	if (res != CURLE_OK) {
		return "";
	}

	// Parse JSON response
	auto json = nlohmann::json::parse(response);
	return json["clientVersionUpload"];
}

bool IsVersionUpToDate() {
	// Hardcoded Roblox version
	std::string githubVersion = XorStr("version-17f2f3d7fcfc48f5");

	// Get the Roblox version
	std::string robloxVersion = GetRobloxVersion();

	// Compare Roblox versions
	if (robloxVersion != githubVersion) {
		return false;
	}

	return true;
}

// above all gpt

std::uint64_t get_render_view()
{
	auto latest_log = antagonist::utils::log::get_latest_log();

	std::ifstream rbx_log(latest_log);
	std::string rbx_log_line;

	while (true)
	{
		std::getline(rbx_log, rbx_log_line);
		if (rbx_log_line.contains(XorStr("initialize view(")))
		{
			rbx_log_line = rbx_log_line.substr(rbx_log_line.find(XorStr("initialize view(")) + 21);
			rbx_log_line = rbx_log_line.substr(0, rbx_log_line.find(')'));

			std::uint64_t renderview = std::strtoull(rbx_log_line.c_str(), nullptr, 16);
			return renderview;
		}
	}
}

std::uint64_t get_visualengine_address()
{
	auto render_view = get_render_view();
	auto visualengine =
		read<std::uint64_t>(render_view + 0x10);

	return visualengine;
}

std::unordered_set<std::string> processedBalls;

bool antagonist::roblox::init()
{
	if (!IsVersionUpToDate()) {
		MessageBox(NULL, XorStr("antagonist is not updated, please wait for an update"), XorStr("discord.gg/antagonistrbx"), MB_OK | MB_ICONWARNING);
		return false;
	}

	std::vector<std::uint8_t> drv_buffer(driverlol, raw_driver + sizeof(raw_driver));

	if (!drv_buffer.size())
	{
		std::perror(XorStr("[debug]: driver: invalid drv_buffer size [contact support : discord.gg/antagonistrbx]\n"));
		return -1;
	}

	

	physmeme::drv_image image(drv_buffer);
	if (!physmeme::load_drv())
	{
		std::perror(XorStr("[debug]: driver: unable to load driver [contact support : discord.gg/antagonistrbx]\n"));
		return -1;
	}

	physmeme::kernel_ctx kernel_ctx;
	//std::printf("[+] driver has been loaded...\n");
	//std::printf("[+] %s mapped physical page -> 0x%p\n", physmeme::syscall_hook.first, physmeme::psyscall_func.load());
	//std::printf("[+] %s page offset -> 0x%x\n", physmeme::syscall_hook.first, physmeme::nt_page_offset);

	//if (!kernel_ctx.clear_piddb_cache(physmeme::drv_key, drv_timestamp))
	//{
		// this is because the signature might be broken on these versions of windows.
		//perror(XorStr("[debug]: driver: failed to clear PiDDBCacheTable [dont attempt to run this on windows 11]\n"));
		//return -1;
	//}

	const auto _get_export_name = [&](const char* base, const char* name)
		{
			return reinterpret_cast<std::uintptr_t>(util::get_kernel_export(base, name));
		};

	image.fix_imports(_get_export_name);
	image.map();

	const auto pool_base = kernel_ctx.allocate_pool(image.size(), NonPagedPool);
	image.relocate(pool_base);
	kernel_ctx.write_kernel(pool_base, image.data(), image.size());
	auto entry_point = reinterpret_cast<std::uintptr_t>(pool_base) + image.entry_point();

	auto result = kernel_ctx.syscall<DRIVER_INITIALIZE>
		(
			reinterpret_cast<void*>(entry_point),
			reinterpret_cast<std::uintptr_t>(pool_base),
			image.size()
		);

	std::printf(XorStr("[debug]: driver: entry returned: 0x%p\n"), result);
	//system("pause");
	kernel_ctx.zero_kernel_memory(pool_base, image.header_size());
	if (!physmeme::unload_drv())
	{
		std::perror(XorStr("[debug]: driver: unable to unload driver... all handles closed? [contact support : discord.gg/antagonistrbx]\n"));
		return -1;
	}

	printf(XorStr("[debug]: initializing\n"));

	if (!mem::find_driver())
	{
		printf(XorStr("[debug]: driver: driver is not loaded\n"));
		return false;
	}

	mem::process_id = mem::find_process(XorStr("RobloxPlayerBeta.exe"));
	if (!mem::process_id)
	{
		printf(XorStr("[debug]: roblox is not initialized\n"));
		return false;
	}

	//virtualaddy = mem::find_image();
	//antagonist::utils::log::debug_log(virtualaddy, XorStr("image base"));

	auto game =
		static_cast<antagonist::roblox::instance_t>(antagonist::utils::datamodel::get_game_address());

	if (!game.self)
	{
		printf(XorStr("[debug]: failed to get datamodel [contact support : discord.gg/antagonistrbx]\n"));
		return false;
	}

	globals::datamodel = game;

	auto players =
		game.find_first_child_of_class(XorStr("Players"));

	if (!players.self)
	{
		printf(XorStr("[debug]: failed to get players [contact support : discord.gg/antagonistrbx]\n"));
		return false;
	}

	globals::players = players;

	auto visualengine =
		static_cast<antagonist::roblox::instance_t>(get_visualengine_address());

	if (!visualengine.self)
	{
		printf(XorStr("[error]: failed to get visual engine [contact support : discord.gg/antagonistrbx]\n"));
		return false;
	}

	globals::visualengine = visualengine;

	std::string teamname;

	antagonist::roblox::instance_t team = players.get_local_player().get_team();

	if (team.self)
	{
		teamname = team.name();
	}
	else
	{
		teamname = XorStr("none");
	}

	auto placeid = antagonist::utils::datamodel::get_place_id();

	if (placeid == 0)
		return false;

	globals::placeid = placeid;

	auto camera = game.find_first_child_of_class("Workspace").find_first_child_of_class("Camera");
	if (!camera.self)
		printf("camera not found kys\n");

	globals::camera = camera;

	std::thread(antagonist::roblox::hook_aimbot).detach();
	std::thread(antagonist::utils::overlay::render).detach();

	printf(XorStr("[debug]: console will be hidden in 3 seconds...\n"));
	std::this_thread::sleep_for(std::chrono::seconds(3));

	ShowWindow(GetConsoleWindow(), SW_HIDE);

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(10));

		if (FindWindowA(NULL, XorStr("Roblox")) == NULL)
		{
			physmeme::unload_drv();
			exit(0);
		}
	}

	return true;
}

float antagonist::roblox::instance_t::fov()
{
	return read<float>(this->self + 0x288 - 4) * 180 / std::numbers::pi_v<float>;
}

void antagonist::roblox::instance_t::set_fov(float nigger)
{
	float new_fov = nigger * std::numbers::pi_v<float> / 180;
	write<float>(this->self + 0x288 - 4, new_fov);
}

antagonist::roblox::vector2_t antagonist::roblox::world_to_screen(antagonist::roblox::vector3_t world)
{
	auto dimensions = globals::visualengine.get_dimensions();
	auto viewmatrix = globals::visualengine.get_view_matrix();


	antagonist::roblox::quaternion quaternion;
	quaternion.x = (world.x * viewmatrix.data[0]) + (world.y * viewmatrix.data[1]) + (world.z * viewmatrix.data[2]) + viewmatrix.data[3];
	quaternion.y = (world.x * viewmatrix.data[4]) + (world.y * viewmatrix.data[5]) + (world.z * viewmatrix.data[6]) + viewmatrix.data[7];
	quaternion.z = (world.x * viewmatrix.data[8]) + (world.y * viewmatrix.data[9]) + (world.z * viewmatrix.data[10]) + viewmatrix.data[11];
	quaternion.w = (world.x * viewmatrix.data[12]) + (world.y * viewmatrix.data[13]) + (world.z * viewmatrix.data[14]) + viewmatrix.data[15];

	if (quaternion.w < 0.1f)
		return{ -1, -1 };

	vector3_t ndc;
	ndc.x = quaternion.x / quaternion.w;
	ndc.y = quaternion.y / quaternion.w;
	ndc.z = quaternion.z / quaternion.w;

	return
	{
		(dimensions.x / 2 * ndc.x) + (ndc.x + dimensions.x / 2),
		-(dimensions.y / 2 * ndc.y) + (ndc.y + dimensions.y / 2)
	};
}

std::string readstring(std::uintptr_t address)
{
	const auto size = read<size_t>(address + 0x10);

	if (size >= 16)
		address = read<std::uintptr_t>(address);

	std::string str;

	BYTE c = 0;

	for (std::int32_t i = 0; c = read<std::uint8_t>(address + i); i++)
		str.push_back(c);

	return str;
}

std::string antagonist::roblox::instance_t::name()
{
	const auto ptr = read<std::uint64_t>(this->self + antagonist::offsets::name);

	if (ptr)
		return readstring(ptr);

	return XorStr("???");
}

std::string antagonist::roblox::instance_t::class_name()
{
	const auto ptr = read<std::uint64_t>(this->self + antagonist::offsets::classname);
	auto ptr2 = read<std::uint64_t>(ptr + 0x8);

	if (ptr)
		return readstring(ptr2);

	return XorStr("???_classname");
}

std::vector<antagonist::roblox::instance_t> antagonist::roblox::instance_t::children()
{
	std::vector<antagonist::roblox::instance_t> container;

	if (!this->self)
		return container;

	auto start = read<std::uint64_t>(this->self + antagonist::offsets::children);

	if (!start)
		return container;

	auto end = read<std::uint64_t>(start + offsets::size);

	for (auto instances = read<std::uint64_t>(start); instances != end; instances += 16)
		container.emplace_back(read<antagonist::roblox::instance_t>(instances));

	return container;
}

antagonist::roblox::instance_t antagonist::roblox::instance_t::find_first_child(std::string child)
{
	antagonist::roblox::instance_t ret;

	for (auto &object : this->children())
	{
		if (object.name() == child)
		{
			ret = static_cast<antagonist::roblox::instance_t>(object);
		}
	}

	return ret;
}

antagonist::roblox::instance_t antagonist::roblox::instance_t::find_first_child_of_class(std::string child)
{
	antagonist::roblox::instance_t ret;

	for (auto& object : this->children())
	{
		if (object.class_name() == child)
		{
			ret = static_cast<antagonist::roblox::instance_t>(object);
		}
	}

	return ret;
}

antagonist::roblox::instance_t antagonist::roblox::instance_t::get_local_player()
{
	auto local_player = read<antagonist::roblox::instance_t>(this->self + antagonist::offsets::local_player);
	return local_player;
}

antagonist::roblox::instance_t antagonist::roblox::instance_t::get_model_instance()
{
	auto character = read<antagonist::roblox::instance_t>(this->self + antagonist::offsets::model_instance);
	return character;
}

antagonist::roblox::vector2_t antagonist::roblox::instance_t::get_dimensions()
{
	auto dimensions = read<antagonist::roblox::vector2_t>(this->self + offsets::dimensions);
	return dimensions;
}

antagonist::roblox::matrix4_t antagonist::roblox::instance_t::get_view_matrix()
{
	auto dimensions = read<antagonist::roblox::matrix4_t>(this->self + offsets::viewmatrix);
	return dimensions;
}

antagonist::roblox::vector3_t antagonist::roblox::instance_t::get_camera_pos()
{
	auto camera_pos = read<antagonist::roblox::vector3_t>(this->self + offsets::camera_pos);
	return camera_pos;
}

antagonist::roblox::vector3_t antagonist::roblox::instance_t::get_part_pos()
{
	vector3_t res{};

	auto primitive = read<std::uint64_t>(this->self + offsets::primitive);

	if (!primitive)
		return res;

	res = read<antagonist::roblox::vector3_t>(primitive + offsets::position + 8);
	return res;
}

antagonist::roblox::vector3_t antagonist::roblox::instance_t::get_part_velocity()
{
	vector3_t res{};

	auto primitive = read<std::uint64_t>(this->self + offsets::primitive);

	if (!primitive)
		return res;

	res = read<antagonist::roblox::vector3_t>(primitive + offsets::primitive + 8);
	return res;
}

union convertion
{
	std::uint64_t hex;
	float f;
} conv;

float antagonist::roblox::instance_t::get_health()
{
	auto one = read<std::uint64_t>(this->self + offsets::health);
	auto two = read<std::uint64_t>(read<std::uint64_t>(this->self + offsets::health));

	conv.hex = one ^ two;
	return conv.f;
}

float antagonist::roblox::instance_t::get_max_health()
{
	auto one = read<std::uint64_t>(this->self + offsets::max_health);
	auto two = read<std::uint64_t>(read<std::uint64_t>(this->self + offsets::max_health));

	conv.hex = one ^ two;
	return conv.f;
}

float antagonist::roblox::instance_t::get_walkspeed()
{
	auto one = read<std::uint64_t>(this->self + offsets::walkspeed - 4);
	auto two = read<std::uint64_t>(read<std::uint64_t>(this->self + offsets::walkspeed - 4));

	conv.hex = one ^ two;
	return conv.f;
}

antagonist::roblox::matrix3_t antagonist::roblox::instance_t::get_camera_cframe()
{
	auto getcframe_cam = read<antagonist::roblox::matrix3_t>(this->self + 0x104);
	return getcframe_cam;
}


antagonist::roblox::vector3_t antagonist::roblox::instance_t::get_value_vector3()
{
	auto getvalue = read<antagonist::roblox::vector3_t>(this->self + antagonist::offsets::value);
	return getvalue;
}

float antagonist::roblox::instance_t::get_value_float()
{
	auto getvalue = read<float>(this->self + antagonist::offsets::value);
	return getvalue;
}

bool antagonist::roblox::instance_t::get_value_bool()
{
	auto getvalue = read<bool>(this->self + antagonist::offsets::value);
	return getvalue;
}

int antagonist::roblox::instance_t::get_value_int()
{
	auto getvalue = read<int>(this->self + antagonist::offsets::value);
	return getvalue;
}

antagonist::roblox::instance_t antagonist::roblox::instance_t::get_team()
{
	auto getteam = read<antagonist::roblox::instance_t>(this->self + antagonist::offsets::team);
	return getteam;
}