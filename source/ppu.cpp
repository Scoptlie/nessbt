#include "ppu.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <GL/gl3w.h>

#include "ppu/ciRam.h"
#include "ppu/globalPalette.h"
#include "ppu/paletteRam.h"
#include "ppu/spriteRam.h"
#include "cart.h"

namespace Ppu {
	bool frameDone;
	U16 nCol, nRow;
	U64 nFrame;
	
	bool reRender;
	
	// Colour and depth buffers for the frame
	Rgb colourBuf[256 * 240];
	U8 depthBuf[256 * 240];
	U8 sprite0Buf[256 * 240];
	
	U32 frame;
	
	bool nmi;
	
	// Vblank flag
	bool vblank;
	// Sprite 0 hit flag
	bool sprite0;
	
	// The column and row where the sprite0 flag will be set
	U16 sprite0Col, sprite0Row;
	
	// Externally-controlled settings
	
		// Whether an NMI is signalled at the start of vertical blanking
		bool nmiOnVblank;
		
		// Whether sprites are visible, and visible in the left 8 pixels
		bool spritesVisible, spritesVisibleLeft;
		// Sprite pattern table index
		U8 spritesPtIdx;
		// Whether 8x16 sprites are enabled
		bool bigSprites;
		
		// Whether the background is visible, and visible in the left 8 pixels
		bool bgVisible, bgVisibleLeft;
		// Background pattern table index
		U8 bgPtIdx;
		
		// Whether greyscale colours are used
		bool greyscale;
		// Colour emphasis
		U8 emphasis;
		
		// Whether the next write to either the scroll or addr register
		// is the second one
		bool secondWrite;
		// Background base nametable
		U8 baseNtX, baseNtY;
		// Background scroll
		U8 scrollX, scrollY, initScrollY;
		// Pointer into PPU memory for the data register
		U16 ppuAddr;
		// Whether to increment ppuAddr by 32 when the data register is accessed
		bool ppuAddrInc32;
		// Buffered result from reading the data register
		U8 readBuf;
		
		// Pointer into sprite memory for the spriteData register
		U8 spriteAddr;
	
	//
	
	void incPpuAddr() {
		if (ppuAddrInc32) {
			ppuAddr += 32;
		} else {
			ppuAddr++;
		}
		ppuAddr &= 0x7fff;
	}
	
	// Registers accessable through
	// reads and writes
	enum {
		regCtrl = 0,
		regMask = 1,
		regStatus = 2,
		regSpriteAddr = 3,
		regSpriteData = 4,
		regScroll = 5,
		regAddr = 6,
		regData = 7
	};
	
	U8 read(U16 addr) {
		if (addr == regStatus) {
			auto r = (vblank << 7) | (sprite0 << 6);
			vblank = false;
			secondWrite = false;
			return r;
		} else if (addr == regSpriteData) {
			return SpriteRam::read(spriteAddr);
		} else if (addr == regData) {
			auto r = readBuf;
			
			if (ppuAddr <= 0x1fff) {
				readBuf = Cart::ppuRead(ppuAddr);
			} else if (ppuAddr <= 0x2fff) {
				readBuf = CiRam::read(ppuAddr & 0xfff);
			}
			
			incPpuAddr();
			return r;
		}
		return 0;
	}
	
	void write(U16 addr, U8 data) {
		if (addr == regCtrl) {
			baseNtX = data & 1;
			baseNtY = (data >> 1) & 1;
			ppuAddrInc32 = (data >> 2) & 1;
			spritesPtIdx = (data >> 3) & 1;
			bgPtIdx = (data >> 4) & 1;
			bigSprites = (data >> 5) & 1;
			nmiOnVblank = data >> 7;
			reRender = true;
		} else if (addr == regMask) {
			greyscale = data & 1;
			bgVisibleLeft = (data >> 1) & 1;
			spritesVisibleLeft = (data >> 2) & 1;
			bgVisible = (data >> 3) & 1;
			spritesVisible = (data >> 4) & 1;
			emphasis = data >> 5;
			reRender = true;
		} else if (addr == regSpriteAddr) {
			spriteAddr = data;
		} else if (addr == regSpriteData) {
			SpriteRam::write(spriteAddr++, data);
			reRender = true;
		} else if (addr == regScroll) {
			ppuAddr = 0;
			if (!secondWrite) {
				scrollX = data;
			} else {
				scrollY = data;
			}
			secondWrite ^= 1;
			reRender = true;
		} else if (addr == regAddr) {
			scrollX = 0;
			scrollY = 0;
			baseNtX = 0;
			baseNtY = 0;
			if (!secondWrite) {
				ppuAddr = (data << 8) & 0x7fff;
			} else {
				ppuAddr |= data;
			}
			secondWrite ^= 1;
			reRender = true;
		} else if (addr == regData) {
			if (ppuAddr >= 0x2000 && ppuAddr <= 0x2fff) {
				CiRam::write(ppuAddr & 0xfff, data);
			} else if (ppuAddr >= 0x3f00) {
				PaletteRam::write(ppuAddr & 0x1f, data);
			}
			
			incPpuAddr();
		}
	}
	
	// Render sprites to the colour and depth buffers,
	// starting at the current row (nRow)
	void renderSprites() {
		// For every sprite
		for (auto i = 0; i < 64; i++) {
			auto sprite = SpriteRam::sprite(i);
			
			// Height (pixels)
			auto h = bigSprites? 16 : 8;
			
			auto palette = PaletteRam::palette(4 + sprite.paletteIdx, greyscale);
			
			auto depth = sprite.priority? 1 : 2;
			
			// For every pixel of the sprite
			for (auto ySprite = 0; ySprite < h; ySprite++) {
				auto eYSprite = sprite.flipY? (h - 1 - ySprite) : ySprite;
				
				// Pixel Y position on the screen
				auto yS = sprite.y + ySprite + 1;
				
				// Discard if before the current or beyond the last row
				if (yS < nRow) {
					continue;
				}
				if (yS >= 240) {
					break;
				}
				
				auto ptIdx = spritesPtIdx;
				auto patternIdx = sprite.patternIdx;
				if (bigSprites) {
					ptIdx = patternIdx & 1;
					patternIdx &= ~1;
					if (ySprite >= 8) {
						// Use next pattern if on the bottom half
						// of a big sprite
						patternIdx++;
					}
				}
				
				// Pattern's low and high bytes
				auto patternLo = Cart::ppuRead((0x1000 * ptIdx) + (16 * patternIdx) + (eYSprite % 8));
				auto patternHi = Cart::ppuRead((0x1000 * ptIdx) + (16 * patternIdx) + 8 + (eYSprite % 8));
				
				for (
					auto xSprite = 0,
						iXSprite = 7,
						// Pixel index on the screen
						idxS = (256 * yS) + sprite.x;
					xSprite < 8;
					xSprite++, iXSprite--, idxS++
				) {
					// Pixel X position on the screen
					auto xS = sprite.x + xSprite;
					
					// Discard if beyond the last column
					if (xS >= 256) {
						break;
					}
					// Discard if in the left-most column while it is masked out
					if (!spritesVisibleLeft && xS < 8) {
						continue;
					}
					
					auto peIdxHi = (patternHi >> (sprite.flipX? xSprite : iXSprite)) & 1;
					auto peIdxLo = (patternLo >> (sprite.flipX? xSprite : iXSprite)) & 1;
					// Palette entry index
					auto peIdx = (2 * peIdxHi) + peIdxLo;
					
					if (i == 0 && peIdx != 0) {
						sprite0Buf[idxS] = 0xff;
					}
					
					// Discard if palette entry index is 0 (transparent)
					// or depth at pixel is >= this sprite's depth
					if (peIdx != 0 && depthBuf[idxS] < depth) {
						auto colourIdx = palette.colourIdxs[peIdx];
						
						colourBuf[idxS] = GlobalPalette::colour(colourIdx, emphasis);
						depthBuf[idxS] = depth;
					}
				}
			}
		}
	}
	
	// Render background to the colour buffer,
	// starting at the current row (nRow)
	void renderBg() {
		auto eScrollX = (256 * baseNtX) + scrollX;
		auto eScrollY = (240 * baseNtY) + initScrollY;
		
		// Iterate over every tile to be rendered
		// x0 and y0 are the pixel position of the top-left-most visible pixel
		for (auto y0 = eScrollY + nRow; y0 < eScrollY + 240; y0 = ((y0 / 8) + 1) * 8) {
			for (auto x0 = eScrollX + (bgVisibleLeft? 0 : 8); x0 < eScrollX + 256; x0 = ((x0 / 8) + 1) * 8) {
				auto ntX = (x0 / 256) % 2;
				auto ntY = (y0 / 240) % 2;
				// Name table's index
				auto ntIdx = (2 * ntY) + ntX;
				
				// X and Y positions (pixels) in the name table
				auto xNt = x0 % 256;
				auto yNt = y0 % 240;
				
				auto tileXNt = xNt / 8;
				auto tileYNt = yNt / 8;
				// Index in the name table
				auto tileIdxNt = (tileYNt * 32) + tileXNt;
				
				auto ateX = xNt / 32;
				auto ateY = yNt / 32;
				// Attribute entry's index in the attribute table
				auto ateIdx = (ateY * 8) + ateX;
				
				auto attrX = (xNt / 16) % 2;
				auto attrY = (yNt / 16) % 2;
				// Attribute's index in the attribute table entry
				auto attrIdx = (2 * attrY) + attrX;
				
				auto patternIdx = CiRam::read((0x400 * ntIdx) + tileIdxNt);
				
				// Attribute table entry
				auto ate = CiRam::read((0x400 * ntIdx) + 0x3c0 + ateIdx);
				
				auto paletteIdx = (ate >> (2 * attrIdx)) & 0b11;
				auto palette = PaletteRam::palette(paletteIdx, greyscale);
				
				// Iterate over the tile's visible pixels
				auto y = y0; do {
					// Y position in the tile
					auto yTile = y % 8;
					// Y position on the screen
					auto yS = y - eScrollY;
					
					// Pattern's low and high bytes
					auto patternLo = Cart::ppuRead((0x1000 * bgPtIdx) + (16 * patternIdx) + yTile);
					auto patternHi = Cart::ppuRead((0x1000 * bgPtIdx) + (16 * patternIdx) + 8 + yTile);
					
					auto x = x0; do {
						// X position in the tile
						auto xTile = x % 8;
						// X position on the screen
						auto xS = x - eScrollX;
						
						// Index on the screen
						auto idxS = (256 * yS) + xS;
						
						auto iXTile = 7 - xTile;
						auto peIdxHi = (patternHi >> iXTile) & 1;
						auto peIdxLo = (patternLo >> iXTile) & 1;
						// Palette entry index
						auto peIdx = (2 * peIdxHi) + peIdxLo;
						
						if (peIdx != 0 && sprite0Buf[idxS] != 0 &&
							(yS < sprite0Row || (yS == sprite0Row && xS < sprite0Col))
						) {
							sprite0Col = xS;
							sprite0Row = yS;
						}
						
						// Discard if palette entry index is 0 (transparent)
						// or depth at pixel is >= 2
						if (peIdx != 0 && depthBuf[idxS] < 2) {
							// Colour index
							auto colourIdx = palette.colourIdxs[peIdx];
							
							colourBuf[idxS] = GlobalPalette::colour(colourIdx, emphasis);
						}
						
						x++;
					} while (x < eScrollX + 256 && x % 8 != 0);
					y++;
				} while (y < eScrollY + 240 && y % 8 != 0);
			}
		}
	}
	
	void tick() {
		auto blanking = nRow >= 240 || nCol >= 256 || (!spritesVisible && !bgVisible);
		
		if (reRender && nCol == 0 && !blanking) {
			reRender = false;
			
			// Clear the depth and sprite0 buffers
			memset(depthBuf + (256 * nRow), 0, (256 * (240 - nRow)) * sizeof(depthBuf[0]));
			memset(sprite0Buf + (256 * nRow), 0, (256 * (240 - nRow)) * sizeof(sprite0Buf[0]));
			
			// Clear the colour buffer
			auto clearColour = GlobalPalette::colour(PaletteRam::clearColour(greyscale), emphasis);
			for (auto i = 256 * nRow; i < (256 * 240); i++) {
				colourBuf[i] = clearColour;
			}
			
			// Render sprites and background
			if (spritesVisible) { renderSprites(); }
			if (bgVisible) { renderBg(); }
		}
		
		if (nCol == sprite0Col && nRow == sprite0Row) {
			sprite0 = 1;
		}
		
		nCol++;
		if (nCol == 341) {
			nCol = 0;
			nRow++;
			
			if (nRow == 240) {
				frameDone = true;
				
				glBindTexture(GL_TEXTURE_2D, frame);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 240, 0, GL_RGB, GL_UNSIGNED_BYTE, colourBuf);
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 240, 0, GL_RED, GL_UNSIGNED_BYTE, sprite0Buf);
				
				vblank = true;
				if (nmiOnVblank) {
					nmi = true;
				}
			}
			
			if (nRow == 262) {
				nRow = 0;
				nFrame++;
				
				reRender = true;
				
				vblank = false;
				sprite0 = false;
				
				sprite0Col = 0xffff;
				sprite0Row = 0xffff;
				
				initScrollY = scrollY;
			}
		}
	}
	
	void init() {
		PaletteRam::init();
		CiRam::init();
		SpriteRam::init();
		
		frameDone = false;
		nCol = 0;
		nRow = 0;
		nFrame = 0;
		
		reRender = true;
		
		memset(colourBuf, 0, sizeof(colourBuf));
		memset(depthBuf, 0, sizeof(depthBuf));
		memset(sprite0Buf, 0, sizeof(sprite0Buf));
		
		glGenTextures(1, &frame);
		glBindTexture(GL_TEXTURE_2D, frame);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 240, 0, GL_RGB, GL_UNSIGNED_BYTE, colourBuf);
		
		nmi = false;
		
		vblank = false;
		sprite0 = false;
		
		sprite0Col = 0xffff;
		sprite0Row = 0xffff;
		
		nmiOnVblank = false;
		
		spritesVisible = false;
		spritesVisibleLeft = false;
		spritesPtIdx = 0;
		bigSprites = false;
		
		bgVisible = false;
		bgVisibleLeft = false;
		bgPtIdx = 0;
		
		greyscale = false;
		emphasis = 0;
		
		secondWrite = 0;
		scrollX = 0;
		scrollY = 0;
		ppuAddr = 0;
		ppuAddrInc32 = false;
		readBuf = 0;
		
		spriteAddr = 0;
	}
}
