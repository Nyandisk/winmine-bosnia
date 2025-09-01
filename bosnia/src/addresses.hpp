#pragma once
#include <stdio.h>

namespace winmine_addr {
	// Variable addresses
	constexpr uintptr_t TilesCleared	      = 0x010057A4;
	constexpr uintptr_t WindowWidth		      = 0x01005B2C;
										     
	// Function addresses				     
	constexpr uintptr_t FuncRevealTile	      = 0x01003512;
	constexpr uintptr_t FuncSetTile	          = 0x01002EAB;
	constexpr uintptr_t FuncRenderFace        = 0x010028D9;
	constexpr uintptr_t FuncRenderFaceWrapper = 0x01002913;
}