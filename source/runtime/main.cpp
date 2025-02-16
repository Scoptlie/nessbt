
#include "runtime/cpu.h"
#include "runtime/env.h"

int main() {
	Env::init();
	Cpu::run();
	
	return 0;
}
