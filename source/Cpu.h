#pragma once

#include "Common.h"

namespace Cpu {
	extern U8 ram[0x800];
	
	extern U64 nCycle;
	
	extern U8 n, v, d, i, z, c;
	
	U8 p(U8 b);
	void setP(U8 p);
	
	extern U8 pcl, pch;
	
	U16 pc();
	
	extern U8 spl;
	extern U8 a;
	extern U8 x, y;
	
	void reset();
	void step();
	
	void init();
	void deinit();
};
