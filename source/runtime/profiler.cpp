#include "profiler.h"

#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <vector>

namespace Profiler {
	#ifdef outputNmiLocations
		bool nmiLocations[0x8000];
		
		void addNmiLocation(U16 addr) {
			if (addr >= 0x8000) {
				nmiLocations[addr - 0x8000] = true;
			}
		}
	#endif
	
	#ifdef outputJumpTargets
		bool jumpTargets[0x8000];
		
		void addJumpTarget(U16 addr) {
			if (addr >= 0x8000) {
				jumpTargets[addr - 0x8000] = true;
			}
		}
	#endif
	
	#ifdef outputFrameProfiles
		struct FrameProfile {
			I64 codeTime;
			I64 renderTime;
		};
		
		std::vector<FrameProfile> frameProfiles;
		
		I64 codeStartTime;
		I64 renderStartTime;
		
		I64 renderTime;
		
		void beginCode() {
			using namespace std::chrono;
			
			codeStartTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
		}
		
		void endCode() {
			using namespace std::chrono;
			
			auto now = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
			
			auto codeTime = (now - codeStartTime) - renderTime;
			
			frameProfiles.push_back(FrameProfile{
				.codeTime = codeTime,
				.renderTime = renderTime
			});
			
			renderTime = 0;
		}
		
		void beginRender() {
			using namespace std::chrono;
			
			renderStartTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
		}
		
		void endRender() {
			using namespace std::chrono;
			
			auto now = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
			
			renderTime += now - renderStartTime;
		}
		
		void addFrameEmuCycles(USize n) {
			
		}
		
		void addFrameStCycles(USize n) {
			
		}
	#endif
	
	void init() {
		#ifdef outputNmiLocations
			std::atexit([]() {
				auto s = fopen("gen/nmiLocations", "wb");
				auto addr = U16(0x8000);
				do {
					if (nmiLocations[addr - 0x8000]) {
						fwrite(&addr, 2, 1, s);
					}
					
					addr++;
				} while (addr != 0xffff);
				fclose(s);
			});
		#endif
		
		#ifdef outputJumpTargets
			std::atexit([]() {
				auto s = fopen("gen/jumpTargets", "wb");
				auto addr = U16(0x8000);
				do {
					if (jumpTargets[addr - 0x8000]) {
						fwrite(&addr, 2, 1, s);
					}
					
					addr++;
				} while (addr != 0xffff);
				fclose(s);
			});
		#endif
		
		#ifdef outputFrameProfiles
			std::atexit([]() {
				auto s = fopen("gen/frameProfiles.txt", "wb");
				fputs("idx,codeTime,renderTime\n", s);
				USize n = 0;
				for (auto p : frameProfiles) {
					fprintf(s, "%llu,%lld,%lld\n", n++, p.codeTime, p.renderTime);
				}
				fclose(s);
			});
		#endif
	}
}
