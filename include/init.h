#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#include "netcode.h"

extern bool isMultiplayer;
extern int mySocket;

bool initMultiplayer()
{
    if (isMultiplayer)
    {
        mySocket = cooc::init();
        if (cooc::connect("127.0.0.1", 25565, mySocket) != 0)
        {
            std::cerr << "[info] Shutting down - Fatal Error\n";
            return 1;
        }
        std::vector<uint8_t> handshakeData = {0x02};
        cooc::appendString16("mrmayman", handshakeData);

        cooc::sendpacket(handshakeData, mySocket);
    }
}

bool initOpenGL()
{
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // Enable backface culling
    //glEnable(GL_CULL_FACE);

    // Set up the 3D cube and camera transformations
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Enable vertex attribute arrays (position and texture coordinates)
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Set the culling mode to cull back-facing triangles
    //glCullFace(GL_BACK);

    // Set the front face to be defined by vertices in counter-clockwise order
    glFrontFace(GL_CW);
    gluPerspective(cameraFOV, (GLfloat)800 / (GLfloat)600, 0.1f, (chunk::renderDistance * 32) + 32);
}

bool initGame()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow("OpenKraft Alpha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_RENDERER_PRESENTVSYNC );
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // Initialize GLEW after creating the OpenGL context
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        // GLEW initialization failed
        // Handle the error here
        std::cerr << "GLEW could not be initialized!\n";
        return false;
    }

    char *buf = SDL_GetBasePath();
    GamePath = buf;
    GamePath.append("Assets/");
    SDL_free(buf);

    if(TTF_Init() == -1) {
        std::cerr << "Error - Failed to start up SDL_ttf font renderer\n";
        return false;
    }

    font = TTF_OpenFont((GamePath + std::string("fonts/minecraft.otf")).c_str(), 28);
    if(font == nullptr) {
        std::cerr << "Error loading font: " << TTF_GetError() << "\n";
    }

    if(!initMultiplayer())
    {
        return false;
    }

    return true;
}