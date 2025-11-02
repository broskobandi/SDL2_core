#ifndef SDL2_CORE_HPP
#define SDL2_CORE_HPP

#include <SDL2/SDL.h>
#include <stdexcept>
#include <iostream>

#ifndef NDEBUG
#define DBGMSG(msg)\
	stsd::cout << "[SDL2_CORE_DEBUG] " << (msg) << "\n";
#else
#define DBGMSG(msg)
#endif

namespace SDL2_Core {

class Sdl {
public:
	Sdl(Uint32 flags) {
		if (SDL_Init(flags))
			throw std::runtime_error("Failed to init SDL.");
	}
}

}

#endif
