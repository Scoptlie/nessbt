#include "spriteRam.h"

#include <cstring>

namespace Ppu::SpriteRam {
	U8 content[256];
	
	U8 read(U8 addr) {
		return content[addr];
	}
	
	void write(U8 addr, U8 data) {
		if (addr % 4 == 2) {
			data &= 0b11100011;
		}
		content[addr] = data;
	}
	
	Sprite sprite(U8 idx) {
		return *(Sprite*)(content + (sizeof(Sprite) * idx));
	}
	
	void init() {
		memset(content, 0, sizeof(content));
	}
}
