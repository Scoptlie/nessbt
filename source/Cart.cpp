#include "Cart.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Cart {
	U8 *buf;
	
	bool mirrorV;
	
	U16 prgRomSize, chrRomSize;
	U8 *prgRom, *chrRom;
	
	U8 cpuRead(U16 addr) {
		return prgRom[addr & (prgRomSize - 1)];
	}
	
	U8 ppuRead(U16 addr) {
		return chrRom[addr & (chrRomSize - 1)];
	}
	
	void init(char const *file) {
		auto s = fopen(file, "rb");
		if (!s) {
			puts("unable to open rom file");
			exit(1);
		}
		defer(fclose(s));
		
		fseek(s, 0, SEEK_END);
		auto len = ftell(s);
		
		buf = new U8[len];
		fseek(s, 0, SEEK_SET);
		fread(buf, 1, len, s);
		
		if (memcmp(buf, "NES\x1a", 4) != 0) {
			puts("missing nes rom signature");
			exit(1);
		}
		
		prgRomSize = buf[4] * 0x4000;
		chrRomSize = buf[5] * 0x2000;
		
		mirrorV = buf[6] & 1;
		
		prgRom = buf + 16;
		chrRom = prgRom + prgRomSize;
	}
	
	void deinit() {
		delete[] buf;
	}
}
