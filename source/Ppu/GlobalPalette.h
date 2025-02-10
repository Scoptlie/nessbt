#pragma once

#include "Common.h"

namespace Ppu {
	struct Rgb {
		U8 r, g, b;
	};
	
	namespace GlobalPalette {
		extern Rgb colours[512];
		
		Rgb colour(U8 idx, U8 emphasis);
	}
}
