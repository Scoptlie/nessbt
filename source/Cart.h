#pragma once

#include "common.h"

namespace Cart {
	extern bool mirrorV;
	
	extern U16 prgRomSize, chrRomSize;
	extern U8 *prgRom, *chrRom;
	
	void init(char const *romFile);
	void deinit();
}
