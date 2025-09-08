# ludoCpp

This is a 2D ludo classic game made with c++ and the sdl3 library.

Instructions: (requires cmake)
- git clone this repo
- execute the script in the external/SDL_image/external to (`download.sh` or `Get-GitModules.ps1` on windows)
- make a subdirectory called 'build' inside of the repo
- copy all the files under assets into build directory
- cd into build
- 'cmake -DSDLIMAGE_AVIF=OFF -B . -S .. '  (using clang and ninja works well -- recomended)
- 'cmake --build .'
the executable should now be in the build directory

