#g++ src/main.cpp\
#    src/entities.cpp\
#    src/chunk.cpp\
#    -lSDL2 -lSDL2_image -lGL -lGLU -lGLEW -lSDL2main -lSDL2_ttf -O3 -lz\
#    -o bin/a.out
g++ src/main.cpp\
    src/entities.cpp\
    src/chunk.cpp\
    -include ./lib/FastNoiseLite.h\
    -lSDL2 -lSDL2_image -lGL -lGLU -lGLEW -lSDL2main -lSDL2_ttf -O3 -lz\
    -o bin/a.out    
