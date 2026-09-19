#pragma once
#include "Overlay.h"
#include <string>
#include <mutex>

// CRITICAL: ImGui ColorEdit3 expects 0.0-1.0 range, NOT 0-255!
float boxTemp[3] = { 0.0f, 0.906f, 1.0f };  // RGB: 0, 231, 255 -> normalized
float nameTemp[3] = { 0.0f, 0.906f, 1.0f }; // RGB: 0, 231, 255 -> normalized

RGBA BoxColor = {0, 231, 255, 255};
RGBA NameColor = {0, 231, 255, 255};

bool ShowMenu = false;
bool Unhook = false;

bool esp = true;
bool enemy_box = true;
bool enemy_name = true;
bool def_models = true;
bool in_lobby = false;

// Aimbot Settings
bool aimbot_active = false;
bool aimbot_on_fire = true; // Only aim when shooting? or Key hold
float aimbot_fov = 100.0f; // Pixel radius
float aimbot_smooth = 5.0f;
float aimbot_bone_z = 22.0f; // Z Offset for Head (Adjustable)
int aimbot_key = 0x01; // VK_LBUTTON

int language = 0; // 0: English, 1: Turkish

struct PlayerPosition {
	float screenPosition[2] = {0};
	float state = 0.0f;
	bool dead = false;
	float health = 0.0f;
	int team = 0;
	std::string name;
};

PlayerPosition players[64];
std::mutex playerMutex; // Mutex for thread safety


namespace DirectX9Interface {
	IDirect3D9Ex* Direct3D9 = NULL;
	IDirect3DDevice9Ex* pDevice = NULL;
	D3DPRESENT_PARAMETERS pParams = { NULL };
	MARGINS Margin = { -1 };
	MSG Message = { NULL };
}

namespace OverlayWindow {
	WNDCLASSEX WindowClass;
	HWND Hwnd;
	LPCSTR Name;
}

class Vector2 {
public:
	float x, y;
};

namespace Process {
	DWORD ID = 0;
	HANDLE Handle = NULL;
	HWND Hwnd = NULL;
	WNDPROC WndProc = NULL;
	int WindowWidth = 0;
	int WindowHeight = 0;
	int WindowLeft = 0;
	int WindowRight = 0;
	int WindowTop = 0;
	int WindowBottom = 0;
	LPCSTR Title = nullptr;
	LPCSTR ClassName = nullptr;
	LPCSTR Path = nullptr;
}

namespace Game {
	HANDLE handle = NULL;
	uintptr_t client = NULL;
	uintptr_t engine = NULL;
	DWORD PID = 0;
}
