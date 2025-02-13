
#include <chrono>
#include <thread>

#include "Cart.h"
#include "Common.h"
#include "Interface.h"
#include "Ppu.h"
#include "Cpu.h"

int main(int argc, char **argv) {
	Interface::init();
	Cart::init("roms/dk.nes");
	Ppu::init();
	Cpu::init();
	
	auto updateSep = 16'666'667ll;
	auto nextUpdate = 0ll;
	
	while (!Interface::quitRequest) {
		using namespace std::chrono;
		
		while (!Ppu::frameDone) {
			Cpu::step();
			
			while (Cpu::nCycle > 0) {
				Ppu::tick();
				Ppu::tick();
				Ppu::tick();
				
				Cpu::nCycle--;
			}
		}
		
		Ppu::frameDone = false;
		
		long long now;
		while (true) {
			now = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
			if (now >= nextUpdate) {
				break;
			}
		}
		
		nextUpdate = ((now / updateSep) + 1) * updateSep;
		
		Interface::update();
	}
	
	Cpu::deinit();
	Ppu::deinit();
	Cart::deinit();
	Interface::deinit();
	
	return 0;
}
