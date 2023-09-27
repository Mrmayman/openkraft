#pragma once

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glu.h>

#include "../../lib/FastNoiseLite.h"
#include "../networking/networkingFunctions.h"
#include "../entities.h"

extern FastNoiseLite terrainNoise;

extern Entity *entities[1024];
extern Entity *commonEntities[1024];

class Camera
{
public:
    float FOV = 90;

    double X = 0;
    double Y = 0;
    double Z = 0;

    float rotX = 0;
    float rotY = 0;
};

class GameRenderer
{
public:
    bool init()
    {
        if (!(initSDL() && initOpenGL() && initFont() && initMultiplayer()))
        {
            return false;
        }

        terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        terrainNoise.SetSeed(seed);

        caveNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        // caveNoise.SetFrequency(0.02f); // Adjust this value to control the scale
        // caveNoise.SetAmplitude(1.0f);  // Adjust this value to control the intensity

        // You can also adjust other parameters like octaves, lacunarity, and gain
        // caveNoise.SetFractalOctaves(4);
        // caveNoise.SetFractalLacunarity(2.0f);
        // caveNoise.SetFractalGain(0.5f);

        texAtlas = loadTexture("terrain.png");

        buildShader();

        return true;
    }

    void cleanup()
    {
        if (isMultiplayer)
        {
            std::vector<uint8_t> quitData = {0xff};
            net::appendString16("Quitting", quitData);
        }

        for (int i = 0; i < int(sizeof(entities) / (sizeof(Entity))); i++)
        {
            delete entities[i];
        }
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void drawText(int x, int y, const std::string &text, SDL_Color color);

    bool isMultiplayer = 0;
    int seed = 69;

    SDL_Window *window;
    SDL_GLContext context;

    GLuint texAtlas;
    FastNoiseLite terrainNoise;
    FastNoiseLite caveNoise;

    GLuint shaderProgram;

    Camera camera;

private:
    void buildShader();
    bool initMultiplayer();
    bool initOpenGL();
    bool initSDL();
    bool initFont();
    GLuint LoadShader(GLenum type, const char *source);
    GLuint loadTexture(const std::string &filePath);

    TTF_Font *font;
    std::string GamePath;
};

extern GameRenderer myRenderer;