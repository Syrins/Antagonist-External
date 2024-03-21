#include "logs.hpp"

#include <Windows.h>

void antagonist::utils::log::debug_log(const std::uint64_t address, const char *name)
{
	printf("[debug]: roblox: %s: %p\n", name, address);
}

std::vector<std::filesystem::path> antagonist::utils::log::get_roblox_file_logs()
{
    std::vector<std::filesystem::path> roblox_log;
    std::wstring roblox_log_path = L"C:\\Users\\" + antagonist::utils::log::get_username() + L"\\AppData\\Local\\Roblox\\logs";

    for (const auto &entry : std::filesystem::directory_iterator(roblox_log_path))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".log" && entry.path().filename().string().find("Player") != std::string::npos) 
            roblox_log.push_back(entry.path());
    }

    return roblox_log;
}

std::wstring antagonist::utils::log::get_username()
{
    wchar_t username[256];
    DWORD length = sizeof(username);
    GetUserNameW(username, &length);
    return username;
}

std::filesystem::path antagonist::utils::log::get_latest_log()
{
    auto logs = get_roblox_file_logs();

    std::sort(logs.begin(), logs.end(), [](const std::wstring &a, const std::wstring &b) {
        return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
        });

    return logs[0];
}