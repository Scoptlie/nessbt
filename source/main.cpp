
#include <chrono>
#include <thread>

#include "cart.h"
#include "cpu.h"
#include "interface.h"
#include "ppu.h"

int main() {
	Interface::init();
	Cart::init("roms/dk.nes");
	Ppu::init();
	Cpu::init();
	
	auto frameSep = 16'666'667ll;
	auto nextFrame = 0ll;
	
	while (!Interface::quitRequest) {
		using namespace std::chrono;
		
		auto now = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
		if (now >= nextFrame) {
			while (!Ppu::frameDone) {
				Cpu::handleInt();
				Cpu::emuInstr();
				while (Cpu::nCycles > 0) {
					Ppu::tick();
					//Ppu::tick();
					//Ppu::tick();
					Cpu::nCycles--;
				}
			}
			
			Ppu::frameDone = false;
			
			Interface::update();
			
			nextFrame = ((now / frameSep) + 1) * frameSep;
		}
		
		std::this_thread::sleep_for(milliseconds(1));
	}
	
	return 0;
}
