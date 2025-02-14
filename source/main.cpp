
#include "cart.h"
#include "cpu.h"

int main() {
	Cart::init("roms/pacman.nes");
	Cpu::init();
	
	while (true) {
		Cpu::emuInstr();
	}
	
	return 0;
}
