#pragma once

#include "runtime/common.h"

namespace Ppu {
	struct Rgb {
		U8 r, g, b;
	};
	
	extern Rgb globalPalette[0x40];
	
	extern U8 chrRom[0x4000];
	extern USize chrRomSize;
	
	extern bool mirrorV;
	
	extern U8 spriteRam[0x100];
	
	extern bool frameDone;
	extern U32 frame;
	
	extern bool nmi;
	
	U8 read(U8 addr);
	void write(U8 addr, U8 data);
	
	void tick();
	
	void init();
}
