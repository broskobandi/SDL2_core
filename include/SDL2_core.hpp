#ifndef SDL2_CORE_HPP
#define SDL2_CORE_HPP

#include <SDL2/SDL.h>
#include <stdexcept>
#include <iostream>
#include <string_view>
#include <utility>

#ifndef NDEBUG
#define DBGMSG(msg)\
	std::cout << "[SDL2_CORE_DEBUG] " << (msg) << "\n";
#else
#define DBGMSG(msg)
#endif

namespace SDL2_Core {

class Texture {
private:
	SDL_Texture* tex;
	friend class Renderer;
	Texture(SDL_Texture* tex) : tex(tex) {}
public:
	~Texture() {
		if (tex)
			SDL_DestroyTexture(tex);
	}
};

class Surface {
private:
	SDL_Surface* sur;
	friend class Sdl;
	Surface(SDL_Surface* sur) : sur(sur) {}
public:
	SDL_Surface* get() const {
		return sur;
	}
	~Surface() {
		if (sur)
			SDL_FreeSurface(sur);
	}
};

class Renderer {
private:
	SDL_Renderer* ren;
	friend class Window;
	Renderer(SDL_Renderer* ren) : ren(ren) {}
public:
	Texture create_texture(const Surface& sur) {
		auto t = SDL_CreateTextureFromSurface(ren, sur.get());
		if (!t) throw std::runtime_error("Failed to create texture.");
		return t;
	}
	~Renderer() {
		if (ren)
			SDL_DestroyRenderer(ren);
		DBGMSG("Renderer destroyed.");
	}
};

class Window {
private:
	SDL_Window* win;
	friend class Sdl;
	Window(SDL_Window* win) : win(win) {}
public:
	Renderer renderer(Uint32 flags) {
		auto r = SDL_CreateRenderer(win, -1, flags);
		if (!r) throw std::runtime_error("Failed to create renderer.");
		DBGMSG("Renderer created.");
		return r;
	}
	std::pair<int, int> size() {
		int w, h;
		SDL_GetWindowSize(win, &w, &h);
		return {w, h};
	}
	~Window() {
		if (win)
			SDL_DestroyWindow(win);
		DBGMSG("Window destroyed.");
	}
};

class Sdl {
public:
	Sdl(Uint32 flags) {
		if (SDL_Init(flags))
			throw std::runtime_error("Failed to init SDL.");
		DBGMSG("SDL2 initialized.");
	}
	Window window(std::string_view title, int w, int h, Uint32 flags) {
		auto win = SDL_CreateWindow(title.data(), 0, 0, w, h, flags);
		if (!win) throw std::runtime_error("Failed to create window.");
		DBGMSG("Window created.");
		return win;
	}
	Surface load_bmp(std::string_view path) {
		auto s = SDL_LoadBMP(path.data());
		if (!s) throw std::runtime_error("Failed to load bmp.");
		return s;
	}
	~Sdl() {
		SDL_Quit();
		DBGMSG("SDL2 terminated.");
	}
};

}

#endif
