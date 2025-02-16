#pragma once

#include "runtime/common.h"

namespace Cpu {
	extern U8 ram[0x800];
	
	extern U8 prgRom[0x8000];
	extern USize prgRomSize;
	
	extern USize nCycles;
	
	extern U8 n, v, d, i, z, c;
	
	U8 p(U8 b);
	void setP(U8 p);
	void setNZ(U8 data);
	
	extern U16 pc;
	
	extern U8 spl;
	
	extern U8 a, x, y;
	
	void setA(U8 data);
	void andA(U8 data);
	void orA(U8 data);
	void eorA(U8 data);
	void addA(U8 data);
	void subA(U8 data);
	void cmpA(U8 data);
	void bitA(U8 data);
	void setX(U8 data);
	void cmpX(U8 data);
	void setY(U8 data);
	void cmpY(U8 data);
	
	U8 inc(U8 data);
	U8 dec(U8 data);
	U8 shL(U8 data);
	U8 shR(U8 data);
	U8 roL(U8 data);
	U8 roR(U8 data);
	
	U8 read(U16 addr);
	U16 read16(U16 addr);
	U16 read16Zpg(U8 addr);
	void write(U16 addr, U8 data);
	
	void push(U8 data);
	void push16(U16 data);
	U8 pull();
	U16 pull16();
	
	void handleInt();
	
	void emuBBlock();
	
	void init();
	
	void run();
}
