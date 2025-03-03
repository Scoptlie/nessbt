#include "env.h"

#include <chrono>
#include <thread>

#include "runtime/controller.h"
#include "runtime/interface.h"
#include "runtime/ppu.h"
#include "runtime/profiler.h"

namespace Env {
	long long nextFrame;
	
	void init() {
		using namespace std::chrono;
		
		Interface::init();
		Controller::init();
		Ppu::init();
		Profiler::init();
		
		Profiler::beginCode();
	}
	
	void frame() {
		using namespace std::chrono;
		
		auto frameSep = 16'666'667ll;
		
		long long now;
		do {
			now = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
			std::this_thread::sleep_for(microseconds(200));
		} while (now < nextFrame);
		
		nextFrame = ((now / frameSep) + 1) * frameSep;
		
		Interface::update();
		Ppu::frameDone = false;
	}
	
	void update(USize nCycles) {
		while (nCycles > 0) {
			for (int i = 0; i < 3; i++) {
				Ppu::tick();
				if (Ppu::frameDone) {
					Profiler::endCode();
					
					frame();
					
					Profiler::beginCode();
				}
			}
			nCycles--;
		}
	}
}
