#pragma once
#include <stdio.h>

namespace winmine_addr {
	// Variable addresses
	constexpr uintptr_t TilesCleared = 0x010057A4;
	constexpr uintptr_t WindowWidth = 0x01005B2C;
	constexpr uintptr_t MineCount = 0x01005330;
	constexpr uintptr_t BoardWidth = 0x01005334;
	constexpr uintptr_t BoardHeight = 0x01005338;

	// Function addresses				     
	constexpr uintptr_t FuncRevealTile = 0x01003512;
	constexpr uintptr_t FuncSetTile = 0x01002EAB;
	constexpr uintptr_t FuncRenderFace = 0x010028D9;
	constexpr uintptr_t FuncRenderFaceWrapper = 0x01002913;

	// im lowk lazy and the built in random function is RIGHT THERE so why not
	constexpr uintptr_t FuncRandomInt = 0x01003940;	
}