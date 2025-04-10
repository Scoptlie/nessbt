#pragma once

#include "runtime/common.h"

namespace Cpu {
	extern U8 ram[0x800];
	
	extern U8 prgRom[0x8000];
	extern USize prgRomSize;
	
	extern void (*stBBlockFuncs[0x8000])();
	
	extern USize nCycles;
	
	struct PT {
		U8 c : 1,
			z : 1,
			i : 1,
			d : 1,
			b : 1,
			one : 1,
			v : 1,
			n : 1;
	};
	
	extern PT p;
	
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
	void rmw(U16 addr, U8(*func)(U8));
	
	void push(U8 data);
	void push16(U16 data);
	U8 pull();
	U16 pull16();
	
	void pushP(U8 b);
	void pullP();
	
	void jumpInd(U16 addr);
	void jumpSub(U16 addr);
	void jumpInt(U8 b);
	void retSub();
	void retInt();
	
	void handleInt();
	
	void runBBlockDyn();
	
	void init();
	
	void run();
}
