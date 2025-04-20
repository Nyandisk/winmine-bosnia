#include <Windows.h>
#include <stdio.h>
#include <MinHook.h>

constexpr BYTE MINE_TILE = 0x80; // 0x8F is undiscovered iirc
constexpr uintptr_t tilesCleared = 0x010057a4; // tiles cleared, used in some checks

constexpr uintptr_t revealTileAddr = 0x01003512;
typedef void(__stdcall* RevealTile)(int, int);
RevealTile WM_RevealTile = reinterpret_cast<RevealTile>(revealTileAddr); // reveals a tile at x and y

constexpr uintptr_t setTileAddr = 0x01002eab;
typedef void(__stdcall* SetTile)(int, int, BYTE);
SetTile WM_SetTile = reinterpret_cast<SetTile>(setTileAddr); // sets a tile at x and y to a given cell ID, also refreshes the specified tile

constexpr int pollInterval = static_cast<int>((1.f / 15.f) * 1000); // milliseconds, 15fps should be ok
bool bosnia = false; // is bosnia mode enabled

RevealTile originalRevealTile = nullptr;
static void __stdcall hkRevealTile(int x, int y) {
	if (bosnia) {
		*(int*)tilesCleared = 1; // ensure tiles cleared is never 0, otherwise the game moves the mine out of the way
		WM_SetTile(x, y, MINE_TILE); // set tile to mine
	}
	originalRevealTile(x, y); // call original function
}

static DWORD WINAPI MainThread(LPVOID lpReserved) {
	if (MH_Initialize() != MH_OK) return 1;
	if (MH_CreateHook((LPVOID)revealTileAddr, &hkRevealTile, reinterpret_cast<LPVOID*>(&originalRevealTile)) != MH_OK) return 1;
	if (MH_EnableHook((LPVOID)revealTileAddr) != MH_OK) return 1;

	bool last = false;
	while (true) {
		if ((GetAsyncKeyState('B') & 0x8000) != 0 && !last) {
			bosnia = !bosnia;
			last = true;
		}
		else {
			last = false;
		}
		Sleep(pollInterval);
	}
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
	}else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		FreeConsole();
		MH_DisableHook((LPVOID)revealTileAddr);
		MH_Uninitialize();
	}
	return TRUE;
}
