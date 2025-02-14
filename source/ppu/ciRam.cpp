#include "CiRam.h"

#include <cstring>

#include "cart.h"

namespace Ppu::CiRam {
	U8 content[0x800];
	
	U16 addrRemapped(U16 addr) {
		auto low = addr & 0b1111111111;
		
		auto high = addr;
		if (!Cart::mirrorV) {
			high >>= 1;
		}
		high &= 0b10000000000;
		
		return low | high;
	}
	
	U8 read(U16 addr) {
		return content[addrRemapped(addr)];
	}
	
	void write(U16 addr, U8 data) {
		content[addrRemapped(addr)] = data;
	}
	
	void init() {
		memset(content, 0, sizeof(content));
	}
}
