#include "SDL2_core.hpp"
#include <ctest.h>
#include <iostream>
#include <string>
#include <vector>

using namespace SDL2_Core;

int main(void) {
	try {
	Sdl sdl("test", 800, 600);
	CTEST(dbg_msg == "Renderer created.");

	sdl.load_texture("../assets/face.bmp");
	CTEST(dbg_msg == "New texture loaded.");

	sdl.load_texture("../assets/face.bmp");
	CTEST(dbg_msg == "Texture was loaded earlier.");

	std::vector<std::string> paths;

	paths.push_back("../assets/face2.bmp");
	paths.push_back("../assets/face3.bmp");

	sdl.load_texture(paths);
	CTEST(dbg_msg == "New texture loaded.");

	Sdl::RenderData data;
	data.dstrect = SDL_Rect{0, 0, 50, 50};
	data.col_or_tex = "../assets/face2.bmp";

	sdl.draw(data);
	CTEST(dbg_msg == "Texture rendered.");

	data.col_or_tex = SDL_Color{100, 0, 0, 255};

	sdl.draw(data);
	CTEST(dbg_msg == "Rect rendered.");

	SDL_Rect text_rect =
		sdl.load_text(
			"some text", {100, 100, 100, 255}, {0, 0}, "../MononokiNerdFont-Regular.ttf", 99
	);
	CTEST(dbg_msg == "Text loaded.");

	data.col_or_tex = "some text";
	data.dstrect = text_rect;

	sdl.draw(data);
	CTEST(dbg_msg == "Texture rendered.");

	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << "\n";
	}

	ctest_print_results();
	return 0;
}
