#pragma once

#include "Common.h"

namespace Ppu {
	// Set to true when a frame is completed
	extern bool frameDone;
	
	// Rendered frame as an OpenGL texture
	extern U32 frame;
	
	// NMI signal
	extern bool nmi;
	
	U8 read(U16 addr);
	void write(U16 addr, U8 data);
	
	void tick();
	
	void init();
	void deinit();
}
