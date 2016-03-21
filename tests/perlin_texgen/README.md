# perlin_texgen

Commandline FBM texture generator. You can generate RGB image with perlin noise in each channel. Outputs image in PNG format in current directory.

### Arguments
1) file name (string)
2) width (int)
3) height (int)
4) persistence 0.1 - 1.0 (float)
5) octaves 1-inf (int)
6) tiled (true/false)

### Example output:
`perlin_texgen "out.png" 256 256 0.7 7 true`
![Preview](/out.png?raw=true "Preview")

### Building
- build granite (see: [Building Granite](https://github.com/c41x/cpptoolset/README.md))
- run `cmake --build "project/release/tests/perlin_textgen" --config Release`
