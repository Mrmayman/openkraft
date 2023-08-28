g++ src/main.cpp\
    src/entities.cpp\
    src/chunk.cpp\
    -lSDL2 -lSDL2_image -lGL -lGLU -lGLEW -lSDL2main -lSDL2_ttf -lz -pg -g\
&& ./bin/a.out && gprof ./bin/a.out gmon.out > profileReport.txt && rm gmon.out
