#include "Cpu.h"

#include <cstring>

#include "Cart.h"
#include "Controller.h"
#include "Ppu.h"

namespace Cpu {
	U8 ram[0x800];
	
	U64 nCycle;
	
	U8 n, v, d, i, z, c;
	
	U8 p(U8 b) {
		return
			n << 7 |
			v << 6 |
			b << 4 |
			d << 3 |
			i << 2 |
			z << 1 |
			c;
	}
	
	void setP(U8 p) {
		n = p >> 7;
		v = (p >> 6) & 1;
		d = (p >> 3) & 1;
		i = (p >> 2) & 1;
		z = (p >> 1) & 1;
		c = p & 1;
	}
	
	static void setNZ(U8 data) {
		n = data >> 7;
		z = (data == 0);
	}
	
	U8 pcl, pch;
	
	U16 pc() {
		return pcl | (pch << 8);
	}
	
	static void incPc() {
		pcl++;
		if (pcl == 0) {
			pch++;
		}
	}
	
	U8 spl;
	
	static U16 sp() {
		return spl | 0x0100;
	}
	
	U8 a;
	
	static void setA(U8 data) {
		a = data;
		setNZ(a);
	}
	
	static void andA(U8 data) {
		a &= data;
		setNZ(a);
	}
	
	static void orA(U8 data) {
		a |= data;
		setNZ(a);
	}
	
	static void eorA(U8 data) {
		a ^= data;
		setNZ(a);
	}
	
	static void addA(U8 data) {
		U16 r = a + data + c;
		c = r > 255;
		v = ((~(a ^ data) & (r ^ data)) >> 7) & 1;
		setA(r);
	}
	
	static void subA(U8 data) {
		addA(~data);
	}
	
	static void cmpA(U8 data) {
		setNZ(a - data);
		c = a >= data;
	}
	
	static void bitA(U8 data) {
		n = data >> 7;
		v = (data >> 6) & 1;
		z = (a & data) == 0;
	}
	
	U8 x, y;
	
	static void setX(U8 data) {
		x = data;
		setNZ(x);
	}
	
	static void cmpX(U8 data) {
		setNZ(x - data);
		c = x >= data;
	}
	
	static void setY(U8 data) {
		y = data;
		setNZ(y);
	}
	
	static void cmpY(U8 data) {
		setNZ(y - data);
		c = y >= data;
	}
	
	static U8 inc(U8 data) {
		setNZ(data + 1);
		return data + 1;
	}
	
	static U8 dec(U8 data) {
		setNZ(data - 1);
		return data - 1;
	}
	
	static U8 shiftL(U8 data) {
		U8 r = data << 1;
		c = data >> 7;
		setNZ(data);
		return r;
	}
	
	static U8 shiftR(U8 data) {
		U8 r = data >> 1;
		c = data & 1;
		setNZ(data);
		return r;
	}
	
	static U8 rotL(U8 data) {
		U8 r = (data << 1) | c;
		c = data >> 7;
		setNZ(data);
		return r;
	}
	
	static U8 rotR(U8 data) {
		U8 r = (data >> 1) | (c << 7);
		c = data & 1;
		setNZ(data);
		return r;
	}
	
	static void tick() {
		nCycle++;
	}
	
	static U8 mdr;
	
	static U8 read(U16 addr) {
		tick();
		
		if (addr <= 0x1fff) {
			mdr = ram[addr & 0x7ff];
		} else if (addr <= 0x3fff) {
			mdr = Ppu::read(addr & 7);
		} else if (addr == 0x4016) {
			mdr = Controller::data();
		} else if (addr >= 0x8000) {
			mdr = Cart::cpuRead(addr & 0x7fff);
		}
		
		return mdr;
	}
	
	static U8 readImm() {
		U8 r = read(pc());
		incPc();
		return r;
	}
	
	static U8 readZpg() {
		U16 ea = readImm();
		return read(ea);
	}
	
	static U8 readZpgX() {
		U16 ea = readImm();
		read(ea);
		return read((ea + x) & 0xff);
	}
	
	static U8 readZpgY() {
		U16 ea = readImm();
		read(ea);
		return read((ea + y) & 0xff);
	}
	
	static U8 readAbs() {
		U8 eal = readImm();
		U8 eah = readImm();
		return read(eal | (eah << 8));
	}
	
	static U8 readAbsX() {
		U8 eal = readImm() + x;
		U8 eah = readImm();
		U8 r = read(eal | (eah << 8));
		if (eal < x) {
			r = read(eal | ((eah + 1) << 8));
		}
		return r;
	}
	
	static U8 readAbsY() {
		U8 eal = readImm() + y;
		U8 eah = readImm();
		U8 r = read(eal | (eah << 8));
		if (eal < y) {
			r = read(eal | ((eah + 1) << 8));
		}
		return r;
	}
	
	static U8 readIndX() {
		U16 ia = readImm();
		read(ia);
		U8 eal = read((ia + x) & 0xff);
		U8 eah = read((ia + x + 1) & 0xff);
		return read(eal | (eah << 8));
	}
	
	static U8 readIndY() {
		U16 ia = readImm();
		U8 eal = read(ia) + y;
		U8 eah = read((ia + 1) & 0xff);
		U8 r = read(eal | (eah << 8));
		if (eal < y) {
			r = read(eal | ((eah + 1) << 8));
		}
		return r;
	}
	
	static void write(U16 addr, U8 data) {
		tick();
		
		mdr = data;
		
		if (addr <= 0x1fff) {
			ram[addr & 0x7ff] = data;
		} else if (addr <= 0x3fff) {
			Ppu::write(addr & 7, data);
		} else if (addr == 0x4014) {
			auto dmaAddr = 0x100 * data;
			for (auto i = 0; i < 256; i++) {
				auto v = read(dmaAddr + i);
				write(0x2004, v);
			}
		} else if (addr == 0x4016) {
			Controller::strobe();
		}
	}
	
	static void writeZpg(U8 data) {
		U16 ea = readImm();
		write(ea, data);
	}
	
	static void writeZpgX(U8 data) {
		U16 ea = readImm();
		read(ea);
		write((ea + x) & 0xff, data);
	}
	
	static void writeZpgY(U8 data) {
		U16 ea = readImm();
		read(ea);
		write((ea + y) & 0xff, data);
	}
	
	static void writeAbs(U8 data) {
		U8 eal = readImm();
		U8 eah = readImm();
		write(eal | (eah << 8), data);
	}
	
	static void writeAbsX(U8 data) {
		U8 eal = readImm() + x;
		U8 eah = readImm();
		read(eal | (eah << 8));
		if (eal < x) {
			eah++;
		}
		write(eal | (eah << 8), data);
	}
	
	static void writeAbsY(U8 data) {
		U8 eal = readImm() + y;
		U8 eah = readImm();
		read(eal | (eah << 8));
		if (eal < y) {
			eah++;
		}
		write(eal | (eah << 8), data);
	}
	
	static void writeIndX(U8 data) {
		U16 ia = readImm();
		read(ia);
		U8 eal = read((ia + x) & 0xff);
		U8 eah = read((ia + x + 1) & 0xff);
		write(eal | (eah << 8), data);
	}
	
	static void writeIndY(U8 data) {
		U16 ia = readImm();
		U8 eal = read(ia) + y;
		U8 eah = read((ia + 1) & 0xff);
		read(eal | (eah << 8));
		if (eal < y) {
			eah++;
		}
		write(eal | (eah << 8), data);
	}
	
	static void rmw(U16 addr, U8 (*func)(U8)) {
		U8 data = read(addr);
		write(addr, data);
		write(addr, func(data));
	}
	
	static void rmwZpg(U8 (*func)(U8)) {
		U16 ea = readImm();
		rmw(ea, func);
	}
	
	static void rmwZpgX(U8 (*func)(U8)) {
		U16 ea = readImm();
		read(ea);
		rmw((ea + x) & 0xff, func);
	}
	
	static void rmwZpgY(U8 (*func)(U8)) {
		U16 ea = readImm();
		read(ea);
		rmw((ea + y) & 0xff, func);
	}
	
	static void rmwAbs(U8 (*func)(U8)) {
		U8 eal = readImm();
		U8 eah = readImm();
		rmw(eal | (eah << 8), func);
	}
	
	static void rmwAbsX(U8 (*func)(U8)) {
		U8 eal = readImm() + x;
		U8 eah = readImm();
		read(eal | (eah << 8));
		if (eal < x) {
			eah++;
		}
		rmw(eal | (eah << 8), func);
	}
	
	static void rmwAbsY(U8 (*func)(U8)) {
		U8 eal = readImm() + y;
		U8 eah = readImm();
		read(eal | (eah << 8));
		if (eal < y) {
			eah++;
		}
		rmw(eal | (eah << 8), func);
	}
	
	static void push(U8 data) {
		read(pc());
		write(sp(), data);
		spl--;
	}
	
	static U8 pull() {
		read(pc());
		read(sp());
		spl++;
		return read(sp());
	}
	
	static void jumpAbs() {
		U8 eal = readImm();
		U8 eah = readImm();
		pcl = eal;
		pch = eah;
	}
	
	static void jumpInd() {
		U8 ial = readImm();
		U8 iah = readImm();
		pcl = read(ial | (iah << 8));
		pch = read((ial + 1) | (iah << 8));
	}
	
	static void jumpSub() {
		U8 eal = readImm();
		
		read(sp());
		write(sp(), pch);
		spl--;
		write(sp(), pcl);
		spl--;
		
		U8 eah = readImm();
		pcl = eal;
		pch = eah;
	}
	
	static void jumpInt(U8 b) {
		read(pc());
		if (b) { incPc(); }
		
		write(sp(), pch);
		spl--;
		write(sp(), pcl);
		spl--;
		
		U16 vec = 0xfffe;
		if (Ppu::nmi) {
			vec = 0xfffa;
			Ppu::nmi = false;
		}
		
		write(sp(), p(b));
		spl--;
		
		i = 1;
		pcl = read(vec);
		pch = read(vec + 1);
	}
	
	static void retSub() {
		read(pc());
		
		read(sp());
		spl++;
		pcl = read(sp());
		spl++;
		pch = read(sp());
		
		readImm();
	}
	
	static void retInt() {
		read(pc());
		
		read(sp());
		spl++;
		setP(read(sp()));
		spl++;
		pcl = read(sp());
		spl++;
		pch = read(sp());
	}
	
	static void branch(bool cond) {
		U8 offs = readImm();
		if (cond) {
			read(pc());
			pcl += offs;
			if ((pcl >= offs) != (offs < 128)) {
				read(pc());
				if (offs < 128) {
					pch++;
				} else {
					pch--;
				}
			}
		}
	}
	
	void reset() {
		read(pc());
		read(pc());
		
		read(sp());
		spl--;
		read(sp());
		spl--;
		read(sp());
		spl--;
		
		i = 1;
		pcl = read(0xfffc);
		pch = read(0xfffd);
	}
	
	void step() {
		if (Ppu::nmi) {
			jumpInt(0);
			return;
		}
		
		U8 opcode = readImm();
		
		switch (opcode) {
		case 0x00: { jumpInt(1); break; }
		case 0x01: { orA(readIndX()); break; }
		case 0x02: { read(pc()); break; }
		case 0x03: { read(pc()); break; }
		case 0x04: { read(pc()); break; }
		case 0x05: { orA(readZpg()); break; }
		case 0x06: { rmwZpg(shiftL); break; }
		case 0x07: { read(pc()); break; }
		case 0x08: { push(p(1)); break; }
		case 0x09: { orA(readImm()); break; }
		case 0x0a: { read(pc()); a = shiftL(a); break; }
		case 0x0b: { read(pc()); break; }
		case 0x0c: { read(pc()); break; }
		case 0x0d: { orA(readAbs()); break; }
		case 0x0e: { rmwAbs(shiftL); break; }
		case 0x0f: { read(pc()); break; }
		case 0x10: { branch(n == 0); break; }
		case 0x11: { orA(readIndY()); break; }
		case 0x12: { read(pc()); break; }
		case 0x13: { read(pc()); break; }
		case 0x14: { read(pc()); break; }
		case 0x15: { orA(readZpgX()); break; }
		case 0x16: { rmwZpgX(shiftL); break; }
		case 0x17: { read(pc()); break; }
		case 0x18: { read(pc()); c = 0; break; }
		case 0x19: { orA(readAbsY()); break; }
		case 0x1a: { read(pc()); break; }
		case 0x1b: { read(pc()); break; }
		case 0x1c: { read(pc()); break; }
		case 0x1d: { orA(readAbsX()); break; }
		case 0x1e: { rmwAbsX(shiftL); break; }
		case 0x1f: { read(pc()); break; }
		case 0x20: { jumpSub(); break; }
		case 0x21: { andA(readIndX()); break; }
		case 0x22: { read(pc()); break; }
		case 0x23: { read(pc()); break; }
		case 0x24: { bitA(readZpg()); break; }
		case 0x25: { andA(readZpg()); break; }
		case 0x26: { rmwZpg(rotL); break; }
		case 0x27: { read(pc()); break; }
		case 0x28: { setP(pull()); break; }
		case 0x29: { andA(readImm()); break; }
		case 0x2a: { read(pc()); a = rotL(a); break; }
		case 0x2b: { read(pc()); break; }
		case 0x2c: { bitA(readAbs()); break; }
		case 0x2d: { andA(readAbs()); break; }
		case 0x2e: { rmwAbs(rotL); break; }
		case 0x2f: { read(pc()); break; }
		case 0x30: { branch(n == 1); break; }
		case 0x31: { andA(readIndY()); break; }
		case 0x32: { read(pc()); break; }
		case 0x33: { read(pc()); break; }
		case 0x34: { read(pc()); break; }
		case 0x35: { andA(readZpgX()); break; }
		case 0x36: { rmwZpgX(rotL); break; }
		case 0x37: { read(pc()); break; }
		case 0x38: { read(pc()); c = 1; break; }
		case 0x39: { andA(readAbsY()); break; }
		case 0x3a: { read(pc()); break; }
		case 0x3b: { read(pc()); break; }
		case 0x3c: { read(pc()); break; }
		case 0x3d: { andA(readAbsX()); break; }
		case 0x3e: { rmwAbsX(rotL); break; }
		case 0x3f: { read(pc()); break; }
		case 0x40: { retInt(); break; }
		case 0x41: { eorA(readIndX()); break; }
		case 0x42: { read(pc()); break; }
		case 0x43: { read(pc()); break; }
		case 0x44: { read(pc()); break; }
		case 0x45: { eorA(readZpg()); break; }
		case 0x46: { rmwZpg(shiftR); break; }
		case 0x47: { read(pc()); break; }
		case 0x48: { push(a); break; }
		case 0x49: { eorA(readImm()); break; }
		case 0x4a: { read(pc()); a = shiftR(a); break; }
		case 0x4b: { read(pc()); break; }
		case 0x4c: { jumpAbs(); break; }
		case 0x4d: { eorA(readAbs()); break; }
		case 0x4e: { rmwAbs(shiftR); break; }
		case 0x4f: { read(pc()); break; }
		case 0x50: { branch(v == 0); break; }
		case 0x51: { eorA(readIndY()); break; }
		case 0x52: { read(pc()); break; }
		case 0x53: { read(pc()); break; }
		case 0x54: { read(pc()); break; }
		case 0x55: { eorA(readZpgX()); break; }
		case 0x56: { rmwZpgX(shiftR); break; }
		case 0x57: { read(pc()); break; }
		case 0x58: { read(pc()); i = 0; break; }
		case 0x59: { eorA(readAbsY()); break; }
		case 0x5a: { read(pc()); break; }
		case 0x5b: { read(pc()); break; }
		case 0x5c: { read(pc()); break; }
		case 0x5d: { eorA(readAbsX()); break; }
		case 0x5e: { rmwAbsX(shiftR); break; }
		case 0x5f: { read(pc()); break; }
		case 0x60: { retSub(); break; }
		case 0x61: { addA(readIndX()); break; }
		case 0x62: { read(pc()); break; }
		case 0x63: { read(pc()); break; }
		case 0x64: { read(pc()); break; }
		case 0x65: { addA(readZpg()); break; }
		case 0x66: { rmwZpg(rotR); break; }
		case 0x67: { read(pc()); break; }
		case 0x68: { setA(pull()); break; }
		case 0x69: { addA(readImm()); break; }
		case 0x6a: { read(pc()); a = rotR(a); break; }
		case 0x6b: { read(pc()); break; }
		case 0x6c: { jumpInd(); break; }
		case 0x6d: { addA(readAbs()); break; }
		case 0x6e: { rmwAbs(rotR); break; }
		case 0x6f: { read(pc()); break; }
		case 0x70: { branch(v == 1); break; }
		case 0x71: { addA(readIndY()); break; }
		case 0x72: { read(pc()); break; }
		case 0x73: { read(pc()); break; }
		case 0x74: { read(pc()); break; }
		case 0x75: { addA(readZpgX()); break; }
		case 0x76: { rmwZpgX(rotR); break; }
		case 0x77: { read(pc()); break; }
		case 0x78: { read(pc()); i = 1; break; }
		case 0x79: { addA(readAbsY()); break; }
		case 0x7a: { read(pc()); break; }
		case 0x7b: { read(pc()); break; }
		case 0x7c: { read(pc()); break; }
		case 0x7d: { addA(readAbsX()); break; }
		case 0x7e: { rmwAbsX(rotR); break; }
		case 0x7f: { read(pc()); break; }
		case 0x80: { read(pc()); break; }
		case 0x81: { writeIndX(a); break; }
		case 0x82: { read(pc()); break; }
		case 0x83: { read(pc()); break; }
		case 0x84: { writeZpg(y); break; }
		case 0x85: { writeZpg(a); break; }
		case 0x86: { writeZpg(x); break; }
		case 0x87: { read(pc()); break; }
		case 0x88: { read(pc()); setY(y - 1); break; }
		case 0x89: { read(pc()); break; }
		case 0x8a: { read(pc()); setA(x); break; }
		case 0x8b: { read(pc()); break; }
		case 0x8c: { writeAbs(y); break; }
		case 0x8d: { writeAbs(a); break; }
		case 0x8e: { writeAbs(x); break; }
		case 0x8f: { read(pc()); break; }
		case 0x90: { branch(c == 0); break; }
		case 0x91: { writeIndY(a); break; }
		case 0x92: { read(pc()); break; }
		case 0x93: { read(pc()); break; }
		case 0x94: { writeZpgX(y); break; }
		case 0x95: { writeZpgX(a); break; }
		case 0x96: { writeZpgY(x); break; }
		case 0x97: { read(pc()); break; }
		case 0x98: { read(pc()); setA(y); break; }
		case 0x99: { writeAbsY(a); break; }
		case 0x9a: { read(pc()); spl = x; break; }
		case 0x9b: { read(pc()); break; }
		case 0x9c: { read(pc()); break; }
		case 0x9d: { writeAbsX(a); break; }
		case 0x9e: { read(pc()); break; }
		case 0x9f: { read(pc()); break; }
		case 0xa0: { setY(readImm()); break; }
		case 0xa1: { setA(readIndX()); break; }
		case 0xa2: { setX(readImm()); break; }
		case 0xa3: { read(pc()); break; }
		case 0xa4: { setY(readZpg()); break; }
		case 0xa5: { setA(readZpg()); break; }
		case 0xa6: { setX(readZpg()); break; }
		case 0xa7: { read(pc()); break; }
		case 0xa8: { read(pc()); setY(a); break; }
		case 0xa9: { setA(readImm()); break; }
		case 0xaa: { read(pc()); setX(a); break; }
		case 0xab: { read(pc()); break; }
		case 0xac: { setY(readAbs()); break; }
		case 0xad: { setA(readAbs()); break; }
		case 0xae: { setX(readAbs()); break; }
		case 0xaf: { read(pc()); break; }
		case 0xb0: { branch(c == 1); break; }
		case 0xb1: { setA(readIndY()); break; }
		case 0xb2: { read(pc()); break; }
		case 0xb3: { read(pc()); break; }
		case 0xb4: { setY(readZpgX()); break; }
		case 0xb5: { setA(readZpgX()); break; }
		case 0xb6: { setX(readZpgX()); break; }
		case 0xb7: { read(pc()); break; }
		case 0xb8: { read(pc()); v = 0; break; }
		case 0xb9: { setA(readAbsY()); break; }
		case 0xba: { read(pc()); setX(spl); break; }
		case 0xbb: { read(pc()); break; }
		case 0xbc: { setY(readAbsX()); break; }
		case 0xbd: { setA(readAbsX()); break; }
		case 0xbe: { setX(readAbsX()); break; }
		case 0xbf: { read(pc()); break; }
		case 0xc0: { cmpY(readImm()); break; }
		case 0xc1: { cmpA(readIndX()); break; }
		case 0xc2: { read(pc()); break; }
		case 0xc3: { read(pc()); break; }
		case 0xc4: { cmpY(readZpg()); break; }
		case 0xc5: { cmpA(readZpg()); break; }
		case 0xc6: { rmwZpg(dec); break; }
		case 0xc7: { read(pc()); break; }
		case 0xc8: { read(pc()); setY(y + 1); break; }
		case 0xc9: { cmpA(readImm()); break; }
		case 0xca: { read(pc()); setX(x - 1); break; }
		case 0xcb: { read(pc()); break; }
		case 0xcc: { cmpY(readAbs()); break; }
		case 0xcd: { cmpA(readAbs()); break; }
		case 0xce: { rmwAbs(dec); break; }
		case 0xcf: { read(pc()); break; }
		case 0xd0: { branch(z == 0); break; }
		case 0xd1: { cmpA(readIndY()); break; }
		case 0xd2: { read(pc()); break; }
		case 0xd3: { read(pc()); break; }
		case 0xd4: { read(pc()); break; }
		case 0xd5: { cmpA(readZpgX()); break; }
		case 0xd6: { rmwZpgX(dec); break; }
		case 0xd7: { read(pc()); break; }
		case 0xd8: { read(pc()); d = 0; break; }
		case 0xd9: { cmpA(readAbsY()); break; }
		case 0xda: { read(pc()); break; }
		case 0xdb: { read(pc()); break; }
		case 0xdc: { read(pc()); break; }
		case 0xdd: { cmpA(readAbsX()); break; }
		case 0xde: { rmwAbsX(dec); break; }
		case 0xdf: { read(pc()); break; }
		case 0xe0: { cmpX(readImm()); break; }
		case 0xe1: { subA(readIndX()); break; }
		case 0xe2: { read(pc()); break; }
		case 0xe3: { read(pc()); break; }
		case 0xe4: { cmpX(readZpg()); break; }
		case 0xe5: { subA(readZpg()); break; }
		case 0xe6: { rmwZpg(inc); break; }
		case 0xe7: { read(pc()); break; }
		case 0xe8: { read(pc()); setX(x + 1); break; }
		case 0xe9: { subA(readImm()); break; }
		case 0xea: { read(pc()); break; }
		case 0xeb: { read(pc()); break; }
		case 0xec: { cmpX(readAbs()); break; }
		case 0xed: { subA(readAbs()); break; }
		case 0xee: { rmwAbs(inc); break; }
		case 0xef: { read(pc()); break; }
		case 0xf0: { branch(z == 1); break; }
		case 0xf1: { subA(readIndY()); break; }
		case 0xf2: { read(pc()); break; }
		case 0xf3: { read(pc()); break; }
		case 0xf4: { read(pc()); break; }
		case 0xf5: { subA(readZpgX()); break; }
		case 0xf6: { rmwZpgX(inc); break; }
		case 0xf7: { read(pc()); break; }
		case 0xf8: { read(pc()); d = 1; break; }
		case 0xf9: { subA(readAbsY()); break; }
		case 0xfa: { read(pc()); break; }
		case 0xfb: { read(pc()); break; }
		case 0xfc: { read(pc()); break; }
		case 0xfd: { subA(readAbsX()); break; }
		case 0xfe: { rmwAbsX(inc); break; }
		case 0xff: { read(pc()); break; }
		}
	}
	
	void init() {
		memset(ram, 0, sizeof(ram));
		
		n = 0;
		v = 0;
		d = 0;
		i = 0;
		z = 0;
		c = 0;
		pcl = 0;
		pch = 0;
		spl = 0;
		a = 0;
		x = 0;
		y = 0;
		
		reset();
	}
	
	void deinit() {
		
	}
}
