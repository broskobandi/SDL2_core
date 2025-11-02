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
#include <vector>

#ifndef NDEBUG
#define DBGMSG(msg)\
	std::cout << "[SDL2_CORE_DEBUG] " << msg << "\n";
#else
#define DBGMSG(msg)
#endif

namespace SDL2_Core {

class Sdl {
private:
	using Texture = std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)>;
	using Surface = std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>;
	using Renderer = std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)>;
	using Window = std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>;
	class Base {
		friend class Sdl;
		Base(Uint32 flags) {
			if (SDL_Init(flags))
				throw std::runtime_error("Failed to initialize SDL2.");
			DBGMSG("SDL2 initialized.");
		}
		~Base() {
			SDL_Quit();
			DBGMSG("SDL2 terminated.");
		}
	};

	Base base;
	Window win;
	Renderer ren;
	std::map<std::string, Texture> textures_map;
public:
	Sdl(std::string_view title, int w, int h) :
		base(SDL_INIT_EVERYTHING),
		win(
			[&](){
				auto wi = SDL_CreateWindow(title.data(), 0, 0, w, h, SDL_WINDOW_SHOWN);
				if (!wi) throw std::runtime_error("Failed to create window.");
				DBGMSG("Window created.");
				return wi;
			}(),
			[](SDL_Window* w) {
				if (w) SDL_DestroyWindow(w);
				DBGMSG("Window destroyed.");
			}
		),
		ren(
			[&](){
				auto r = SDL_CreateRenderer(win.get(), -1, SDL_RENDERER_PRESENTVSYNC);
				if (!r) throw std::runtime_error("Failed to create renderer.");
				DBGMSG("Renderer created.");
				return r;
			}(),
			[](SDL_Renderer* r) {
				if (r) SDL_DestroyRenderer(r);
				DBGMSG("Renderer destroyed.");
			}
		)
	{};
	void load_texture(std::string path) {
		auto maybe_tex = textures_map.find(path);
		if (maybe_tex != textures_map.end()) {
			DBGMSG("Texture for " << path << " was loaded earlier.");
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
				auto t = SDL_CreateTextureFromSurface(ren.get(), sur.get());
				if (!t) throw std::runtime_error("Failed to create texture.");
				return t;
			}(),
			[](SDL_Texture* t) {
				if (t) SDL_DestroyTexture(t);
				DBGMSG("Texture destroyed.");
			}
		);
		textures_map.emplace(path, std::move(tex));
		DBGMSG("Texture loaded for: " << path);
	}
	void load_texture(std::vector<std::string> paths) {
		for (auto path : paths) {
			load_texture(path);
		}
	}
};

}

#endif
