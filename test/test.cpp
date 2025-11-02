#include "SDL2_core.hpp"
#include <ctest.h>
#include <iostream>
#include <string>
#include <vector>

using namespace SDL2_Core;

int main(void) {
	try {
	Sdl sdl("test", 800, 600);

	sdl.load_texture("../assets/face.bmp");
	sdl.load_texture("../assets/face.bmp");

	std::vector<std::string> paths;

	paths.push_back("../assets/face2.bmp");
	paths.push_back("../assets/face3.bmp");

	sdl.load_texture(paths);

	Sdl::RenderData data;
	data.dstrect = SDL_Rect{0, 0, 50, 50};
	data.col_or_tex = "../assets/face2.bmp";

	sdl.draw(data);

	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << "\n";
	}

	ctest_print_results();
	return 0;
}
