#include "Controller.h"

#include <SDL2/SDL.h>

namespace Controller {
	U8 state;
	
	void strobe() {
		auto keys = SDL_GetKeyboardState(nullptr);
		
		state =
			keys[SDL_SCANCODE_Z] |
			(keys[SDL_SCANCODE_X] << 1) |
			(keys[SDL_SCANCODE_RSHIFT] << 2) |
			(keys[SDL_SCANCODE_RETURN] << 3) |
			(keys[SDL_SCANCODE_UP] << 4) |
			(keys[SDL_SCANCODE_DOWN] << 5) |
			(keys[SDL_SCANCODE_LEFT] << 6) |
			(keys[SDL_SCANCODE_RIGHT] << 7);
	}
	
	U8 data() {
		auto r = state & 1;
		state >>= 1;
		return r;
	}
	
	void init() {
		state = 0;
	}
	
	void deinit() {
		
	}
}
