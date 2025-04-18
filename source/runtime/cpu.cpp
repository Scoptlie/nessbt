#include "cpu.h"

#include <cstring>
#include <cstdio>

#include "runtime/controller.h"
#include "runtime/env.h"
#include "runtime/ppu.h"
#include "runtime/profiler.h"

namespace Cpu {
	U8 ram[0x800];
	
	USize nCycles;
	
	PT p;
	
	void setNZ(U8 data) {
		p.n = data >> 7;
		p.z = data == 0;
	}
	
	U16 pc;
	
	U8 spl;
	
	U8 a, x, y;
	
	void setA(U8 data) {
		a = data;
		setNZ(a);
	}
	
	void andA(U8 data) {
		setA(a & data);
	}
	
	void orA(U8 data) {
		setA(a | data);
	}
	
	void eorA(U8 data) {
		setA(a ^ data);
	}
	
	void addA(U8 data) {
		auto r = a + data + p.c;
		p.c = r > 255;
		p.v = ((~(a ^ data) & (r ^ data)) >> 7) & 1;
		setA(r);
	}
	
	void subA(U8 data) {
		addA(~data);
	}
	
	void cmpA(U8 data) {
		setNZ(a - data);
		p.c = a >= data;
	}
	
	void bitA(U8 data) {
		p.n = data >> 7;
		p.v = (data >> 6) & 1;
		p.z = (a & data) == 0;
	}
	
	void setX(U8 data) {
		x = data;
		setNZ(x);
	}
	
	void cmpX(U8 data) {
		setNZ(x - data);
		p.c = x >= data;
	}
	
	void setY(U8 data) {
		y = data;
		setNZ(y);
	}
	
	void cmpY(U8 data) {
		setNZ(y - data);
		p.c = y >= data;
	}
	
	U8 inc(U8 data) {
		setNZ(data + 1);
		return data + 1;
	}
	
	U8 dec(U8 data) {
		setNZ(data - 1);
		return data - 1;
	}
	
	U8 shL(U8 data) {
		auto r = data << 1;
		p.c = data >> 7;
		setNZ(r);
		return r;
	}
	
	U8 shR(U8 data) {
		auto r = data >> 1;
		p.c = data & 1;
		setNZ(r);
		return r;
	}
	
	U8 roL(U8 data) {
		auto r = (data << 1) | p.c;
		p.c = data >> 7;
		setNZ(r);
		return r;
	}
	
	U8 roR(U8 data) {
		auto r = (data >> 1) | (p.c << 7);
		p.c = data & 1;
		setNZ(r);
		return r;
	}
	
	U8 read(U16 addr) {
		if (addr <= 0x1fff) {
			return ram[addr];
		} else if (addr <= 0x3fff) {
			Env::update(nCycles);
			nCycles = 0;
			
			return Ppu::read(addr & 7);
		} else if (addr == 0x4016) {
			return Controller::data();
		} else if (addr >= 0x8000) {
			return prgRom[(addr & 0x7fff) & (prgRomSize - 1)];
		}
		return 0;
	}
	
	U16 read16(U16 addr) {
		return read(addr) | (read(addr + 1) << 8);
	}
	
	U16 read16Zpg(U8 addr) {
		return ram[addr] | (ram[U8(addr + 1)] << 8);
	}
	
	U8 readImm() {
		return read(pc++);
	}
	
	U16 readImm16() {
		auto r = read16(pc);
		pc += 2;
		return r;
	}
	
	U8 readZpg() {
		auto ea = readImm();
		return read(ea);
	}
	
	U8 readZpgX() {
		auto ea = U8(readImm() + x);
		return read(ea);
	}
	
	U8 readZpgY() {
		auto ea = U8(readImm() + y);
		return read(ea);
	}
	
	U8 readAbs() {
		auto ea = readImm16();
		return read(ea);
	}
	
	U8 readAbsX() {
		auto ea = readImm16() + x;
		return read(ea);
	}
	
	U8 readAbsY() {
		auto ea = readImm16() + y;
		return read(ea);
	}
	
	U8 readIndX() {
		auto ia = U8(readImm() + x);
		auto ea = read16Zpg(ia);
		return read(ea);
	}
	
	U8 readIndY() {
		auto ia = readImm();
		auto ea = read16Zpg(ia) + y;
		return read(ea);
	}
	
	void write(U16 addr, U8 data) {
		if (addr <= 0x1fff) {
			ram[addr] = data;
		} else if (addr <= 0x3fff) {
			Env::update(nCycles);
			nCycles = 0;
			
			Ppu::write(addr & 7, data);
		} else if (addr == 0x4014) {
			if (data < 8) {
				memcpy(Ppu::spriteRam, ram + (256 * data), 256);
				nCycles += 512;
			}
		} else if (addr == 0x4016) {
			Controller::strobe();
		}
	}
	
	void writeZpg(U8 data) {
		auto ea = readImm();
		write(ea, data);
	}
	
	void writeZpgX(U8 data) {
		auto ea = U8(readImm() + x);
		write(ea, data);
	}
	
	void writeZpgY(U8 data) {
		auto ea = U8(readImm() + y);
		write(ea, data);
	}
	
	void writeAbs(U8 data) {
		auto ea = readImm16();
		write(ea, data);
	}
	
	void writeAbsX(U8 data) {
		auto ea = readImm16() + x;
		write(ea, data);
	}
	
	void writeAbsY(U8 data) {
		auto ea = readImm16() + y;
		write(ea, data);
	}
	
	void writeIndX(U8 data) {
		auto ia = U8(readImm() + x);
		auto ea = read16Zpg(ia);
		write(ea, data);
	}
	
	void writeIndY(U8 data) {
		auto ia = readImm();
		auto ea = read16Zpg(ia) + y;
		write(ea, data);
	}
	
	void rmw(U16 addr, U8(*func)(U8)) {
		write(addr, func(read(addr)));
	}
	
	void rmwZpg(U8(*func)(U8)) {
		auto ea = readImm();
		rmw(ea, func);
	}
	
	void rmwZpgX(U8(*func)(U8)) {
		auto ea = U8(readImm() + x);
		rmw(ea, func);
	}
	
	void rmwZpgY(U8(*func)(U8)) {
		auto ea = U8(readImm() + y);
		rmw(ea, func);
	}
	
	void rmwAbs(U8(*func)(U8)) {
		auto ea = readImm16();
		rmw(ea, func);
	}
	
	void rmwAbsX(U8(*func)(U8)) {
		auto ea = readImm16() + x;
		rmw(ea, func);
	}
	
	void rmwAbsY(U8(*func)(U8)) {
		auto ea = readImm16() + y;
		rmw(ea, func);
	}
	
	void rmwIndX(U8(*func)(U8)) {
		auto ia = U8(readImm() + x);
		auto ea = read16Zpg(ia);
		rmw(ea, func);
	}
	
	void rmwIndY(U8(*func)(U8)) {
		auto ia = readImm();
		auto ea = read16Zpg(ia) + y;
		rmw(ea, func);
	}
	
	void push(U8 data) {
		write(0x0100 | spl--, data);
	}
	
	void push16(U16 data) {
		push(data >> 8);
		push(data & 0xff);
	}
	
	U8 pull() {
		return read(0x0100 | ++spl);
	}
	
	U16 pull16() {
		auto r = U16(pull());
		r |= pull() << 8;
		return r;
	}
	
	void pushP(U8 b) {
		p.b = b;
		push(*(U8*)&p);
		p.b = 1;
	}
	
	void pullP() {
		auto data = pull();
		p = *(PT*)&data;
		p.one = 1;
		p.b = 1;
	}
	
	void jumpInd(U16 addr) {
		pc = read(addr);
		pc |= read((addr & 0xff00) | ((addr + 1) & 0x00ff)) << 8;
	}
	
	void jumpSub(U16 addr) {
		push16(pc - 1);
		pc = addr;
	}
	
	void jumpInt(U8 b) {
		push16(pc + b);
		pushP(b);
		p.i = 1;
		pc = read16(Ppu::nmi? 0xfffa : 0xfffe);
		Ppu::nmi = false;
	}
	
	void retSub() {
		pc = pull16() + 1;
	}
	
	void retInt() {
		pullP();
		pc = pull16();
	}
	
	bool branch(bool cond) {
		auto offs = I8(readImm());
		if (cond) {
			pc += offs;
			return true;
		}
		return false;
	}
	
	void emuInstr() {
		auto opcode = readImm();
		
		switch (opcode) {
		case 0x00: { jumpInt(1); nCycles += 7; tailCall(runBBlockDyn()); }
		case 0x01: { orA(readIndX()); nCycles += 6; tailCall(emuInstr()); }
		case 0x02: { nCycles += 2; tailCall(emuInstr()); }
		case 0x03: { nCycles += 2; tailCall(emuInstr()); }
		case 0x04: { nCycles += 2; tailCall(emuInstr()); }
		case 0x05: { orA(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0x06: { rmwZpg(shL); nCycles += 5; tailCall(emuInstr()); }
		case 0x07: { nCycles += 2; tailCall(emuInstr()); }
		case 0x08: { pushP(1); nCycles += 3; tailCall(emuInstr()); }
		case 0x09: { orA(readImm()); nCycles += 2; tailCall(emuInstr()); }
		case 0x0a: { a = shL(a); nCycles += 2; tailCall(emuInstr()); }
		case 0x0b: { nCycles += 2; tailCall(emuInstr()); }
		case 0x0c: { nCycles += 2; tailCall(emuInstr()); }
		case 0x0d: { orA(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0x0e: { rmwAbs(shL); nCycles += 6; tailCall(emuInstr()); }
		case 0x0f: { nCycles += 2; tailCall(emuInstr()); }
		case 0x10: { nCycles += branch(!p.n)? 3: 2; tailCall(runBBlockDyn()); }
		case 0x11: { orA(readIndY()); nCycles += 5; tailCall(emuInstr()); }
		case 0x12: { nCycles += 2; tailCall(emuInstr()); }
		case 0x13: { nCycles += 2; tailCall(emuInstr()); }
		case 0x14: { nCycles += 2; tailCall(emuInstr()); }
		case 0x15: { orA(readZpgX()); nCycles += 4; tailCall(emuInstr()); }
		case 0x16: { rmwZpgX(shL); nCycles += 6; tailCall(emuInstr()); }
		case 0x17: { nCycles += 2; tailCall(emuInstr()); }
		case 0x18: { p.c = 0; nCycles += 2; tailCall(emuInstr()); }
		case 0x19: { orA(readAbsY()); nCycles += 4; tailCall(emuInstr()); }
		case 0x1a: { nCycles += 2; tailCall(emuInstr()); }
		case 0x1b: { nCycles += 2; tailCall(emuInstr()); }
		case 0x1c: { nCycles += 2; tailCall(emuInstr()); }
		case 0x1d: { orA(readAbsX()); nCycles += 4; tailCall(emuInstr()); }
		case 0x1e: { rmwAbsX(shL); nCycles += 7; tailCall(emuInstr()); }
		case 0x1f: { nCycles += 2; tailCall(emuInstr()); }
		case 0x20: { jumpSub(readImm16()); nCycles += 6; tailCall(runBBlockDyn()); }
		case 0x21: { andA(readIndX()); nCycles += 6; tailCall(emuInstr()); }
		case 0x22: { nCycles += 2; tailCall(emuInstr()); }
		case 0x23: { nCycles += 2; tailCall(emuInstr()); }
		case 0x24: { bitA(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0x25: { andA(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0x26: { rmwZpg(roL); nCycles += 5; tailCall(emuInstr()); }
		case 0x27: { nCycles += 2; tailCall(emuInstr()); }
		case 0x28: { pullP(); nCycles += 4; tailCall(emuInstr()); }
		case 0x29: { andA(readImm()); nCycles += 2; tailCall(emuInstr()); }
		case 0x2a: { a = roL(a); nCycles += 2; tailCall(emuInstr()); }
		case 0x2b: { nCycles += 2; tailCall(emuInstr()); }
		case 0x2c: { bitA(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0x2d: { andA(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0x2e: { rmwAbs(roL); nCycles += 6; tailCall(emuInstr()); }
		case 0x2f: { nCycles += 2; tailCall(emuInstr()); }
		case 0x30: { nCycles += branch(p.n)? 3: 2; tailCall(runBBlockDyn()); }
		case 0x31: { andA(readIndY()); nCycles += 5; tailCall(emuInstr()); }
		case 0x32: { nCycles += 2; tailCall(emuInstr()); }
		case 0x33: { nCycles += 2; tailCall(emuInstr()); }
		case 0x34: { nCycles += 2; tailCall(emuInstr()); }
		case 0x35: { andA(readZpgX()); nCycles += 4; tailCall(emuInstr()); }
		case 0x36: { rmwZpgX(roL); nCycles += 6; tailCall(emuInstr()); }
		case 0x37: { nCycles += 2; tailCall(emuInstr()); }
		case 0x38: { p.c = 1; nCycles += 2; tailCall(emuInstr()); }
		case 0x39: { andA(readAbsY()); nCycles += 4; tailCall(emuInstr()); }
		case 0x3a: { nCycles += 2; tailCall(emuInstr()); }
		case 0x3b: { nCycles += 2; tailCall(emuInstr()); }
		case 0x3c: { nCycles += 2; tailCall(emuInstr()); }
		case 0x3d: { andA(readAbsX()); nCycles += 4; tailCall(emuInstr()); }
		case 0x3e: { rmwAbsX(roL); nCycles += 7; tailCall(emuInstr()); }
		case 0x3f: { nCycles += 2; tailCall(emuInstr()); }
		case 0x40: { retInt(); nCycles += 6; tailCall(runBBlockDyn()); }
		case 0x41: { eorA(readIndX()); nCycles += 6; tailCall(emuInstr()); }
		case 0x42: { nCycles += 2; tailCall(emuInstr()); }
		case 0x43: { nCycles += 2; tailCall(emuInstr()); }
		case 0x44: { nCycles += 2; tailCall(emuInstr()); }
		case 0x45: { eorA(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0x46: { rmwZpg(shR); nCycles += 5; tailCall(emuInstr()); }
		case 0x47: { nCycles += 2; tailCall(emuInstr()); }
		case 0x48: { push(a); nCycles += 3; tailCall(emuInstr()); }
		case 0x49: { eorA(readImm()); nCycles += 2; tailCall(emuInstr()); }
		case 0x4a: { a = shR(a); nCycles += 2; tailCall(emuInstr()); }
		case 0x4b: { nCycles += 2; tailCall(emuInstr()); }
		case 0x4c: { pc = readImm16(); nCycles += 3; tailCall(runBBlockDyn()); }
		case 0x4d: { eorA(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0x4e: { rmwAbs(shR); nCycles += 6; tailCall(emuInstr()); }
		case 0x4f: { nCycles += 2; tailCall(emuInstr()); }
		case 0x50: { nCycles += branch(!p.v)? 3: 2; tailCall(runBBlockDyn()); }
		case 0x51: { eorA(readIndY()); nCycles += 5; tailCall(emuInstr()); }
		case 0x52: { nCycles += 2; tailCall(emuInstr()); }
		case 0x53: { nCycles += 2; tailCall(emuInstr()); }
		case 0x54: { nCycles += 2; tailCall(emuInstr()); }
		case 0x55: { eorA(readZpgX()); nCycles += 4; tailCall(emuInstr()); }
		case 0x56: { rmwZpgX(shR); nCycles += 6; tailCall(emuInstr()); }
		case 0x57: { nCycles += 2; tailCall(emuInstr()); }
		case 0x58: { p.i = 0; nCycles += 2; tailCall(emuInstr()); }
		case 0x59: { eorA(readAbsY()); nCycles += 4; tailCall(emuInstr()); }
		case 0x5a: { nCycles += 2; tailCall(emuInstr()); }
		case 0x5b: { nCycles += 2; tailCall(emuInstr()); }
		case 0x5c: { nCycles += 2; tailCall(emuInstr()); }
		case 0x5d: { eorA(readAbsX()); nCycles += 4; tailCall(emuInstr()); }
		case 0x5e: { rmwAbsX(shR); nCycles += 7; tailCall(emuInstr()); }
		case 0x5f: { nCycles += 2; tailCall(emuInstr()); }
		case 0x60: { retSub(); nCycles += 6; tailCall(runBBlockDyn()); }
		case 0x61: { addA(readIndX()); nCycles += 6; tailCall(emuInstr()); }
		case 0x62: { nCycles += 2; tailCall(emuInstr()); }
		case 0x63: { nCycles += 2; tailCall(emuInstr()); }
		case 0x64: { nCycles += 2; tailCall(emuInstr()); }
		case 0x65: { addA(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0x66: { rmwZpg(roR); nCycles += 5; tailCall(emuInstr()); }
		case 0x67: { nCycles += 2; tailCall(emuInstr()); }
		case 0x68: { setA(pull()); nCycles += 4; tailCall(emuInstr()); }
		case 0x69: { addA(readImm()); nCycles += 2; tailCall(emuInstr()); }
		case 0x6a: { a = roR(a); nCycles += 2; tailCall(emuInstr()); }
		case 0x6b: { nCycles += 2; tailCall(emuInstr()); }
		case 0x6c: { jumpInd(readImm16()); nCycles += 5; tailCall(runBBlockDyn()); }
		case 0x6d: { addA(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0x6e: { rmwAbs(roR); nCycles += 6; tailCall(emuInstr()); }
		case 0x6f: { nCycles += 2; tailCall(emuInstr()); }
		case 0x70: { nCycles += branch(p.v)? 3: 2; tailCall(runBBlockDyn()); }
		case 0x71: { addA(readIndY()); nCycles += 5; tailCall(emuInstr()); }
		case 0x72: { nCycles += 2; tailCall(emuInstr()); }
		case 0x73: { nCycles += 2; tailCall(emuInstr()); }
		case 0x74: { nCycles += 2; tailCall(emuInstr()); }
		case 0x75: { addA(readZpgX()); nCycles += 4; tailCall(emuInstr()); }
		case 0x76: { rmwZpgX(roR); nCycles += 6; tailCall(emuInstr()); }
		case 0x77: { nCycles += 2; tailCall(emuInstr()); }
		case 0x78: { p.i = 1; nCycles += 2; tailCall(emuInstr()); }
		case 0x79: { addA(readAbsY()); nCycles += 4; tailCall(emuInstr()); }
		case 0x7a: { nCycles += 2; tailCall(emuInstr()); }
		case 0x7b: { nCycles += 2; tailCall(emuInstr()); }
		case 0x7c: { nCycles += 2; tailCall(emuInstr()); }
		case 0x7d: { addA(readAbsX()); nCycles += 4; tailCall(emuInstr()); }
		case 0x7e: { rmwAbsX(roR); nCycles += 7; tailCall(emuInstr()); }
		case 0x7f: { nCycles += 2; tailCall(emuInstr()); }
		case 0x80: { nCycles += 2; tailCall(emuInstr()); }
		case 0x81: { writeIndX(a); nCycles += 6; tailCall(emuInstr()); }
		case 0x82: { nCycles += 2; tailCall(emuInstr()); }
		case 0x83: { nCycles += 2; tailCall(emuInstr()); }
		case 0x84: { writeZpg(y); nCycles += 3; tailCall(emuInstr()); }
		case 0x85: { writeZpg(a); nCycles += 3; tailCall(emuInstr()); }
		case 0x86: { writeZpg(x); nCycles += 3; tailCall(emuInstr()); }
		case 0x87: { nCycles += 2; tailCall(emuInstr()); }
		case 0x88: { setY(y - 1); nCycles += 2; tailCall(emuInstr()); }
		case 0x89: { nCycles += 2; tailCall(emuInstr()); }
		case 0x8a: { setA(x); nCycles += 2; tailCall(emuInstr()); }
		case 0x8b: { nCycles += 2; tailCall(emuInstr()); }
		case 0x8c: { writeAbs(y); nCycles += 4; tailCall(emuInstr()); }
		case 0x8d: { writeAbs(a); nCycles += 4; tailCall(emuInstr()); }
		case 0x8e: { writeAbs(x); nCycles += 4; tailCall(emuInstr()); }
		case 0x8f: { nCycles += 2; tailCall(emuInstr()); }
		case 0x90: { nCycles += branch(!p.c)? 3: 2; tailCall(runBBlockDyn()); }
		case 0x91: { writeIndY(a); nCycles += 6; tailCall(emuInstr()); }
		case 0x92: { nCycles += 2; tailCall(emuInstr()); }
		case 0x93: { nCycles += 2; tailCall(emuInstr()); }
		case 0x94: { writeZpgX(y); nCycles += 4; tailCall(emuInstr()); }
		case 0x95: { writeZpgX(a); nCycles += 4; tailCall(emuInstr()); }
		case 0x96: { writeZpgY(x); nCycles += 4; tailCall(emuInstr()); }
		case 0x97: { nCycles += 2; tailCall(emuInstr()); }
		case 0x98: { setA(y); nCycles += 2; tailCall(emuInstr()); }
		case 0x99: { writeAbsY(a); nCycles += 5; tailCall(emuInstr()); }
		case 0x9a: { spl = x; nCycles += 2; tailCall(emuInstr()); }
		case 0x9b: { nCycles += 2; tailCall(emuInstr()); }
		case 0x9c: { nCycles += 2; tailCall(emuInstr()); }
		case 0x9d: { writeAbsX(a); nCycles += 5; tailCall(emuInstr()); }
		case 0x9e: { nCycles += 2; tailCall(emuInstr()); }
		case 0x9f: { nCycles += 2; tailCall(emuInstr()); }
		case 0xa0: { setY(readImm()); nCycles += 2; tailCall(emuInstr()); }
		case 0xa1: { setA(readIndX()); nCycles += 6; tailCall(emuInstr()); }
		case 0xa2: { setX(readImm()); nCycles += 2; tailCall(emuInstr()); }
		case 0xa3: { nCycles += 2; tailCall(emuInstr()); }
		case 0xa4: { setY(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0xa5: { setA(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0xa6: { setX(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0xa7: { nCycles += 2; tailCall(emuInstr()); }
		case 0xa8: { setY(a); nCycles += 2; tailCall(emuInstr()); }
		case 0xa9: { setA(readImm()); nCycles += 2; tailCall(emuInstr()); }
		case 0xaa: { setX(a); nCycles += 2; tailCall(emuInstr()); }
		case 0xab: { nCycles += 2; tailCall(emuInstr()); }
		case 0xac: { setY(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0xad: { setA(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0xae: { setX(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0xaf: { nCycles += 2; tailCall(emuInstr()); }
		case 0xb0: { nCycles += branch(p.c)? 3: 2; tailCall(runBBlockDyn()); }
		case 0xb1: { setA(readIndY()); nCycles += 5; tailCall(emuInstr()); }
		case 0xb2: { nCycles += 2; tailCall(emuInstr()); }
		case 0xb3: { nCycles += 2; tailCall(emuInstr()); }
		case 0xb4: { setY(readZpgX()); nCycles += 4; tailCall(emuInstr()); }
		case 0xb5: { setA(readZpgX()); nCycles += 4; tailCall(emuInstr()); }
		case 0xb6: { setX(readZpgY()); nCycles += 4; tailCall(emuInstr()); }
		case 0xb7: { nCycles += 2; tailCall(emuInstr()); }
		case 0xb8: { p.v = 0; nCycles += 2; tailCall(emuInstr()); }
		case 0xb9: { setA(readAbsY()); nCycles += 4; tailCall(emuInstr()); }
		case 0xba: { setX(spl); nCycles += 2; tailCall(emuInstr()); }
		case 0xbb: { nCycles += 2; tailCall(emuInstr()); }
		case 0xbc: { setY(readAbsX()); nCycles += 4; tailCall(emuInstr()); }
		case 0xbd: { setA(readAbsX()); nCycles += 4; tailCall(emuInstr()); }
		case 0xbe: { setX(readAbsY()); nCycles += 4; tailCall(emuInstr()); }
		case 0xbf: { nCycles += 2; tailCall(emuInstr()); }
		case 0xc0: { cmpY(readImm()); nCycles += 2; tailCall(emuInstr()); }
		case 0xc1: { cmpA(readIndX()); nCycles += 6; tailCall(emuInstr()); }
		case 0xc2: { nCycles += 2; tailCall(emuInstr()); }
		case 0xc3: { nCycles += 2; tailCall(emuInstr()); }
		case 0xc4: { cmpY(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0xc5: { cmpA(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0xc6: { rmwZpg(dec); nCycles += 5; tailCall(emuInstr()); }
		case 0xc7: { nCycles += 2; tailCall(emuInstr()); }
		case 0xc8: { setY(y + 1); nCycles += 2; tailCall(emuInstr()); }
		case 0xc9: { cmpA(readImm()); nCycles += 2; tailCall(emuInstr()); }
		case 0xca: { setX(x - 1); nCycles += 2; tailCall(emuInstr()); }
		case 0xcb: { nCycles += 2; tailCall(emuInstr()); }
		case 0xcc: { cmpY(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0xcd: { cmpA(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0xce: { rmwAbs(dec); nCycles += 6; tailCall(emuInstr()); }
		case 0xcf: { nCycles += 2; tailCall(emuInstr()); }
		case 0xd0: { nCycles += branch(!p.z)? 3: 2; tailCall(runBBlockDyn()); }
		case 0xd1: { cmpA(readIndY()); nCycles += 5; tailCall(emuInstr()); }
		case 0xd2: { nCycles += 2; tailCall(emuInstr()); }
		case 0xd3: { nCycles += 2; tailCall(emuInstr()); }
		case 0xd4: { nCycles += 2; tailCall(emuInstr()); }
		case 0xd5: { cmpA(readZpgX()); nCycles += 4; tailCall(emuInstr()); }
		case 0xd6: { rmwZpgX(dec); nCycles += 6; tailCall(emuInstr()); }
		case 0xd7: { nCycles += 2; tailCall(emuInstr()); }
		case 0xd8: { p.d = 0; nCycles += 2; tailCall(emuInstr()); }
		case 0xd9: { cmpA(readAbsY()); nCycles += 4; tailCall(emuInstr()); }
		case 0xda: { nCycles += 2; tailCall(emuInstr()); }
		case 0xdb: { nCycles += 2; tailCall(emuInstr()); }
		case 0xdc: { nCycles += 2; tailCall(emuInstr()); }
		case 0xdd: { cmpA(readAbsX()); nCycles += 4; tailCall(emuInstr()); }
		case 0xde: { rmwAbsX(dec); nCycles += 7; tailCall(emuInstr()); }
		case 0xdf: { nCycles += 2; tailCall(emuInstr()); }
		case 0xe0: { cmpX(readImm()); nCycles += 2; tailCall(emuInstr()); }
		case 0xe1: { subA(readIndX()); nCycles += 6; tailCall(emuInstr()); }
		case 0xe2: { nCycles += 2; tailCall(emuInstr()); }
		case 0xe3: { nCycles += 2; tailCall(emuInstr()); }
		case 0xe4: { cmpX(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0xe5: { subA(readZpg()); nCycles += 3; tailCall(emuInstr()); }
		case 0xe6: { rmwZpg(inc); nCycles += 5; tailCall(emuInstr()); }
		case 0xe7: { nCycles += 2; tailCall(emuInstr()); }
		case 0xe8: { setX(x + 1); nCycles += 2; tailCall(emuInstr()); }
		case 0xe9: { subA(readImm()); nCycles += 2; tailCall(emuInstr()); }
		case 0xea: { nCycles += 2; tailCall(emuInstr()); }
		case 0xeb: { nCycles += 2; tailCall(emuInstr()); }
		case 0xec: { cmpX(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0xed: { subA(readAbs()); nCycles += 4; tailCall(emuInstr()); }
		case 0xee: { rmwAbs(inc); nCycles += 6; tailCall(emuInstr()); }
		case 0xef: { nCycles += 2; tailCall(emuInstr()); }
		case 0xf0: { nCycles += branch(p.z)? 3: 2; tailCall(runBBlockDyn()); }
		case 0xf1: { subA(readIndY()); nCycles += 5; tailCall(emuInstr()); }
		case 0xf2: { nCycles += 2; tailCall(emuInstr()); }
		case 0xf3: { nCycles += 2; tailCall(emuInstr()); }
		case 0xf4: { nCycles += 2; tailCall(emuInstr()); }
		case 0xf5: { subA(readZpgX()); nCycles += 4; tailCall(emuInstr()); }
		case 0xf6: { rmwZpgX(inc); nCycles += 6; tailCall(emuInstr()); }
		case 0xf7: { nCycles += 2; tailCall(emuInstr()); }
		case 0xf8: { p.d = 1; nCycles += 2; tailCall(emuInstr()); }
		case 0xf9: { subA(readAbsY()); nCycles += 4; tailCall(emuInstr()); }
		case 0xfa: { nCycles += 2; tailCall(emuInstr()); }
		case 0xfb: { nCycles += 2; tailCall(emuInstr()); }
		case 0xfc: { nCycles += 2; tailCall(emuInstr()); }
		case 0xfd: { subA(readAbsX()); nCycles += 4; tailCall(emuInstr()); }
		case 0xfe: { rmwAbsX(inc); nCycles += 7; tailCall(emuInstr()); }
		case 0xff: { nCycles += 2; tailCall(emuInstr()); }
		}
	}
	
	void runBBlockDyn() {
#ifdef onlyUseTranslations
		tailCall(stBBlockFuncs[pc & 0x7fff]());
#elif !defined(onlyUseEmulation)
		auto f = stBBlockFuncs[pc & 0x7fff];
		if (f) {
			tailCall(f());
		} else {
#endif
			Profiler::addJumpTarget(pc);
			
			Env::update(nCycles);
			nCycles = 0;
			
			if (Ppu::nmi) {
				Profiler::addNmiLocation(pc);
				
				jumpInt(0);
				nCycles += 7;
				tailCall(runBBlockDyn());
			}
			
			tailCall(emuInstr());
#if !defined(onlyUseEmulation) && !defined(onlyUseTranslations)
		}
#endif
	}
	
	void init() {
		memset(ram, 0, sizeof(ram));
		
		nCycles = 0;
		
		pc = read16(0xfffc);
		spl = 0xfd;
		p.n = 0;
		p.v = 0;
		p.one = 1;
		p.b = 1;
		p.d = 0;
		p.i = 1;
		p.z = 0;
		p.c = 0;
		a = 0;
		x = 0;
		y = 0;
	}
	
	void run() {
		init();
		
		runBBlockDyn();
	}
}
