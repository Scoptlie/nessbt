nessbt is an experimental static binary translator and runtime for NES games, written for an undergraduate dissertation project. It supports a variety of mapper 0 games.

Inspired by [Jamulator](https://github.com/andrewrk/jamulator), and the increasing number of static recompilation projects for more complicated video game consoles.

This project was developed on Windows using [MSYS 2](https://www.msys2.org/). It has been known to build on Linux with some tweaking to the linker flags specified in the `build.py` file. You may need to link additional libraries or change the names of existing ones.

# Prerequisites for use
Ensure fairly recent versions of [Go](https://go.dev/dl/) and [Python](https://www.python.org/downloads/) are installed on your computer as well as C and C++ compilers with command line interfaces compatible with GCC (clang shall work). Your C++ compiler should probably support C++20, but you might be able to get away with using an earlier standard if you don't mind changing the compiler flags in `build.py`.

Make sure you have the following libraries installed through your package manager:
- SDL2
- OpenGL

# How to use
Parameters are given to the build script using environment variables. At minimum you have to set these ones:
- `ROM` - The path of the ROM file (.nes) to translate
- `PALETTE` - The path of the palette file (.pal) to use
	- I like [this](https://pixeltao.itch.io/pixeltao-cxa-nes-palette) one, personally
- `C_COMPILER` - The path to your C compiler front-end (e.g. `gcc`, `clang`)
- `CPP_COMPILER` - The path to your C++ compiler front-end (e.g. `g++`, `clang++`)
- `LINKER` - Also the path to your C++ compiler front-end in all likelihood

You probably also want this one:
- `USE_OPTIMISATIONS` - Set to 1 to enable compiler optimisations

Frame time statistics can be dumped with this:
- `OUTPUT_FRAME_PROFILES`

Profile-guided translation is supported using these variables. Just don't enable outputs and inputs in the same build:
- `OUTPUT_JUMP_TARGETS` - Set to 1 to have the runtime dump a list of jump targets
- `OUTPUT_NMI_LOCATIONS` - Set to 1 to have the runtime dump a list of basic blocks that were interrupted by an NMI
- `INPUT_JUMP_TARGETS` - Set to 1 to use the previously-dumped list of jump targets to optimise the translation
- `INPUT_NMI_LOCATIONS` - Set to 1 to use the previously-dumped list of NMI locations to optimise the translation

Other miscellaneous options (use zero or one of these):
- `ONLY_USE_EMULATION` - Disable execution of statically translated code. This turns the runtime into a regular emulator
- `ONLY_USE_TRANSLATIONS` - Disable falling back on emulation for untranslated code (likely to cause crashes)

With environment variables set, invoke `build.py` using your Python interpreter. With luck, the generated binary with the ROM and translations built into it will be at `gen/a.exe`.
