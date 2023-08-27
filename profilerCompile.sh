g++ src/main.cpp\
    src/entities.cpp\
    src/chunk.cpp\
    -lSDL2 -lSDL2_image -lGL -lGLU -lGLEW -lSDL2main -lSDL2_ttf -lz -pg -g\
&& ./a.out && gprof a.out gmon.out > profileReport.txt && grep -v "FastNoiseLite" profileReport.txt > filtered_profile_report.txt
