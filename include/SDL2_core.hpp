#ifndef SDL2_CORE_HPP
#define SDL2_CORE_HPP

#include <SDL2/SDL.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#ifndef NDEBUG
#define DBGMSG(msg)\
	std::cout << "[SDL2_CORE_DEBUG] " << msg << "\n";
#else
#define DBGMSG(msg)
#endif

namespace SDL2_Core {

class Renderer {
private:

	using Surface = std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>;
	using Texture = std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)>;

	SDL_Renderer* ren;
	friend class Window;
	std::map<std::string, Texture> textures_map;
	Renderer(SDL_Renderer* ren) : ren(ren) {}
public:
	void load_texture(std::string path) {
		auto maybe_tex = textures_map.find(path);
		if (maybe_tex != textures_map.end()) {
			DBGMSG(path << " was loaded earlier.");
			return;
		}
		auto sur = Surface(
			[&](){
				auto s = SDL_LoadBMP(path.data());
				if (!s) throw std::runtime_error("Failed to load bmp.");
				return s;
			}(),
			[](SDL_Surface* s) {
				if (s) SDL_FreeSurface(s);
			}
		);
		auto tex = Texture(
			[&](){
				auto t = SDL_CreateTextureFromSurface(ren, sur.get());
				if (!t) throw std::runtime_error("Failed to create texture.");
				return t;
			}(),
			[](SDL_Texture* t) {
				if (t) SDL_DestroyTexture(t);
			}
		);
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
	~Sdl() {
		SDL_Quit();
		DBGMSG("SDL2 terminated.");
	}
};

}

#endif
