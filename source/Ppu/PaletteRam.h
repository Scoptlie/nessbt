#pragma once

#include "Common.h"

namespace Ppu {
	struct Palette {
		U8 colourIdxs[4];
	};
	
	namespace PaletteRam {
		extern U8 content[0x20];
		
		U8 read(U16 addr, bool greyscale);
		void write(U16 addr, U8 data);
		
		U8 clearColour(bool greyscale);
		Palette palette(U16 paletteIdx, bool greyscale);
		
		void init();
		void deinit();
	}
}
