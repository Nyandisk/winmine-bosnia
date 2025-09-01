#include <cstdint>
#include <Windows.h>
#include <MinHook.h>
#include "resource.h"
#include "addresses.hpp"

constexpr uint8_t MineTile = 0x80;
constexpr int PollInterval = static_cast<int>((1.0f / 15.0f) * 1000);
constexpr int FaceSize = 24;

typedef void(__stdcall* revealTile)(int, int); // x, y
revealTile wm_revealTile = reinterpret_cast<revealTile>(winmine_addr::FuncRevealTile);

typedef void(__stdcall* setTile)(int, int, uint8_t); // x, y, val (ex. 0x80 MineTile)
setTile wm_setTile = reinterpret_cast<setTile>(winmine_addr::FuncSetTile);

typedef void(__stdcall* renderFace)(HDC, int); // hdc, faceState (0 -> 3 or something like that)
renderFace wm_renderFace = reinterpret_cast<renderFace>(winmine_addr::FuncRenderFace);

typedef void(__stdcall* renderFaceWrapper)(int); // faceState but cooler (no manual GetDC, ReleaseDC)
renderFaceWrapper wm_renderFaceWrapper = reinterpret_cast<renderFaceWrapper>(winmine_addr::FuncRenderFaceWrapper);

bool bosnia         = false;
HINSTANCE hInst     = nullptr;
HBITMAP hBosnia     = nullptr;
BITMAP bosniaBMP    = {};
uint8_t* bosniaBits = nullptr;

static void loadBosniaFlag() {
    HBITMAP hBMP = (HBITMAP)LoadImageA(hInst, MAKEINTRESOURCEA(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if (!hBMP) return;
    
    hBosnia = hBMP;
    GetObjectA(hBMP, sizeof(BITMAP), &bosniaBMP);
    bosniaBits = reinterpret_cast<uint8_t*>(bosniaBMP.bmBits);
}

revealTile originalRevealTile = nullptr;
static void __stdcall hkRevealTile(int x, int y) {
    if (bosnia) {
        *reinterpret_cast<int*>(winmine_addr::TilesCleared) = 1; // if the tiles cleared is 1, then the game assumes second move, and won't remove the mine out of the way
        wm_setTile(x, y, MineTile);
    }
    originalRevealTile(x, y);
}

renderFace originalRenderFace = nullptr;
static void __stdcall hkRenderFace(HDC hdc, int faceState) {
    if (bosnia) {
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = bosniaBMP.bmWidth;
        bmi.bmiHeader.biHeight = -bosniaBMP.bmHeight;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;

        int width = *reinterpret_cast<int*>(winmine_addr::WindowWidth);
        int faceX = (width - 24) / 2;
        int faceY = 16;

        SetDIBitsToDevice(
            hdc,
            faceX,
            faceY,
            FaceSize,
            FaceSize,
            0, 0, 0,
            FaceSize,
            bosniaBits,
            &bmi,
            DIB_RGB_COLORS
        );
        return;
    }
    originalRenderFace(hdc, faceState);
}

static bool tryHook(LPVOID target, LPVOID detour, LPVOID* original) {
    if (MH_CreateHook(target, detour, original) != MH_OK) return false;
    if (MH_EnableHook(target) != MH_OK) return false;
    return true;
}

static DWORD WINAPI MainThread(LPVOID lpReserved) {
    loadBosniaFlag();
    if (MH_Initialize() != MH_OK) return 1;

    // do some fishing
    if (!tryHook((LPVOID)winmine_addr::FuncRevealTile,
        &hkRevealTile,
        reinterpret_cast<LPVOID*>(&originalRevealTile))) return 1;

    if (!tryHook((LPVOID)winmine_addr::FuncRenderFace,
        &hkRenderFace,
        reinterpret_cast<LPVOID*>(&originalRenderFace))) return 1;

    bool last = false;
    while (true) {
        if ((GetAsyncKeyState('B') & 0x8000) != 0 && !last) {
            bosnia = !bosnia;
            wm_renderFaceWrapper(0);
            last = true;
        }
        else {
            last = false;
        }
        Sleep(PollInterval);
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        hInst = hModule;
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        MH_DisableHook((LPVOID)winmine_addr::FuncRevealTile);
        MH_DisableHook((LPVOID)winmine_addr::FuncRenderFace);
        MH_Uninitialize();
    }
    return TRUE;
}

