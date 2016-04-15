# cpptoolset (my common C++ libs)

## Components:
- math library (SSE optimized) (vectors, matrices, 2D/3D, bounding shapes, frustum and many more)
- image encoding / decoding (PNG, JPEG, BMP, TGA)
- logger with signal handling
- signal - slots, events
- fast string manipulation utilities
- high frequency timers
- hardware info utility
- LISP interpreter
- random number generators
- perlin noise generators, FBM generators
- file system utilities
- directory watch
- streams
- VFS / LZ4 compression
- multithread task scheduler
- system hotkeys

## Requirements:
- Platform: Windows / Linux
- Compiler: C++14 compilant: GCC 4.9+, MinGW 4.9+, VS 2015+

## Installation

### Submodules
Repository is using submodules. To download all dependencies run `git submodule init` followed by `git submodule update` after clonning.
More info: http://git-scm.com/book/en/v2/Git-Tools-Submodules

### Preparing project and compiling dependencies
- install NASM (nasm executable must be in path)
- install OpenGL development files (libgl1-mesa-dev, mesa-common-dev)
- install X11 development files (xorg-dev)
- build and install glbinding (https://github.com/cginternals/glbinding)
- go to `project` directory.
- run script `init.bat` (on Windows) or `sh init.sh` (on Linux)

### Building
- run `cmake --build "project/release" --config Release`