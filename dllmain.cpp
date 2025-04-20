#include "resource.h"
#include <Windows.h>
#include <MinHook.h>
#include <stdio.h>

constexpr BYTE MINE_TILE = 0x80; // 0x8F is undiscovered iirc
constexpr uintptr_t tilesCleared = 0x010057a4; // tiles cleared, used in some checks

constexpr uintptr_t revealTileAddr = 0x01003512;
typedef void(__stdcall* RevealTile)(int, int);
RevealTile WM_RevealTile = reinterpret_cast<RevealTile>(revealTileAddr); // reveals a tile at x and y

constexpr uintptr_t setTileAddr = 0x01002eab;
typedef void(__stdcall* SetTile)(int, int, BYTE);
SetTile WM_SetTile = reinterpret_cast<SetTile>(setTileAddr); // sets a tile at x and y to a given cell ID, also refreshes the specified tile

constexpr uintptr_t renderFaceAddr = 0x010028d9;
typedef void(__stdcall* RenderFace)(HDC hdc, int faceState);
RenderFace WM_RenderFace = reinterpret_cast<RenderFace>(renderFaceAddr); // draws specified face (from some spritesheet) with SetDIBitsToDevice

constexpr uintptr_t faceRenderWrapperAddr = 0x01002913;
typedef void(__stdcall* FaceRenderWrapper)(int faceState);
FaceRenderWrapper WM_FaceRenderWrapper = reinterpret_cast<FaceRenderWrapper>(faceRenderWrapperAddr); // wrapper for WM_RenderFace, good bc it does GetDC and ReleaseDC itself

constexpr int pollInterval = static_cast<int>((1.f / 15.f) * 1000); // milliseconds, 15fps should be ok
bool bosnia = false; // is bosnia mode enabled

HINSTANCE hInst = nullptr;
HBITMAP hBosnia = nullptr;
BITMAP bosniaBMP = {};
BYTE* bosniaBits = nullptr;

static void loadBosniaFlag() {
	HBITMAP hBMP = (HBITMAP)LoadImageA(hInst, MAKEINTRESOURCEA(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (hBMP) {
		hBosnia = hBMP;
		GetObjectA(hBMP, sizeof(BITMAP), &bosniaBMP);
		bosniaBits = (BYTE*)bosniaBMP.bmBits;
	}
}

RevealTile originalRevealTile = nullptr;
static void __stdcall hkRevealTile(int x, int y) {
	if (bosnia) {
		*(int*)tilesCleared = 1; // ensure tiles cleared is never 0, otherwise the game moves the mine out of the way
		WM_SetTile(x, y, MINE_TILE); // set tile to mine
	}
	originalRevealTile(x, y); // call original function
}

RenderFace originalRenderFace = nullptr;
static void __stdcall hkRenderFace(HDC hdc, int faceState) {
	if (bosnia) {
		BITMAPINFO bmi = {};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = bosniaBMP.bmWidth;
		bmi.bmiHeader.biHeight = -bosniaBMP.bmHeight; // Negative = top-down
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 24;
		bmi.bmiHeader.biCompression = BI_RGB;
		SetDIBitsToDevice(hdc, (*(int*)(0x01005b2c) - 24) / 2, 16, 24, 24, 0, 0, 0, 24, bosniaBits, &bmi, DIB_RGB_COLORS); // WindowWidth addr should be 0x01005b2c
		return;
	}
	originalRenderFace(hdc, faceState); // call original function
}

static DWORD WINAPI MainThread(LPVOID lpReserved) {
	loadBosniaFlag();
	if (MH_Initialize() != MH_OK) return 1;
	if (MH_CreateHook((LPVOID)revealTileAddr, &hkRevealTile, reinterpret_cast<LPVOID*>(&originalRevealTile)) != MH_OK) return 1;
	if (MH_EnableHook((LPVOID)revealTileAddr) != MH_OK) return 1;
	if (MH_CreateHook((LPVOID)renderFaceAddr, &hkRenderFace, reinterpret_cast<LPVOID*>(&originalRenderFace)) != MH_OK) return 1;
	if (MH_EnableHook((LPVOID)renderFaceAddr) != MH_OK) return 1;

	bool last = false;
	while (true) {
		if ((GetAsyncKeyState('B') & 0x8000) != 0 && !last) {
			bosnia = !bosnia;
			WM_FaceRenderWrapper(0); // refresh face
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
		hInst = hModule;
		CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
	}else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		MH_DisableHook((LPVOID)revealTileAddr);
		MH_DisableHook((LPVOID)renderFaceAddr);
		MH_Uninitialize();
	}
	return TRUE;
}
