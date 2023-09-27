#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <iostream>

#include "../include/gameRenderer/main.h"
#include "../include/gameRenderer/privateFunctions.h"
#include "../include/graphics.h"
#include "../include/renderer.h"
#include "../include/networking/networkingMain.h"

bool quit = false;

float mouseX = 0;
float mouseY = 0;

float delta = 1;

GameRenderer myRenderer;

void manageGame()
{
    // std::lock_guard<std::mutex> lock(chunkMapMutex);
    while (!quit)
    {
        if (myRenderer.isMultiplayer)
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
    clearScreen();
    drawText(13, 13, "Generating World...", {255, 50, 50, 50});
    drawText(10, 10, "Generating World...", {255, 255, 255, 255});
    finishRender();

    if (!myRenderer.isMultiplayer)
    {
        // chunk::manage();
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
            myRenderer.isMultiplayer = 1;
        }
    }

    if (!myRenderer.init())
    {
        return 1;
    }

    generateWorld();

    // Spawn Player
    commonEntities[0] = new EntityPlayer();
    commonEntities[0]->y = 96;
    commonEntities[0]->x = std::pow(2,1);

    myRenderer.camera.X = commonEntities[0]->x;
    myRenderer.camera.Y = commonEntities[0]->y;
    myRenderer.camera.Z = commonEntities[0]->z;

    std::thread manageThread(manageGame);

    while (!quit)
    {
        handleEvents(quit);
        render();
        buildMeshes();
    }

    manageThread.join();

    myRenderer.cleanup();
    return 0;
}
