#pragma once

#include "runtime/common.h"

namespace Ppu {
	struct Rgb {
		U8 r, g, b;
	};
	
	extern Rgb globalPalette[64];
}
