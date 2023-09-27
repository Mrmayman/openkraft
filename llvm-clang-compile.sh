#!/usr/bin/sh
clang src/main.cpp\
      src/entities.cpp\
      src/chunk.cpp\
      src/networkingFunctions.cpp\
      -lSDL2 -lSDL2_image -lGL -lGLU -lGLEW -lSDL2main -lSDL2_ttf -lstdc++ -lm -lz\
      -o ./bin/a.out
