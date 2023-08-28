# OpenKraft
A basic Minecraft clone in C++ and OpenGL.
This project is fully free and open source under the GPL3.

## Features
### Singleplayer
- 3D Noise based world generation (only stone)
- Movement (walking, looking, jumping)
- Infinite terrain and chunk loading
- 32x32x32 cubic chunks. (Infinite height limit)
### Multiplayer
- Supports joining Minecraft beta 1.7.3 servers
- Chunk loading
- Movement (walking, looking, jumping) (kinda glitchy)
- Live block changes (If someone places/breaks a block, you will see it)

## Disclaimer
I am very inexperienced at programming. This code is very poorly written. While I am trying to improve it, expect horrible bugs.
Currently some chunk packets aren't supported in multiplayer, so the game will randomly close. Just reopen the game when this happens, for the time being. I am working on fixing this.

## Building and Support
- Currently only supported on Linux
- Will support Windows soon
### Build scripts
- `compile.sh` - Release build, maximum optimizations (linux, x86_64)
- `debug-compile.sh` - Debug build, for use with gdb. Memory leak detection included (linux, x86_64)
- `llvm-clang-compile.sh` - Release build, compiled with clang (linux, x86_64)
- `profiler-compile.sh` - Profiler build. Once you close the game, a profile report `profileReport.txt` will be created. Useful for analysing the performance.
Just run these scripts, no arguments needed. The executable will be in `./bin/a.out`
### Required libraries
- SDL
- OpenGL
- GLEW
- GLU
- zlib