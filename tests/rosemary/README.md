# rosemary

Fragment shader toy application.

### Arguments
1. fragment shader file path (string)
2. window width (int)
3. window height (int)
4. monitor ID (-1 to default)
5. debug output (true/false)

### Example output:
`rosemary "sss.frag" 256 256 -1 false`

![Preview](https://github.com/c41x/cpptoolset/blob/master/tests/rosemary/out.png?raw=true "Preview")

Run `rosemary` to print available monitors

### Building
- build granite (see: [Building Granite](https://github.com/c41x/cpptoolset/blob/master/README.md))
- run `cmake --build "project/release/tests/rosemary" --config Release`
