#pragma once

#include "common.h"

namespace Ppu::CiRam {
	extern U8 content[0x800];
	
	U8 read(U16 addr);
	void write(U16 addr, U8 data);
	
	void init();
	
}
