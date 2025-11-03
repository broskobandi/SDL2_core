cmake_minimum_required(VERSION 3.20)
project(SDL2_core)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(test EXCLUDE_FROM_ALL test/test.cpp)
target_link_libraries(test PRIVATE SDL2 SDL2_ttf ctest)
target_compile_options(test PRIVATE -Wall -Wextra -Werror -Wunused-result -Wconversion)
target_compile_definitions(test PRIVATE TEST)
target_include_directories(test PRIVATE include)

install(FILES include/SDL2_core.hpp DESTINATION include)
