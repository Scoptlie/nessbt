#pragma once

#include "Common.h"

namespace Cart {
	extern bool mirrorV;
	
	U8 cpuRead(U16 addr);
	
	U8 ppuRead(U16 addr);
	
	void init(char const *file);
	void deinit();
}
