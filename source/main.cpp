
#include "cpu.h"
#include "env.h"

int main() {
	Env::init();
	Cpu::run();
	
	return 0;
}
