#define IMGUI_DEFINE_MATH_OPERATORS

#include "overlay.hpp"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <dwmapi.h>

#include "../../roblox/esp/esp.hpp"
#include "../../roblox/globals/globals.hpp"

#include "ckeybind/keybind.hpp"
#include "../skcrypt/skStr.hpp"
#include "../xorstr/xorstr.hpp"
#include "../../keyauth/keyauth.hpp"

#include "../configs/configs.hpp"
#include "../json/json.hpp"

#include <filesystem>
#include "../datamodel/datamodel.hpp"

auto name = skCrypt("antagonist");
auto ownerid = skCrypt("edoerDh0mj");
auto secret = skCrypt("7d35557094348565afdd4244f65c40e14652452ee3a93c98bda642fd9493a61f");
auto version = skCrypt("1.0");
auto url = skCrypt("https://keyauth.win/api/1.2/");

KeyAuth::api KeyAuthApp(name.decrypt(), ownerid.decrypt(), secret.decrypt(), version.decrypt(), url.decrypt());

ID3D11Device* antagonist::utils::overlay::d3d11_device = nullptr;

ID3D11DeviceContext* antagonist::utils::overlay::d3d11_device_context = nullptr;

IDXGISwapChain* antagonist::utils::overlay::dxgi_swap_chain = nullptr;

ID3D11RenderTargetView* antagonist::utils::overlay::d3d11_render_target_view = nullptr;

static const char* combo_items_4[4] = { ("Head"), ("UpperTorso"), ("HumanoidRootPart"), ("LowerTorso") };
static const char* box_items_3[3] = { ("2D Box"), ("2D Corner Box"), ("2D Circle [Requested by Mot]") };

bool Keybind(CKeybind* keybind, const ImVec2& size_arg = ImVec2(0, 0), bool clicked = false, ImGuiButtonFlags flags = 0)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;
	// SetCursorPosX(window->Size.x - 14 - size_arg.x);
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(keybind->get_name().c_str());
	const ImVec2 label_size = ImGui::CalcTextSize(keybind->get_name().c_str(), NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) &&
		style.FramePadding.y <
		window->DC.CurrLineTextBaseOffset)  // Try to vertically align buttons that are smaller/have no padding so that
		// text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = ImGui::CalcItemSize(
		size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	bool value_changed = false;
	int key = keybind->key;

	auto io = ImGui::GetIO();

	std::string name = keybind->get_key_name();

	if (keybind->waiting_for_input)
		name = "waiting";

	if (ImGui::GetIO().MouseClicked[0] && hovered)
	{
		if (g.ActiveId == id)
		{
			keybind->waiting_for_input = true;
		}
	}
	else if (ImGui::GetIO().MouseClicked[1] && hovered)
	{
		ImGui::OpenPopup(keybind->get_name().c_str());
	}
	else if (ImGui::GetIO().MouseClicked[0] && !hovered)
	{
		if (g.ActiveId == id)
			ImGui::ClearActiveID();
	}

	if (keybind->waiting_for_input)
	{
		if (ImGui::GetIO().MouseClicked[0] && !hovered)
		{
			keybind->key = VK_LBUTTON;

			ImGui::ClearActiveID();
			keybind->waiting_for_input = false;
		}
		else
		{
			if (keybind->set_key())
			{
				ImGui::ClearActiveID();
				keybind->waiting_for_input = false;
			}
		}
	}

	// Render
	ImVec4 textcolor = ImLerp(ImVec4(201 / 255.f, 204 / 255.f, 210 / 255.f, 1.f), ImVec4(1.0f, 1.0f, 1.0f, 1.f), 1.f);

	window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(33 / 255.0f, 33 / 255.0f, 33 / 255.0f, 1.f), 2.f);
	//window->DrawList->AddRect( bb.Min, bb.Max, ImColor( 0.f, 0.f, 0.f, 1.f ) );

	window->DrawList->AddText(
		bb.Min +
		ImVec2(
			size_arg.x / 2 - ImGui::CalcTextSize(name.c_str()).x / 2,
			size_arg.y / 2 - ImGui::CalcTextSize(name.c_str()).y / 2),
		ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)),
		name.c_str());

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar;
	//ImGui::SetNextWindowPos( pos + ImVec2( 0, size_arg.y - 1 ) );
	//ImGui::SetNextWindowSize( ImVec2( size_arg.x, 47 * 1.f ) );

	{
		if (ImGui::BeginPopup(keybind->get_name().c_str(), 0))
		{

			{
				{
					ImGui::BeginGroup();
					{
						if (ImGui::Selectable("hold", keybind->type == CKeybind::HOLD))
							keybind->type = CKeybind::HOLD;
						if (ImGui::Selectable("toggle", keybind->type == CKeybind::TOGGLE))
							keybind->type = CKeybind::TOGGLE;
					}
					ImGui::EndGroup();
				}
			}

			ImGui::EndPopup();
		}
	}

	return pressed;
}

bool antagonist::utils::overlay::fullsc(HWND windowHandle)
{
	MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
	if (GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
	{
		RECT windowRect;
		if (GetWindowRect(windowHandle, &windowRect))
		{
			return windowRect.left == monitorInfo.rcMonitor.left
				&& windowRect.right == monitorInfo.rcMonitor.right
				&& windowRect.top == monitorInfo.rcMonitor.top
				&& windowRect.bottom == monitorInfo.rcMonitor.bottom;
		}
	}
}

void display_explorer_nodes(antagonist::roblox::instance_t instance)
{
	for (auto& child : instance.children())
	{
		std::string displayText = child.name() + " [" + child.class_name() + "]";

		bool isNodeOpen = ImGui::TreeNodeEx(displayText.c_str(), ImGuiTreeNodeFlags_OpenOnArrow);

		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
		{
			ImGui::OpenPopup("Context Menu");
		}

		if (ImGui::BeginPopup("Context Menu"))
		{
			if (ImGui::MenuItem("Copy Text"))
			{
				ImGui::SetClipboardText(displayText.c_str());
			}
			ImGui::EndPopup();
		}

		if (isNodeOpen)
		{
			display_explorer_nodes(child);
			ImGui::TreePop();
		}
	}
}


static char text[999] = "";
char configname[100];

bool antagonist::utils::overlay::init = false;

bool if_first = true;

bool ButtonCenteredOnLine(const char* label, float alignment = 0.5f)
{
	ImGuiStyle& style = ImGui::GetStyle();

	float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
	float avail = ImGui::GetContentRegionAvail().x;

	float off = (avail - size) * alignment;
	if (off > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

	return ImGui::Button(label);
}

bool antagonist::utils::overlay::render()
{
	name.clear(); ownerid.clear(); secret.clear(); version.clear(); url.clear();

	if (if_first)
	{
		KeyAuthApp.init();
		if_first = false;
	}

	nlohmann::json json;

	if (globals::show_auth)
	{
		std::string filePath = appdata_path() + XorStr("\\antagonist\\license.json");

		if (!std::filesystem::exists(filePath))
		{
			exit(0);
		}

		if (std::filesystem::exists(filePath))
		{
			std::ifstream file(filePath);
			if (file.is_open())
			{
				file >> json;
				file.close();

				std::string stored_license = json["license"];
				KeyAuthApp.license(stored_license);

				if (KeyAuthApp.data.success)
				{
					globals::show_auth = false;
				}
				else
				{
					exit(0);
				}
			}
		}

		ImGui_ImplWin32_EnableDpiAwareness();

		WNDCLASSEX wc;
		wc.cbClsExtra = NULL;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.cbWndExtra = NULL;
		wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hInstance = GetModuleHandle(nullptr);
		wc.lpfnWndProc = window_proc;
		wc.lpszClassName = TEXT("flexmusix");
		wc.lpszMenuName = nullptr;
		wc.style = CS_VREDRAW | CS_HREDRAW;

		RegisterClassEx(&wc);
		const HWND hw = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, wc.lpszClassName, TEXT("4daflex"),
			WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wc.hInstance, nullptr);

		SetLayeredWindowAttributes(hw, 0, 255, LWA_ALPHA);
		const MARGINS margin = { -1 };
		DwmExtendFrameIntoClientArea(hw, &margin);

		if (!create_device_d3d(hw))
		{
			cleanup_device_d3d();
			UnregisterClass(wc.lpszClassName, wc.hInstance);
			return false;
		}

		ShowWindow(hw, SW_SHOW);
		UpdateWindow(hw);

		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui::GetIO().IniFilename = nullptr;

		ImGui_ImplWin32_Init(hw);
		ImGui_ImplDX11_Init(d3d11_device, d3d11_device_context);

		const ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		init = true;

		bool draw = false;
		bool done = false;
		int tab = 0;
		while (!done)
		{
			MSG msg;
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
				{
					done = true;
				}
			}

			if (done)
				break;

			move_window(hw);

			if ((GetAsyncKeyState(VK_INSERT) & 1 || GetAsyncKeyState(VK_F5) & 1) && !globals::show_auth)
				draw = !draw;

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			{
				if (GetForegroundWindow() == FindWindowA(0, XorStr("Roblox")) || GetForegroundWindow() == hw)
				{
					ImGui::Begin(XorStr("overlay"), nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
					{
						antagonist::roblox::hook_esp();
						ImGui::End();
					}

					if (draw && !globals::show_auth)
					{
						ImGui::SetNextWindowSize(ImVec2(714, 397));
						ImGui::Begin(XorStr("antagonist"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
						ImGui::BeginChild(XorStr("ChildWindow"), ImVec2(110, 358), true);

						ImGuiStyle& style = ImGui::GetStyle();
						ImVec4 originalButtonColor = style.Colors[ImGuiCol_Button];
						style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

						if (tab == 0)
						{
							ImGui::EndChild();

							ImGui::SetCursorPosX(125);
							ImGui::SetCursorPosY(27);

							ImGui::BeginChild(XorStr("AimbotChildWindow"), ImVec2(580, 358), true);
							{
								ImGui::Text(XorStr("Aimbot"));

								ImGui::Separator();

								ImGui::Checkbox(XorStr("Enabled"), &globals::aimbot);
								ImGui::SameLine(); Keybind(&globals::aimbotkey, ImVec2(60, 20));
								ImGui::Combo(XorStr("Part"), &globals::aimpart, combo_items_4, 4);
								ImGui::SliderFloat(XorStr("Smoothness X"), &globals::smoothness_x, 1, 10);
								ImGui::SliderFloat(XorStr("Smoothness Y"), &globals::smoothness_y, 1, 10);

								ImGui::Separator();

								ImGui::Text(XorStr("Prediction"));
								ImGui::Checkbox(XorStr("Prediction"), &globals::prediction);
								ImGui::SliderFloat(XorStr("Prediction X"), &globals::prediction_x, 1, 10);
								ImGui::SliderFloat(XorStr("Prediction Y"), &globals::prediction_y, 1, 10);

								//ImGui::Separator();
								//ImGui::Text(XorStr("Offsets"));
								//ImGui::SliderInt(XorStr("X Offset"), &globals::x_offset, 1, 10);
								//ImGui::SliderInt(XorStr("Y Offset"), &globals::y_offset, 1, 10);

								//ImGui::Separator();
								//ImGui::SliderFloat(XorStr("Range"), &globals::range, 0, 1000);
								//ImGui::SliderFloat(XorStr("Range Multiplier"), &globals::range_mult, 0, 100);

								ImGui::Separator();

								ImGui::Text(XorStr("Misc Aimbot"));
								ImGui::Checkbox(XorStr("Teamcheck"), &globals::team_check);
								ImGui::Checkbox(XorStr("Sticky Aim"), &globals::sticky_aim);
								ImGui::SliderFloat(XorStr("Aimbot Sensitivity"), &globals::sensitivity, 0, 1);
								ImGui::Checkbox(XorStr("Disable Outside of FOV"), &globals::disable_outside_fov);
								//ImGui::Checkbox(XorStr("Knock Check [Da Hood]"), &globals::knock_check);

								//ImGui::Separator();

								//ImGui::SliderInt(XorStr("Max Distance"), &globals::max_dist, 1, 10000);

								//ImGui::Checkbox(XorStr("Triggerbot"), &globals::triggerbot);

								//ImGui::Checkbox("Shake", &globals::shake);
								//ImGui::SliderFloat(XorStr("Shake Value"), &globals::shake_value, 0, 10);
							}
							ImGui::EndChild();

							ImGui::BeginChild(XorStr("ChildWindow"), ImVec2(110, 358), true);
						}

						if (tab == 1)
						{
							ImGui::EndChild();

							ImGui::SetCursorPosX(125);
							ImGui::SetCursorPosY(27);

							ImGui::BeginChild(XorStr("VisualChildWindow"), ImVec2(580, 358), true);
							{
								ImGui::Text(XorStr("Visuals"));

								ImGui::Separator();

								ImGui::Checkbox(XorStr("Enabled"), &globals::esp);
								ImGui::Checkbox(XorStr("Box"), &globals::box);
								ImGui::Combo(XorStr("Type"), &globals::boxtype, box_items_3, 3);
								ImGui::Checkbox(XorStr("Name"), &globals::name_esp);
								ImGui::Checkbox(XorStr("Tracers"), &globals::tracers);

								ImGui::Separator();

								ImGui::Checkbox(XorStr("Draw FOV"), &globals::fov_on);
								ImGui::SliderInt(XorStr("FOV"), &globals::fov, 1, 1000);

								ImGui::Separator();

								ImGui::Checkbox(XorStr("Teamcheck"), &globals::team_check_esp);

								ImGui::SliderFloat(XorStr("Field of View [Camera]"), &globals::fov_roblox, 1, 120);
								{
									globals::camera.set_fov(globals::fov_roblox);
								}
							}
							ImGui::EndChild();

							ImGui::BeginChild(XorStr("ChildWindow"), ImVec2(110, 358), true);
						}

						if (tab == 2)
						{
							ImGui::EndChild();

							ImGui::SetCursorPosX(125);
							ImGui::SetCursorPosY(27);

							ImGui::BeginChild(XorStr("SettingsChildWindow"), ImVec2(580, 358), true);
							{
								ImGui::Text(XorStr("Settings"));

								ImGui::Separator();

								ImGui::Checkbox(XorStr("Streamproof"), &globals::streamproof);
								ImGui::Checkbox(XorStr("V-Sync"), &globals::vsync);

								if (ImGui::IsItemHovered())
								{
									ImGui::BeginTooltip();
									ImGui::Text("turn v-sync off if you have a good pc");
									ImGui::EndTooltip();
								}

								ImGui::Separator();

								ImGui::Text(XorStr("Configs"));

								std::vector<std::string> configFiles;
								std::string configFolderPath = antagonist::utils::appdata_path() + "\\antagonist\\configs";
								for (auto file : std::filesystem::directory_iterator(configFolderPath)) {
									std::filesystem::path filePath = file;
									std::string extension = filePath.extension().string();

									if (extension == ".cfg") {
										if (!std::filesystem::is_directory(file.path())) {
											auto path2 = file.path().string().substr(configFolderPath.length() + 1);
											configFiles.push_back(path2.c_str());
										}
									}
								}

								static std::string current_item = configFiles.empty() ? "" : configFiles[0];

								for (int n = 0; n < configFiles.size(); n++) {
									if (configFiles[n] == "")
										continue;

									bool is_selected = (current_item == configFiles[n]);
									if (ImGui::Selectable(configFiles[n].c_str(), is_selected)) {
										current_item = configFiles[n];

										size_t extensionPos = current_item.find(".cfg");
										if (extensionPos != std::string::npos) {
											current_item.erase(extensionPos);
										}

										strncpy(configname, current_item.c_str(), IM_ARRAYSIZE(configname));
									}
								}

								ImGui::InputText(XorStr("Config Name"), configname, IM_ARRAYSIZE(configname));
								if (ImGui::Button(XorStr("Load"))) {
									antagonist::utils::configs::load(configname);
								}

								ImGui::SameLine();

								if (ImGui::Button(XorStr("Save"))) {
									antagonist::utils::configs::save(configname);
								}

								ImGui::SameLine();

								if (ImGui::Button(XorStr("Open Folder Location")))
								{
									ShellExecute(NULL, "open", "explorer.exe", (antagonist::utils::appdata_path() + "\\antagonist\\configs").c_str(), NULL, SW_SHOWNORMAL);
								}

							}
							ImGui::EndChild();

							ImGui::BeginChild(XorStr("ChildWindow"), ImVec2(110, 358), true);
						}

						if (tab == 3)
						{
							ImGui::EndChild();

							ImGui::SetCursorPosX(125);
							ImGui::SetCursorPosY(27);

							ImGui::BeginChild(XorStr("ExplorerChildWindow"), ImVec2(580, 358), true);
							{
								display_explorer_nodes(globals::datamodel);
							}

							ImGui::TreePop();
							ImGui::EndChild();

							//ImGui::SetCursorPosX(515);
							//ImGui::SetCursorPosY(27);

							//ImGui::BeginChild(XorStr("PropertiesChildWindow"), ImVec2(190, 358), true);
							//{
							//
							//}
							//ImGui::EndChild();

							ImGui::BeginChild(XorStr("ChildWindow"), ImVec2(110, 358), true);
						}

						//if (tab == 4)
						//{
						//	ImGui::EndChild();

						//	ImGui::SetCursorPosX(125);
						//	ImGui::SetCursorPosY(27);

						//	ImGui::BeginChild(XorStr("PlayerChildWindow"), ImVec2(580, 358), true);
						//	{
						//		std::vector<std::string> ret;

						//		for (auto plr : globals::players.children())
						//		{
						//			if (plr.self && plr.name() != globals::datamodel.find_first_child_of_class("Players").get_local_player().name())
						//			{
						//				ret.push_back(plr.name());
						//			}
						//		}

						//		if (!ret.size())
						//			ret.push_back("No Players");

						//		static std::string current_item = ret[0];

						//		if (ret.size())
						//		{
						//			for (int n = 0; n < ret.size(); n++)
						//			{
						//				bool is_selected = (current_item == ret[n]);
						//				if (ImGui::Selectable(ret[n].c_str(), is_selected))
						//				{
						//					current_item = ret[n];
						//				}
						//			}
						//		}
						//		Sleep(1);
						//	}

						//	ImGui::EndChild();

						//	ImGui::BeginChild(XorStr("ChildWindow"), ImVec2(110, 358), true);
						//}

						if (ButtonCenteredOnLine(XorStr("Aimbot")))
						{
							tab = 0;
						}

						if (ButtonCenteredOnLine(XorStr("Visuals")))
						{
							tab = 1;
						}

						if (ButtonCenteredOnLine(XorStr("Settings")))
						{
							tab = 2;
						}

						ImGui::Separator();

						if (ButtonCenteredOnLine(XorStr("Explorer")))
						{
							tab = 3;
						}
						
						//if (ButtonCenteredOnLine(XorStr("Players")))
						//{
						//	tab = 4;
						//}

						style.Colors[ImGuiCol_Button] = originalButtonColor;

						ImGui::EndChild();
						ImGui::End();
					}
				}


				if (globals::streamproof)
				{
					SetWindowDisplayAffinity(hw, WDA_EXCLUDEFROMCAPTURE);
				}
				else
				{
					SetWindowDisplayAffinity(hw, WDA_NONE);
				}

				//if (globals::show_auth)
				if (draw)
				{
					SetWindowLong(hw, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
				}
				else
				{
					SetWindowLong(hw, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
				}

				ImGui::EndFrame();
				ImGui::Render();

				const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
				d3d11_device_context->OMSetRenderTargets(1, &d3d11_render_target_view, nullptr);
				d3d11_device_context->ClearRenderTargetView(d3d11_render_target_view, clear_color_with_alpha);
				ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

				if (globals::vsync)
				{
					dxgi_swap_chain->Present(1, 0);
				}
				else
				{
					dxgi_swap_chain->Present(0, 0);

				}
			}
		}

		init = false;

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		cleanup_device_d3d();
		DestroyWindow(hw);
		UnregisterClass(wc.lpszClassName, wc.hInstance);
	}
}

void antagonist::utils::overlay::move_window(HWND hw)
{
	HWND target = FindWindowA(0, XorStr("Roblox"));
	HWND foregroundWindow = GetForegroundWindow();

	if (target != foregroundWindow && hw != foregroundWindow)
	{
		MoveWindow(hw, 0, 0, 0, 0, true);
	}
	else
	{
		RECT rect;
		GetWindowRect(target, &rect);

		int rsize_x = rect.right - rect.left;
		int rsize_y = rect.bottom - rect.top;

		if (fullsc(target))
		{
			rsize_x += 16;
			rsize_y -= 24;

			MoveWindow(hw, rect.left, rect.top, rsize_x, rsize_y, TRUE);
		}
		else
		{
			rsize_y -= 63;
			rect.left += 8;
			rect.top += 31;
		}

		MoveWindow(hw, rect.left, rect.top, rsize_x, rsize_y, TRUE);
	}
}

bool antagonist::utils::overlay::create_device_d3d(HWND hw)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hw;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	const UINT create_device_flags = 0;
	D3D_FEATURE_LEVEL d3d_feature_level;
	const D3D_FEATURE_LEVEL feature_level_arr[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_device_flags, feature_level_arr, 2, D3D11_SDK_VERSION, &sd, &dxgi_swap_chain, &d3d11_device, &d3d_feature_level, &d3d11_device_context);
	if (res == DXGI_ERROR_UNSUPPORTED)
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, create_device_flags, feature_level_arr, 2, D3D11_SDK_VERSION, &sd, &dxgi_swap_chain, &d3d11_device, &d3d_feature_level, &d3d11_device_context);
	if (res != S_OK)
		return false;

	create_render_target();
	return true;
}

void antagonist::utils::overlay::cleanup_device_d3d()
{
	cleanup_render_target();

	if (dxgi_swap_chain)
	{
		dxgi_swap_chain->Release();
		dxgi_swap_chain = nullptr;
	}

	if (d3d11_device_context)
	{
		d3d11_device_context->Release();
		d3d11_device_context = nullptr;
	}

	if (d3d11_device)
	{
		d3d11_device->Release();
		d3d11_device = nullptr;
	}
}

void antagonist::utils::overlay::create_render_target()
{
	ID3D11Texture2D* d3d11_back_buffer;
	dxgi_swap_chain->GetBuffer(0, IID_PPV_ARGS(&d3d11_back_buffer));
	if (d3d11_back_buffer != nullptr)
	{
		d3d11_device->CreateRenderTargetView(d3d11_back_buffer, nullptr, &d3d11_render_target_view);
		d3d11_back_buffer->Release();
	}
}

void antagonist::utils::overlay::cleanup_render_target()
{
	if (d3d11_render_target_view)
	{
		d3d11_render_target_view->Release();
		d3d11_render_target_view = nullptr;
	}
}

LRESULT __stdcall antagonist::utils::overlay::window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (d3d11_device != nullptr && wParam != SIZE_MINIMIZED)
		{
			cleanup_render_target();
			dxgi_swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			create_render_target();
		}
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}