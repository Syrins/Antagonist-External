#include "roblox/classes/classes.hpp"
#include "utils/configs/configs.hpp"

#include <filesystem>
#include <iostream>
#include <thread>

bool isadmin()
{
	BOOL isAdmin = FALSE;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID admin;

	if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &admin))
	{
		CheckTokenMembership(NULL, admin, &isAdmin);
		FreeSid(admin);
	}

	return isAdmin != 0;
}


int main(/* credits to AGC / A GREAT CHAOS / KEN CARSON */)
{
	SetConsoleTitle(TEXT("flexflexflex"));

	HWND hwndConsole = GetConsoleWindow();
	SetWindowPos(hwndConsole, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	if (!isadmin)
	{
		printf("[error]: the cheat must be ran as administrator\n");
	}

	//std::string appdata = antagonist::utils::appdata_path();
	//if (!std::filesystem::exists(appdata + "\\antagonist"))
	//{
	//	std::filesystem::create_directory(appdata + "\\antagonist");
	//}

	//if (!std::filesystem::exists(appdata + "\\antagonist\\configs"))
	//{
	//	std::filesystem::create_directory(appdata + "\\antagonist\\configs");
	//}

	antagonist::roblox::init();

	printf("[debug]: press enter to close console [this might show up if theres an issue]");
	std::cin.get();

	//system("pause");
}