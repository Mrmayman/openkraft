#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glu.h>
#include <zlib.h>

#include <iostream>
#include <cstdio>
#include <cmath>
#include <string>
#include <csignal>
#include <exception>
#include <cstdint>
#include <iomanip>

#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>

#include "../include/chunk.h"
#include "../include/entities.h"
#include "../include/facedraw.h"
#include "../include/init.h"
#include "../include/graphics.h"
#include "../include/networkingMain.h"

float cameraX = 0;
float cameraY = 0;
float cameraZ = 0;

float rotX = 0;
float rotY = 0;

bool quit = false;

float mouseX = 0;
float mouseY = 0;

float delta = 1;

void manageGame()
{
    // std::lock_guard<std::mutex> lock(chunkMapMutex);
    while (!quit)
    {
        if (isMultiplayer)
        {
            runMultiplayer();
        }
        else
        {
            chunk::unload();
            chunk::manage();
        }
    }
}

void generateWorld()
{
    glClearColor(39.0f / 256.0f, 25.0f / 256.0f, 17.0f / 256.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    nDrawText(13, 13, "Generating World...", {255, 50, 50, 50});
    nDrawText(10, 10, "Generating World...", {255, 255, 255, 255});
    SDL_GL_SwapWindow(window);

    if (!isMultiplayer)
    {
        chunk::manage();
    }
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--version")
        {
            std::cout << "OpenKraft 0.1 pre-1\nA primitive Minecraft clone written in C++ and OpenGL\nFor help, run with the --help argument\n\nCopyright (C) 2023 Mrmayman.\nLicence GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>. This is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\nWritten by Mrmayman\n";
            return 0;
        }
        else if (arg == "--help")
        {
            std::cout << "OpenKraft is a primitive Minecraft clone written in C++ and OpenGL\n\nOptions:\n  --help           Prints this help message\n  --version        Prints the program version and related information\n  --multiplayer    Runs the early multiplayer test. VERY BUGGY\n";
            return 0;
        }
        else if (arg == "--multiplayer")
        {
            isMultiplayer = 1;
        }
    }

    if (!initGame())
    {
        return 1;
    }

    generateWorld();

    std::thread manageThread(manageGame);

    // Spawn Player
    commonEntities[0] = new EntityPlayer();
    commonEntities[0]->y = 96;
    commonEntities[0]->x = 0;

    while (!quit)
    {
        handleEvents(quit);
        render();
        buildMeshes();
        nDrawText(13, 13,
                  std::to_string(int(1 / delta)) + " FPS\nx: " + std::to_string(cameraX) +
                      "\ny: " + std::to_string(cameraY) +
                      "\nz: " + std::to_string(cameraZ),
                  {255, 50, 50, 50});
        nDrawText(10, 10,
                  std::to_string(int(1 / delta)) + " FPS\nx: " + std::to_string(cameraX) +
                      "\ny: " + std::to_string(cameraY) +
                      "\nz: " + std::to_string(cameraZ),
                  {255, 255, 255, 255});
        SDL_GL_SwapWindow(window);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    manageThread.join();

    cleanup();
    return 0;
}
