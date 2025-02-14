#pragma once

#include "common.h"

namespace Ppu {
	struct Sprite {
		U8 y;
		U8 patternIdx;
		U8 paletteIdx : 2;
		U8 padding : 3;
		U8 priority : 1;
		U8 flipX : 1;
		U8 flipY : 1;
		U8 x;
	};
	
	namespace SpriteRam {
		extern U8 content[256];
		
		U8 read(U8 addr);
		void write(U8 addr, U8 data);
		
		Sprite sprite(U8 idx);
		
		void init();
	}
}
