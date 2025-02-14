#pragma once

#include "common.h"

namespace Cpu {
	extern U8 ram[0x800];
	
	extern USize nCycles;
	
	extern U8 n, v, d, i, z, c;
	extern U16 pc;
	extern U8 spl;
	extern U8 a, x, y;
	
	U8 read(U16 addr);
	U16 read16(U16 addr);
	U16 read16Zpg(U8 addr);
	void write(U16 addr, U8 data);
	
	void push(U8 data);
	void push16(U16 data);
	U8 pull();
	U16 pull16();
	
	void handleInt();
	
	void emuInstr();
	
	void init();
}
