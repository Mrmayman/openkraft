g++ src/main.cpp\
    src/entities.cpp\
    src/chunk.cpp\
    -lSDL2 -lSDL2_image -lGL -lGLU -lGLEW -lSDL2main -lSDL2_ttf -lz\
    -Wall -Wextra -Werror -Wno-unused-variable -fsanitize=address -g\
    -o ./bin/a.out
