#include "cart.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Cart {
	U8 *buf;
	
	bool mirrorV;
	
	U16 prgRomSize, chrRomSize;
	U8 *prgRom, *chrRom;
	
	void init(char const *romFile) {
		auto s = fopen(romFile, "rb");
		if (!s) {
			puts("failed to open rom file");
			exit(1);
		}
		
		fseek(s, 0, SEEK_END);
		auto bufLen = ftell(s);
		
		buf = new U8[bufLen];
		fseek(s, 0, SEEK_SET);
		fread(buf, 1, bufLen, s);
		
		fclose(s);
		
		if (memcmp(buf, "NES\x1a", 4) != 0) {
			puts("missing nes rom signature");
			exit(1);
		}
		
		if (buf[4] != 1 && buf[4] != 2) {
			puts("unsupported prg rom size");
			exit(1);
		}
		
		if (buf[5] != 1 && buf[5] != 2) {
			puts("unsupported chr rom size");
			exit(1);
		}
		
		if ((buf[6] & ~1) != 0) {
			puts("unsupported flags 6");
			exit(1);
		}
		
		if (buf[7] != 0 && buf[7] != 0b00001000) {
			puts("unsupported flags 7");
			exit(1);
		}
		
		mirrorV = buf[6] & 1;
		
		prgRomSize = buf[4] * 0x4000;
		chrRomSize = buf[5] * 0x2000;
		prgRom = buf + 16;
		chrRom = prgRom + prgRomSize;
	}
}
