/*
MIT License

Copyright (c) 2025 broskobandi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/** @file include/SDL2_core.hpp
 * @brief Public header file for the SDL2_core library. */

#ifndef SDL2_CORE_HPP
#define SDL2_CORE_HPP

#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Debug messages are only printed when NDEBUG and TEST are not defined.
// When TEST is defined, debug messages are saved in a thread local 
// string object.

#ifndef NDEBUG
#ifndef TEST
#include <iostream>
#define DBGMSG(msg)\
	std::cout << "[SDL2_CORE_DEBUG] " << (msg) << "\n";
#else
#define DBGMSG(msg)\
	dbg_msg = (msg)
#endif
#else
#define DBGMSG(msg)
#endif

namespace SDL2_Core {

#ifdef TEST
	inline thread_local std::string dbg_msg;
#endif

/** Class to manage SDL2 resources and behaviour. */
class Sdl {

private:

	// Custom types

	using Texture = std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)>;
	using Surface = std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>;
	using Font = std::unique_ptr<TTF_Font, void(*)(TTF_Font*)>;
	using Renderer = std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)>;
	using Window = std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>;
	class Base {
		friend class Sdl;
		Base(Uint32 flags) {
			if (SDL_Init(flags))
				throw std::runtime_error("Failed to initialize SDL2.");
			if (TTF_Init())
				throw std::runtime_error("Failed to initialize TTF.");
			DBGMSG("SDL2 and TTF initialized.");
		}
		~Base() {
			TTF_Quit();
			SDL_Quit();
			DBGMSG("SDL2 and TTF terminated.");
		}
	};
	
	// Variables

	Base base;
	Window win;
	Renderer ren;
	std::map<std::string, Texture> textures_map;

	// Private methods
	
	Texture create_texture(const Surface& surface) {
		return Texture(
			[&](){
				auto t = SDL_CreateTextureFromSurface(ren.get(), surface.get());
				if (!t) throw std::runtime_error("Failed to create texture.");
				DBGMSG("Texture created.");
				return t;
			}(),
			[](SDL_Texture* t) {
				if (t) SDL_DestroyTexture(t);
				DBGMSG("Texture destroyed.");
			}
		);
	}
	
public:

	// Structs and enums

	/** Struct to group data to be passed to the draw functions. */
	struct RenderData {

		/** The portion of the texture to be rendered.
		 * If nullopt, the whole texture gets rendered. */
		std::optional<SDL_Rect> srcrect {std::nullopt};

		/** The target rect to render the texture over or fill with color.
		 * If nullopt, the whole rendering target gets filled. */
		std::optional<SDL_Rect> dstrect {std::nullopt};

		/** Variable to hold either a color or path to the texture to be used. */
		std::variant<SDL_Color, std::string>  col_or_tex {SDL_Color{255, 0, 0, 255}};

		/** The angle by which the texture should be rotated. */
		float angle {0.0f};

		/** The texture's flip state. */
		SDL_RendererFlip flip {SDL_FLIP_NONE};

		/** Text to be rendered.
		 * If nullopt, texture will be considered as non-text. */
		std::optional<std::string> text;

		/** Size of the font to be rendered.
		 * If nullopt, texture will be considered as non-text. */
		std::optional<int> ptsize;
	};

	// Constructor

	/** Instantiates an Sdl object.
	 * @param title The title of the window.
	 * @param w The width of the window. 
	 * @param h The height of the window.
	 * @throws std::runtime_error on failure. */
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

	// Public methods

	/** Loads and stores a texture.
	 * @param path The path to the bmp to be used.
	 * @throws std::runtime_error on failure. */
	void load_texture(const std::string& path) {
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
		auto tex = create_texture(sur);
		textures_map.emplace(path, std::move(tex));
		DBGMSG("New texture loaded.");
	}

	/** Loads a vector of textures.
	 * @param paths A vector of strings containing the paths to be used.
	 * @throws std::runtime_error on failure. */
	void load_texture(const std::vector<std::string>& paths) {
		for (auto path : paths) {
			load_texture(path);
		}
	}

	/** Loads text for later use.
	 * @param text The text to be rendered. 
	 * @param col The color of the text.
	 * @param path_to_font Path to the .ttf font to be used. 
	 * @param ptsize Size of the font as ptsize. 
	 * @return A rect appropriately sized to fit the text.
	 * @throws std::runtime_error on failure. */
	SDL_Rect load_text(
		std::string text,
		SDL_Color col,
		SDL_Point pos,
		std::string path_to_font,
		int ptsize)
	{
		auto maybe_text = textures_map.find(text);
		if (maybe_text != textures_map.end())
			throw std::runtime_error("Text cannot be loaded twice.");
		auto font = Font(
			[&](){
				auto f = TTF_OpenFont(path_to_font.data(), ptsize);
				if (!f) throw std::runtime_error("Faield to load font.");
				DBGMSG("Font opened.");
				return f;
			}(),
			[](TTF_Font* f) {
				if (f) TTF_CloseFont(f);
			}
		);
		auto sur = Surface(
			[&](){
				auto s = TTF_RenderText_Blended(font.get(), text.data(), col);
				if (!s) throw std::runtime_error("Failed to create text surface.");
				DBGMSG("Text surface created.");
				return s;
			}(),
			[](SDL_Surface* s){
				if (s) SDL_FreeSurface(s);
			}
		);
		auto tex = create_texture(sur);
		SDL_Rect rect = {pos.x, pos.y, 0, 0};
		if (TTF_SizeText(font.get(), text.data(), &rect.w, &rect.h))
			throw std::runtime_error("Failed to set text rect size.");
		textures_map.emplace(text, std::move(tex));
		DBGMSG("Text loaded.");
		return rect;
	}

	/** Sets the renderer's draw color.
	 * @param col The color to be used.
	 * @throws std::runtime_error on failure. */
	void set_draw_color(SDL_Color col) {
		if (SDL_SetRenderDrawColor(ren.get(), col.r, col.g, col.b, col.a))
			throw std::runtime_error("Failed to set draw color.");
	}

	/** Clears the renderer with the specified color.
	 * @param col The color to be used.
	 * @throws std::runtime_error on failure. */
	void clear(SDL_Color col) {
		set_draw_color(col);
		if (SDL_RenderClear(ren.get()))
			throw std::runtime_error("Failed to clear renderer.");
	}

	/** Draws based on the specified renderer data.
	 * @param data The renderer data to be used.
	 * @throws std::runtime_error on failure. */
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

	/** Presents the rendered objects. */
	void present() {
		SDL_RenderPresent(ren.get());
	}
};

}

#endif
