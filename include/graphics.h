#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <string>
#include <iostream>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glu.h>

#include "entities.h"
#include "chunk.h"

/*GLuint SkyVBO, SkyIBO;
std::vector<GLfloat> SkyVertices;
std::vector<GLuint> SkyIndices;*/
GLuint shaderProgram;

TTF_Font* font;
std::string GamePath;
float cameraFOV = 90;

int windowWidth = 800;
int windowHeight = 600;

bool isMouseLocked = false;

SDL_Window* window;
SDL_GLContext context;

float aspectRatio = 800.0f/600.0f;

extern float mouseX;
extern float mouseY;
extern bool isMouseLocked;
extern float cameraX, cameraY, cameraZ, cameraFOV;
extern float delta, rotX, rotY;

extern Entity *entities[1024];
extern Entity *commonEntities[1024];

GLuint LoadShader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check compilation status
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<GLchar> errorLog(logLength);
        glGetShaderInfoLog(shader, logLength, NULL, &errorLog[0]);

        std::cerr << "Shader compilation error:\n" << &errorLog[0] << "\n";

        // Handle the error, perhaps by cleaning up and returning an error code

        // Delete the shader
        glDeleteShader(shader);

        return 0; // Or some error code
    }

    return shader;
}

void nDrawText(int x, int y, std::string text, SDL_Color color) {
    SDL_Surface* tempSurface = TTF_RenderText_Solid_Wrapped(font, text.c_str(), color, 800);
    if (!tempSurface) {
        // Error handling if the text rendering failed
        std::cerr << "Failed to render text: " << TTF_GetError() << std::endl;
        return;
    }

    //SDL_SetSurfaceBlendMode(tempSurface, SDL_BLENDMODE_NONE);
    SDL_Surface* textSurface = SDL_ConvertSurfaceFormat(tempSurface, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(tempSurface);

    GLuint textTextureID;
    glGenTextures(1, &textTextureID);
    glBindTexture(GL_TEXTURE_2D, textTextureID);
    //std::cout << textSurface->w << ", " << textSurface->h << "\n";
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textSurface->w, textSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, textSurface->pixels);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Enable alpha blending for the text
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    //std::cout << windowWidth << "\n";

    // Draw the text using OpenGL quads
    glColor3f(1.0f, 1.0f, 1.0f); // Set text color
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(x, y);
    glTexCoord2f(1, 0); glVertex2f((x + textSurface->w), y);
    glTexCoord2f(1, 1); glVertex2f((x + textSurface->w), y + textSurface->h);
    glTexCoord2f(0, 1); glVertex2f(x, y + textSurface->h);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Clean up the texture and surface
    glDeleteTextures(1, &textTextureID);
    SDL_FreeSurface(textSurface);
}

GLuint loadTexture(const std::string& filePath) {
    // Load the image using SDL_image
    SDL_Surface* surface = IMG_Load((GamePath + filePath).c_str());
    if (!surface) {
        // Handle image loading error
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load image: %s\n", IMG_GetError());
        return 0; // Return an invalid texture ID
    }

    // Generate a new OpenGL texture ID
    GLuint textureID;
    glGenTextures(1, &textureID);

    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Load the image data into the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

    // Free the surface
    SDL_FreeSurface(surface);
    return textureID;
}

void handleEvents(bool& quit)
{
    mouseX = 0;
    mouseY = 0;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            { quit = true; }
        else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                isMouseLocked = false;
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
            if (event.key.keysym.sym == SDLK_g) {
                int entityspawniter;
                for(entityspawniter = 0; entities[entityspawniter] != nullptr; entityspawniter++) {
                }
                std::cout << "[info] Spawned entity with index " << entityspawniter << "\n";
                entities[entityspawniter] = new Entity();
                entities[entityspawniter]->x = cameraX;
                entities[entityspawniter]->y = cameraY;
                entities[entityspawniter]->z = cameraZ;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (!isMouseLocked) {
                isMouseLocked = true;
                SDL_SetRelativeMouseMode(SDL_TRUE);
                SDL_WarpMouseInWindow(window, 400, 300);
            }
        }
        else if (event.type == SDL_MOUSEMOTION && isMouseLocked) {
            mouseX = event.motion.xrel;
            mouseY = event.motion.yrel;
        }
        else if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
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

void render()
{
    static Uint32 prevTime = SDL_GetTicks();  // Variable to store the previous frame time
    Uint32 currentTime = SDL_GetTicks();      // Current frame time
    delta = (currentTime - prevTime) / 1000.0f;  // Delta time in seconds

    //glClearColor(56.0f/256.0f, 62.0f/256.0f, 189.0f/256.0f, 1.0f);
    glClearColor((172.0f / 256.0f), (202.0f / 256.0f), (256.0f / 256.0f), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    GLint location = glGetUniformLocation(shaderProgram, "renderDistanceUniform");
    glUniform1i(location, chunk::renderDistance);

    glMatrixMode(GL_PROJECTION);
    //std::cout << aspectRatio << "\n";
    glLoadIdentity();
    gluPerspective(cameraFOV, aspectRatio, 0.1f, (chunk::renderDistance * 32) + 32);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float cachecosy = cos(rotY);
    float lookX = cameraX + (sin(rotX) * cachecosy);
    float lookY = cameraY - (sin(rotY) * 1.5);
    float lookZ = cameraZ - (cos(rotX) * cachecosy);

    gluLookAt(cameraX, cameraY, cameraZ,  // Camera position
              lookX, lookY, lookZ,        // Look-at point
              0.0f, 1.0f, 0.0f);          // Up vector

    // Enable Z-clipping by setting the depth function
    glDepthFunc(GL_LEQUAL);

    // Bind the VBO and IBO for drawing
    /*glBindBuffer(GL_ARRAY_BUFFER, SkyVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SkyIBO);

    // Set up vertex and texture coordinate pointers
    glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), 0);
    glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

    glBindTexture(GL_TEXTURE_2D, texAtlas);

    //GLint colorAttributeLocation = glGetAttribLocation(shaderProgram, "inColor");
    //glVertexAttribPointer(colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
    //glEnableVertexAttribArray(colorAttributeLocation);

    glPushMatrix();
    glTranslatef(cameraX,cameraY,cameraZ);
    glScalef((chunk::renderDistance * 96), (chunk::renderDistance * 10), (chunk::renderDistance * 96));
    // Draw the entity using the IBO
    glDrawElements(GL_TRIANGLES, SkyIndices.size(), GL_UNSIGNED_INT, 0);

    glPopMatrix();
    //glDisableVertexAttribArray(colorAttributeLocation);
    // Unbind the VBO and IBO after drawing
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);*/

    std::lock_guard<std::mutex> lock(chunkMapMutex);
    for (std::pair<const ChunkCoordinate, Chunk>& i : chunkMap) {
        if(abs((i.first.x * 32) - cameraX)/32 > chunk::renderDistance ||
        abs((i.first.y * 32) - cameraY)/32 > chunk::renderDistance ||
        abs((i.first.z * 32) - cameraZ)/32 > chunk::renderDistance) {
            //std::cerr << "[error] Invalid Chunk " << i.first << "\n";
        } else if(i.second.lock) {
            //std::cout << "[info] Skipped chunk " << i.first << "\n";
        } else {
            i.second.draw();
        }
    }
    tickEntities();

    // Disable fog after rendering
    //glDisable(GL_FOG);
    glUseProgram(0);

    prevTime = currentTime;  // Update previous frame time
}

void buildMeshes()
{
    std::lock_guard<std::mutex> lock(vectorMutex);

    for (int i = (chunksToUpdate.size() - 1); i >= 0; i--) {
        bool shouldBeDeleted = 1;
        try {
            Chunk& myChunk = chunkMap.at(chunksToUpdate[i]);
            if(myChunk.lock == 1) {
                shouldBeDeleted = 1;
            } else {
                myChunk.destroyBuffers();
                glGenBuffers(1, &myChunk.squareVBO);
                glBindBuffer(GL_ARRAY_BUFFER, myChunk.squareVBO);
                glBufferData(GL_ARRAY_BUFFER, myChunk.vertices.size() * sizeof(GLfloat), &myChunk.vertices[0], GL_STATIC_DRAW);

                glGenBuffers(1, &myChunk.squareIBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myChunk.squareIBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, myChunk.indices.size() * sizeof(GLuint), &myChunk.indices[0], GL_STATIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                myChunk.meshUpdated = 1;
            }
        } catch(const std::out_of_range& e) {
        ;
        }

        if(shouldBeDeleted) { chunksToUpdate.pop_back(); }
    }
}

#endif