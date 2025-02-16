#pragma once

#include "runtime/common.h"

namespace Ppu {
	extern U8 spriteRam[0x100];
	
	extern bool frameDone;
	extern U32 frame;
	
	extern bool nmi;
	
	U8 read(U8 addr);
	void write(U8 addr, U8 data);
	
	void tick();
	
	void init();
}
