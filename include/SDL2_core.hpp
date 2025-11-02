#ifndef SDL2_CORE_HPP
#define SDL2_CORE_HPP

#include <SDL2/SDL.h>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#ifndef NDEBUG
#ifndef TEST
#include <iostream>
#define DBGMSG(msg)\
	std::cout << "[SDL2_CORE_DEBUG] " << (msg) << "\n";
#else
inline thread_local std::string dbg_msg;
#define DBGMSG(msg)\
	dbg_msg = (msg)
#endif
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
	struct RenderData {
		std::string_view path_to_bmp;
		std::optional<SDL_Rect> srcrect {std::nullopt};
		std::optional<SDL_Rect> dstrect {std::nullopt};
		std::variant<SDL_Color, std::string>  col_or_tex {SDL_Color{255, 0, 0, 255}};
		float angle {0.0f};
		SDL_RendererFlip flip {SDL_FLIP_NONE};
	};
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
			DBGMSG("Texture was loaded earlier.");
			return;
		}
		auto sur = Surface(
			[&](){
				auto s = SDL_LoadBMP(path.data());
				if (!s) throw std::runtime_error("Failed to load bmp.");
				DBGMSG("bmp loaded:");
				DBGMSG(path);
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
		DBGMSG("New texture loaded.");
	}
	void load_texture(std::vector<std::string> paths) {
		for (auto path : paths) {
			load_texture(path);
		}
	}
	void set_draw_color(SDL_Color col) {
		if (SDL_SetRenderDrawColor(ren.get(), col.r, col.g, col.b, col.a))
			throw std::runtime_error("Failed to set draw color.");
	}
	void clear(SDL_Color col) {
		set_draw_color(col);
		if (SDL_RenderClear(ren.get()))
			throw std::runtime_error("Failed to clear renderer.");
	}
	void draw(RenderData data) {
		SDL_Rect *srcrect = data.srcrect.has_value() ? &data.srcrect.value() : nullptr;
		SDL_Rect *dstrect = data.dstrect.has_value() ? &data.dstrect.value() : nullptr;
		if (std::holds_alternative<std::string>(data.col_or_tex)) {
			auto tex = textures_map.find(std::get<std::string>(data.col_or_tex));
			if (tex == textures_map.end())
				throw std::runtime_error("Failed to find texture.");
			if (
				SDL_RenderCopyEx(
					ren.get(),
					tex->second.get(),
					srcrect,
					dstrect,
					data.angle,
					nullptr,
					data.flip
				)
			)
				throw std::runtime_error("Failed to render texture.");
			DBGMSG("Texture rendered.");
		} else {
			SDL_Color col = std::get<SDL_Color>(data.col_or_tex);
			set_draw_color(col);
			if (SDL_RenderFillRect(ren.get(), dstrect))
				throw std::runtime_error("Failed to fill rect.");
			DBGMSG("Rect rendered.");
		}
	}
	void present() {
		SDL_RenderPresent(ren.get());
	}
};

}

#endif
