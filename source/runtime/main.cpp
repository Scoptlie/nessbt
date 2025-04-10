
#include "runtime/cpu.h"
#include "runtime/env.h"

int main(int argc, char **argv) {
	Env::init();
	Cpu::run();
	
	return 0;
}
