#pragma once

#include "runtime/common.h"

namespace Profiler {
	#ifdef outputNmiLocations
		void addNmiLocation(U16 addr);
	#else
		static void addNmiLocation(U16 addr) { }
	#endif
	
	#ifdef outputJumpTargets
		void addJumpTarget(U16 addr);
	#else
		static void addJumpTarget(U16 addr) { }
	#endif
	
	#ifdef outputFrameProfiles
		void beginCode();
		void endCode();
		
		void beginRender();
		void endRender();
		
		void addFrameEmuCycles(USize n);
		void addFrameStCycles(USize n);
	#else
		static void beginCode() { }
		static void endCode() { }
		
		static void beginRender() { }
		static void endRender() { }
		
		static void addFrameEmuCycles(USize n) { }
		static void addFrameStCycles(USize n) { }
	#endif
	
	void init();
}
