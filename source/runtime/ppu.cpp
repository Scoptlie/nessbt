#include "ppu.h"

#include <cstring>

#include <GL/gl3w.h>

namespace Ppu {
	U8 ciRam[0x800];
	
	U16 ciRamAddr(U16 addr) {
		auto lo = addr & 0b1111111111;
		auto hi = (mirrorV? addr : (addr >> 1)) & 0b10000000000;
		return lo | hi;
	}
	
	struct Sprite {
		U8 y;
		U8 patternIdx;
		U8 paletteIdx : 2;
		U8 padding : 3;
		U8 back : 1;
		U8 flipX : 1;
		U8 flipY : 1;
		U8 x;
	};
	
	U8 spriteRam[0x100];
	
	U8 paletteRam[0x20];
	
	U8 paletteRamAddr(U8 addr) {
		if ((addr & 0b10011) == 0b10000) {
			return addr & 0b01111;
		}
		return addr;
	}
	
	U16 nRow, nCol;
	
	bool shouldRender;
	
	enum : U8 {
		pflagsSpriteBack = 1,
		pflagsSpriteFore = 2,
		pflagsSprite0 = 4,
	};
	
	Rgb colourBuf[256 * 240];
	U8 pflagsBuf[256 * 240];
	
	bool frameDone;
	U32 frame;
	
	bool nmi;
	
	bool vblank, sprite0;
	
	U16 sprite0Row, sprite0Col;
	
	bool nmiOnVblank;
	
	bool bgVis, spritesVis,
		bgVisLeft, spritesVisLeft;
	
	U8 bgPtIdx, spritesPtIdx;
	
	bool bigSprites;
	
	bool secondWrite;
	
	U8 baseNtX, baseNtY,
		scrollX, scrollY,
		curBaseNtX, curBaseNtY,
		curScrollX, curScrollY;
	
	U16 ptr, ptrInc;
	U8 dataBuf;
	
	U8 spritePtr;
	
	enum : U8 {
		regCtrl = 0,
		regMask = 1,
		regStatus = 2,
		regSpritePtr = 3,
		regSpriteData = 4,
		regScroll = 5,
		regPtr = 6,
		regData = 7
	};
	
	U8 read(U8 addr) {
		if (addr == regStatus) {
			auto r = (vblank << 7) | (sprite0 << 6);
			vblank = false;
			secondWrite = false;
			return r;
		} else if (addr == regSpriteData) {
			return spriteRam[spritePtr];
		} else if (addr == regData) {
			auto r = dataBuf;
			if (ptr <= 0x1fff) {
				dataBuf = chrRom[ptr & (chrRomSize - 1)];
			} else if (ptr <= 0x2fff) {
				dataBuf = ciRam[ciRamAddr(ptr & 0xfff)];
			}
			ptr = (ptr + ptrInc) & 0x7fff;
			return r;
		}
		return 0;
	}
	
	void write(U8 addr, U8 data) {
		if (addr == regCtrl) {
			auto newBaseNtX = data & 1;
			auto newBaseNtY = (data >> 1) & 1;
			ptrInc = ((data >> 2) & 1)? 32 : 1;
			auto newSpritesPtIdx = (data >> 3) & 1;
			auto newBgPtIdx = (data >> 4) & 1;
			auto newBigSprites = (data >> 5) & 1;
			nmiOnVblank = (data >> 7) & 1;
			
			if (
				baseNtX != curBaseNtX ||
				spritesPtIdx != newSpritesPtIdx ||
				bgPtIdx != newBgPtIdx ||
				bigSprites != newBigSprites
			) {
				shouldRender = true;
			}
			
			baseNtX = newBaseNtX;
			baseNtY = newBaseNtY;
			curBaseNtX = newBaseNtX;
			spritesPtIdx = newSpritesPtIdx;
			bgPtIdx = newBgPtIdx;
			bigSprites = newBigSprites;
		} else if (addr == regMask) {
			bgVisLeft = (data >> 1) & 1;
			spritesVisLeft = (data >> 2) & 1;
			bgVis = (data >> 3) & 1;
			spritesVis = (data >> 4) & 1;
			
			shouldRender = true;
		} else if (addr == regSpritePtr) {
			spritePtr = data;
		} else if (addr == regSpriteData) {
			spriteRam[spritePtr++] = data;
		} else if (addr == regScroll) {
			ptr = 0;
			if (!secondWrite) {
				scrollX = data;
				curScrollX = data;
			} else {
				scrollY = data;
			}
			secondWrite ^= 1;
			shouldRender = true;
		} else if (addr == regPtr) {
			scrollX = 0;
			scrollY = 0;
			baseNtX = 0;
			baseNtY = 0;
			if (!secondWrite) {
				ptr = (data << 8) & 0x7fff;
			} else {
				ptr |= data;
			}
			secondWrite ^= 1;
		} else if (addr == regData) {
			if (ptr >= 0x2000 && ptr <= 0x2fff) {
				ciRam[ciRamAddr(ptr & 0xfff)] = data;
			} else if (ptr >= 0x3f00) {
				paletteRam[paletteRamAddr(ptr & 0x1f)] = data & 0x3f;
			}
			ptr = (ptr + ptrInc) & 0x7fff;
		}
	}
	
	void renderSprites() {
		for (auto i = 0; i < 64; i++) {
			auto sprite = ((Sprite*)spriteRam)[i];
			
			auto h = bigSprites? 16 : 8;
			
			U8 palette[4];
			memcpy(palette, paletteRam + 16 + (4 * sprite.paletteIdx), sizeof(palette));
			
			for (auto ySprite = 0; ySprite < h; ySprite++) {
				auto yS = sprite.y + ySprite + 1;
				if (yS < nRow) { continue; }
				if (yS >= 240) { break; }
				
				auto ptIdx = spritesPtIdx;
				auto patternIdx = sprite.patternIdx;
				if (bigSprites) {
					ptIdx = patternIdx & 1;
					patternIdx &= ~1;
					if (ySprite >= 8) {
						patternIdx++;
					}
				}
				
				auto yPattern = (sprite.flipY? (h - 1 - ySprite) : ySprite) & 7;
				
				auto patternAddr = (0x1000 * ptIdx) + (0x10 * patternIdx);
				auto patternLo = chrRom[(patternAddr + yPattern) & (chrRomSize - 1)];
				auto patternHi = chrRom[(patternAddr + 8 + yPattern) & (chrRomSize - 1)];
				
				for (auto xSprite = 0; xSprite < 8; xSprite++) {
					auto xS = sprite.x + xSprite;
					if (xS >= 256) { break; }
					if (!spritesVisLeft && xS <= 7) {
						continue;
					}
					
					auto idxS = (256 * yS) + xS;
					
					auto xPattern = sprite.flipX? xSprite : (7 - xSprite);
					
					auto peIdx =
						(((patternHi >> xPattern) & 1) << 1) |
						((patternLo >> xPattern) & 1);
					
					if (peIdx != 0) {
						auto pflags = pflagsBuf[idxS];
						
						if (i == 0) {
							pflags |= pflagsSprite0;
						}
						
						if (
							(pflags & (pflagsSpriteBack | pflagsSpriteFore)) == 0 ||
							(!sprite.back && (pflags & pflagsSpriteFore) == 0)
						) {
							colourBuf[idxS] = globalPalette[palette[peIdx]];
							pflags |= sprite.back? pflagsSpriteBack : pflagsSpriteFore;
						}
						
						pflagsBuf[idxS] = pflags;
					}
					
					
				}
			}
		}
	}
	
	void renderBg() {
		auto scrollX = (256 * curBaseNtX) + curScrollX;
		auto scrollY = (240 * curBaseNtY) + curScrollY;
		
		for (
			auto y0 = scrollY + nRow;
			y0 < scrollY + 240;
			y0 = ((y0 / 8) + 1) * 8
		) {
			for (
				auto x0 = scrollX + (bgVisLeft? 0 : 8);
				x0 < scrollX + 256;
				x0 = ((x0 / 8) + 1) * 8
			) {
				auto ntIdx =
					(2 * ((y0 / 240) % 2)) |
					((x0 / 256) % 2);
				auto ntAddr = 0x400 * ntIdx;
				auto atAddr = ntAddr + 0x3c0;
				
				auto xNt = x0 % 256;
				auto yNt = y0 % 240;
				
				auto tileIdxNt =
					(32 * (yNt / 8)) + (xNt / 8);
				auto ateIdx =
					(8 * (yNt / 32)) + (xNt / 32);
				auto attrIdx =
					(2 * ((yNt / 16) % 2)) + ((xNt / 16) % 2);
				auto patternIdx =
					ciRam[ciRamAddr(ntAddr + tileIdxNt)];
				auto ate =
					ciRam[ciRamAddr(atAddr + ateIdx)];
				auto paletteIdx =
					(ate >> (2 * attrIdx)) & 0b11;
				
				U8 palette[4];
				memcpy(palette, paletteRam + (4 * paletteIdx), sizeof(palette));
				
				auto y = y0; do {
					auto yTile = y % 8;
					auto yS = y - scrollY;
					
					auto patternAddr = (0x1000 * bgPtIdx) + (0x10 * patternIdx);
					auto patternLo = chrRom[(patternAddr + yTile) & (chrRomSize - 1)];
					auto patternHi = chrRom[(patternAddr + 8 + yTile) & (chrRomSize - 1)];
					
					auto x = x0; do {
						auto xTile = x % 8;
						auto xS = x - scrollX;
						
						auto idxS = (256 * yS) + xS;
						
						auto xPattern = 7 - xTile;
						
						auto peIdx =
							(((patternHi >> xPattern) & 1) << 1) |
							((patternLo >> xPattern) & 1);
						
						if (peIdx != 0) {
							auto pflags = pflagsBuf[idxS];
							
							if (
								pflags & pflagsSprite0 &&
								(yS < sprite0Row || (yS == sprite0Row && xS < sprite0Col))
							) {
								sprite0Row = yS;
								sprite0Col = xS;
							}
							
							if (!(pflags & pflagsSpriteFore)) {
								colourBuf[idxS] = globalPalette[palette[peIdx]];
							}
							
							pflagsBuf[idxS] = pflags;
						}
						
						x++;
					} while (x < scrollX + 256 && x % 8 != 0);
					
					y++;
				} while (y < scrollY + 240 && y % 8 != 0);
			}
		}
	}
	
	void render() {
		auto colour = globalPalette[paletteRam[0]];
		for (auto i = 256 * nRow; i < (256 * 240); i++) {
			colourBuf[i] = colour;
		}
		
		memset(pflagsBuf + (256 * nRow), 0, (256 * (240 - nRow)) * sizeof(pflagsBuf[0]));
		
		if (spritesVis) { renderSprites(); }
		
		if (bgVis) { renderBg(); }
	}
	
	void tick() {
		if (
			shouldRender &&
			nRow < 240 && nCol == 0
		) {
			render();
			
			shouldRender = false;
		}
		
		if (nCol == sprite0Col && nRow == sprite0Row) {
			sprite0 = true;
		}
		
		nCol++;
		if (nCol == 341) {
			nCol = 0;
			nRow++;
			
			if (nRow == 240) {
				frameDone = true;
				
				glBindTexture(GL_TEXTURE_2D, frame);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 240, 0, GL_RGB, GL_UNSIGNED_BYTE, colourBuf);
				
				vblank = true;
				if (nmiOnVblank) {
					nmi = true;
				}
			}
			
			if (nRow == 262) {
				nRow = 0;
				
				shouldRender = true;
				
				vblank = false;
				sprite0 = false;
				
				sprite0Row = 0xffff;
				sprite0Col = 0xffff;
				
				curScrollX = scrollX;
				curScrollY = scrollY;
				curBaseNtX = baseNtX;
				curBaseNtY = baseNtY;
			}
		}
	}
	
	void init() {
		glGenTextures(1, &frame);
		glBindTexture(GL_TEXTURE_2D, frame);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 240, 0, GL_RGB, GL_UNSIGNED_BYTE, colourBuf);
		
		sprite0Row = 0xffff;
		sprite0Col = 0xffff;
		
		ptrInc = 1;
	}
}
