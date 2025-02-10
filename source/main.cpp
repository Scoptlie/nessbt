
#include <chrono>

#include "Common.h"
#include "Interface.h"
#include "Ppu.h"

int main(int argc, char **argv) {
	Interface::init();
	Ppu::init();
	
	auto updateSep = 16'666'667ll;
	auto nextUpdate = 0ll;
	
	while (!Interface::quitRequest) {
		using namespace std::chrono;
		
		auto now = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
		if (now >= nextUpdate) {
			Ppu::frameDone = false;
			while (!Ppu::frameDone) {
				Ppu::tick();
			}
			
			Interface::update();
			
			nextUpdate = ((now / updateSep) + 1) * updateSep;
		}
	}
	
	Ppu::deinit();
	Interface::deinit();
	
	return 0;
}
