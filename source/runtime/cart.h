#pragma once

#include "runtime/common.h"

namespace Cart {
	extern bool mirrorV;
	
	extern U16 prgRomSize, chrRomSize;
	extern U8 *prgRom, *chrRom;
	
	U8 cpuRead(U16 addr);
	U8 ppuRead(U16 addr);
	
	void init(char const *romFile);
}
