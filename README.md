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
- streams
- VFS / LZ4 compression
- multithread task scheduler

## Requirements:
- Platform: Windows / Linux
- Compiler: C++14 compilant: GCC 4.9+, MinGW 4.9+, VS 2015+

## Installation

### Submodules
Repository uses submodules. To download all dependencies run `git submodule init` followed by `git submodule update` right after clonning.
More info: http://git-scm.com/book/en/v2/Git-Tools-Submodules

### Preparing project and compiling dependencies
- go to `project` directory.
- run script `init.bat` (on Windows) or `sh init.sh` (on Linux)

### Building
- run `cmake --build "project/release" --config Release`