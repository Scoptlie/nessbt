#include "env.h"

#include <chrono>
#include <thread>

#include "cart.h"
#include "controller.h"
#include "interface.h"
#include "ppu.h"

namespace Env {
	auto nextFrame = 0ll;
	
	void init() {
		Interface::init();
		Controller::init();
		Cart::init("roms/smb.nes");
		Ppu::init();
	}
	
	void update(USize nCycles) {
		auto frameSep = 16'666'667ll;
		
		while (nCycles > 0) {
			for (int i = 0; i < 3; i++) {
				Ppu::tick();
				if (Ppu::frameDone) {
					using namespace std::chrono;
					
					long long now;
					do {
						now = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
						std::this_thread::sleep_for(microseconds(200));
					} while (now < nextFrame);
					
					nextFrame = ((now / frameSep) + 1) * frameSep;
					
					Interface::update();
					Ppu::frameDone = false;
				}
			}
			
			nCycles--;
		}
	}
}
