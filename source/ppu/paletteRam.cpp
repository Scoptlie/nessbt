#include "paletteRam.h"

#include <cstring>

namespace Ppu::PaletteRam {
	U8 content[0x20];
	
	U16 addrMapped(U16 addr) {
		if (addr % 4 == 0) {
			addr %= 16;
		}
		return addr;
	}
	
	U8 colourIdxMasked(U8 colourIdx, bool greyscale) {
		return colourIdx & (greyscale? 0xf0 : 0xff);
	}
	
	U8 read(U16 addr, bool greyscale) {
		return colourIdxMasked(content[addrMapped(addr)], greyscale);
	}
	
	void write(U16 addr, U8 data) {
		content[addrMapped(addr)] = data % 64;
	}
	
	U8 clearColour(bool greyscale) {
		return colourIdxMasked(content[0], greyscale);
	}
	
	Palette palette(U16 paletteIdx, bool greyscale) {
		U16 addr = 4 * paletteIdx;
		return Palette{
			.colourIdxs = {
				colourIdxMasked(content[addr], greyscale),
				colourIdxMasked(content[addr + 1], greyscale),
				colourIdxMasked(content[addr + 2], greyscale),
				colourIdxMasked(content[addr + 3], greyscale)
			}
		};
	}
	
	void init() {
		memset(content, 0, sizeof(content));
	}
}
