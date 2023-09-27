#pragma once

#include <string>
#include <iostream>
#include <memory>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glu.h>

#include "entities.h"
#include "chunk.h"
#include "../include/gameRenderer/main.h"

int windowWidth = 800;
int windowHeight = 600;

bool isMouseLocked = false;

float aspectRatio = 800.0f / 600.0f;

extern float mouseX;
extern float mouseY;
extern bool isMouseLocked;
extern float delta;

extern Entity *entities[1024];
extern Entity *commonEntities[1024];

void handleEvents(bool &quit)
{
    mouseX = 0;
    mouseY = 0;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            quit = true;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                isMouseLocked = false;
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
            if (event.key.keysym.sym == SDLK_g)
            {
                int entityspawniter;
                for (entityspawniter = 0; entities[entityspawniter] != nullptr; entityspawniter++)
                {
                }
                std::cout << "[info] Spawned entity with index " << entityspawniter << "\n";
                entities[entityspawniter] = new Entity();
                entities[entityspawniter]->x = myRenderer.camera.X;
                entities[entityspawniter]->y = myRenderer.camera.Y;
                entities[entityspawniter]->z = myRenderer.camera.Y;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            if (!isMouseLocked)
            {
                isMouseLocked = true;
                SDL_SetRelativeMouseMode(SDL_TRUE);
                SDL_WarpMouseInWindow(myRenderer.window, 400, 300);
            }
        }
        else if (event.type == SDL_MOUSEMOTION && isMouseLocked)
        {
            mouseX = event.motion.xrel;
            mouseY = event.motion.yrel;
        }
        else if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                // Update window dimensions
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;
                // Update OpenGL viewport
                glViewport(0, 0, windowWidth, windowHeight);
                // Calculate new aspect ratio
                aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
            }
        }
    }
}

void buildMeshes()
{
    std::lock_guard<std::mutex> lock(vectorMutex);

    // Loop through all the chunk whose meshes need to be updated
    for (int i = (chunksToUpdate.size() - 1); i >= 0; i--)
    {
        bool shouldBeDeleted = 1;
        try
        {
            Chunk &myChunk = *chunkMap.at(chunksToUpdate[i]).get();
            if (myChunk.lock == 1)
            {
                std::cout << "Skipped mesh " << chunksToUpdate[i] << "\n";
                shouldBeDeleted = 0;
            }
            else
            {
                myChunk.destroyBuffers();
                myChunk.updateBuffers();
                myChunk.meshUpdated = 1;
            }
        }
        catch (const std::out_of_range &e)
        {
            ;
        }

        if (shouldBeDeleted)
        {
            chunksToUpdate.pop_back();
        }
    }
}