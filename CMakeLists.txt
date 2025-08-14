cmake_minimum_required(VERSION 3.31)
project(LudoCpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# for clangd support
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(SOURCES src/main.cpp src/controller.cpp
            src/model.cpp src/view.cpp
)


# TODO: copy assets to build folder automatically at generation time

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
add_subdirectory(external/SDL_image EXCLUDE_FROM_ALL)

# Create your game executable target as usual
add_executable(ludo ${SOURCES})

# Link to the actual SDL3 library.
target_link_libraries(ludo PRIVATE SDL3_image::SDL3_image SDL3::SDL3)
target_compile_options(ludo PRIVATE -Werror -Wall
  -Wextra -pedantic -g -O0 # -O3
)
