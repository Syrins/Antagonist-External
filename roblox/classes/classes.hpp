#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace antagonist
{
	namespace offsets
	{
		constexpr std::uint32_t size = 0x8; /* 8 bytes */
		constexpr std::uint32_t name = 0x48;
		constexpr std::uint32_t classname = 0x18;
		constexpr std::uint32_t children = 0x50;
		constexpr std::uint32_t parent = 0x60;
		
		constexpr std::uint32_t local_player = 0x240;
		constexpr std::uint32_t model_instance = 0x188;
		
		constexpr std::uint32_t primitive = 0x148;
		constexpr std::uint32_t position = 0x13C;

		constexpr std::uint32_t dimensions = 0x718;
		constexpr std::uint32_t viewmatrix = 0x4B0;

		constexpr std::uint32_t health = 0x358;
		constexpr std::uint32_t max_health = 0x368;
		constexpr std::uint32_t walkspeed = 0x340;
		constexpr std::uint32_t jumpspeed = 0x370;

		constexpr std::uint32_t team = 0x1D8;
		constexpr std::uint32_t value = 0xD4;

		constexpr std::uint32_t camera_x = 0x10C;
		constexpr std::uint32_t camera_y = 0x118;
		constexpr std::uint32_t camera_pos = 0xA0;
	}

	namespace roblox
	{
		bool init();

		struct vector2_t final { float x, y; };
		struct vector3_t final { float x, y, z; };
		struct quaternion final { float x, y, z, w; };
		struct matrix3_t final { float elt[3][3]; };
		struct matrix4_t final { float data[16]; };

		class instance_t final
		{
		public:
			std::uint64_t self;

			float fov();
			void set_fov(float nigger);

			std::string name();
			std::string class_name();

			std::vector<antagonist::roblox::instance_t> children();
			antagonist::roblox::instance_t find_first_child(std::string child);
			antagonist::roblox::instance_t find_first_child_of_class(std::string child);

			/* decided to have everything in one class rather than have multiple, it will be harder to manage (im lazy) */

			antagonist::roblox::instance_t get_local_player();
			antagonist::roblox::instance_t get_model_instance();
			antagonist::roblox::instance_t get_team();

			antagonist::roblox::vector2_t get_dimensions();
			antagonist::roblox::matrix4_t get_view_matrix();
			antagonist::roblox::vector3_t get_camera_pos(); 
			antagonist::roblox::vector3_t get_part_pos();
			antagonist::roblox::vector3_t get_part_velocity();

			float get_health();
			float get_max_health();

			float get_walkspeed();

			float get_value_float();
			bool get_value_bool();
			int get_value_int();

			antagonist::roblox::matrix3_t get_camera_cframe();
			antagonist::roblox::vector3_t get_value_vector3();
		};

		antagonist::roblox::vector2_t world_to_screen(antagonist::roblox::vector3_t world);
	}
}