#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glu.h>

#include "main.h"
#include "../chunk.h"

void GameRenderer::buildShader()
{
    const char *vertexShaderSource = R"(
#version 120
// Vertex Shader

varying float distanceToCamera;
attribute vec4 inColor; // Vertex color attribute
varying vec4 fragColor; // Varying color for the fragment shader
varying vec2 texCoord;  // Added texture coordinate variable

void main() {
    //inColor = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 vertexPosition = gl_ModelViewMatrix * gl_Vertex;
    vec3 cameraPosition = vec3(gl_ModelViewMatrixInverse[3]);
    // Calculate the squared distance using the Pythagorean theorem
    vec3 difference = vertexPosition.xyz;
    float distanceSquared = dot(difference, difference);
    distanceToCamera = sqrt(distanceSquared);

    texCoord = gl_MultiTexCoord0.xy;

    fragColor = inColor; // Pass the vertex color to the fragment shader

    gl_Position = gl_ProjectionMatrix * vertexPosition;
}
    )";

    const char *fragmentShaderSource = R"(
#version 120
// Fragment Shader
varying vec4 fragColor; // Received interpolated color from the vertex shader
varying float distanceToCamera;
varying vec2 texCoord;  // Varying variable for texture coordinates
uniform int renderDistanceUniform;
uniform sampler2D textureSampler; // Uniform for the texture sampler

void main() {
    // Define fog parameters
    float fogStart = float(renderDistanceUniform) * 16.0; // Adjust as needed
    float fogEnd = float(renderDistanceUniform) * 33.0;   // Adjust as needed
    vec3 fogColor = vec3(0.671875, 0.7890625, 1.0); // Adjust fog color

    // Calculate fog factor based on distance
    float fogFactor = (distanceToCamera - fogStart) / (fogEnd - fogStart);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Sample the texture using the texture coordinates
    vec4 textureColor = texture2D(textureSampler, texCoord);

    // Interpolate between fragment color and fog color using fog factor
    vec3 fragmentColor = textureColor.rgb * fragColor.rgb;

    // Apply alpha blending based on the texture's alpha channel
    vec3 blendedColor = mix(fragmentColor, fogColor, fogFactor);
    float alpha = textureColor.a;

    gl_FragColor = vec4(blendedColor, alpha);
}
)";

    GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    shaderProgram = glCreateProgram();
    GLint renderDistanceUniform = glGetUniformLocation(shaderProgram, "renderDistanceUniform");
    glUniform1i(renderDistanceUniform, chunk::renderDistance);
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
}

bool GameRenderer::initMultiplayer()
{
    extern int mySocket;
    if (myRenderer.isMultiplayer)
    {
        mySocket = net::init();
        if (net::connect("127.0.0.1", 25565, mySocket) != 0)
        {
            std::cerr << "[info] Shutting down - Fatal Error\n";
            return false;
        }
        std::vector<uint8_t> handshakeData = {0x02};
        net::appendString16("mrmayman", handshakeData);

        net::sendpacket(handshakeData, mySocket);
    }
    return true;
}

bool GameRenderer::initOpenGL()
{
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // Enable backface culling
    // glEnable(GL_CULL_FACE);

    // Set up the 3D cube and camera transformations
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Enable vertex attribute arrays (position and texture coordinates)
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Set the culling mode to cull back-facing triangles
    // glCullFace(GL_BACK);

    // Set the front face to be defined by vertices in counter-clockwise order
    glFrontFace(GL_CW);
    gluPerspective(camera.FOV, (GLfloat)800 / (GLfloat)600, 0.1f, (chunk::renderDistance * 32) + 32);

    return true;
}

bool GameRenderer::initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow("OpenKraft Alpha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_RENDERER_PRESENTVSYNC);
    if (window == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    context = SDL_GL_CreateContext(window);
    if (context == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // Initialize GLEW after creating the OpenGL context
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK)
    {
        // GLEW initialization failed
        // Handle the error here
        std::cerr << "GLEW could not be initialized!\n";
        return false;
    }

    char *buf = SDL_GetBasePath();
    GamePath = buf;
    GamePath.append("Assets/");
    SDL_free(buf);

    return true;
}

bool GameRenderer::initFont()
{
    if (TTF_Init() == -1)
    {
        std::cerr << "Error - Failed to start up SDL_ttf font renderer\n";
        return false;
    }

    font = TTF_OpenFont((GamePath + std::string("fonts/minecraft.otf")).c_str(), 28);
    if (font == nullptr)
    {
        std::cerr << "Error loading font: " << TTF_GetError() << "\n";
        return false;
    }
    return true;
}

GLuint GameRenderer::LoadShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check compilation status
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<GLchar> errorLog(logLength);
        glGetShaderInfoLog(shader, logLength, NULL, &errorLog[0]);

        std::cerr << "Shader compilation error:\n"
                  << &errorLog[0] << "\n";

        // Handle the error, perhaps by cleaning up and returning an error code

        // Delete the shader
        glDeleteShader(shader);

        return 0; // Or some error code
    }

    return shader;
}

void GameRenderer::drawText(int x, int y, const std::string &text, SDL_Color color)
{
    SDL_Surface *tempSurface = TTF_RenderText_Solid_Wrapped(font, text.c_str(), color, 800);
    if (!tempSurface)
    {
        // Error handling if the text rendering failed
        std::cerr << "Failed to render text: " << TTF_GetError() << std::endl;
        return;
    }

    // SDL_SetSurfaceBlendMode(tempSurface, SDL_BLENDMODE_NONE);
    SDL_Surface *textSurface = SDL_ConvertSurfaceFormat(tempSurface, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(tempSurface);

    GLuint textTextureID;
    glGenTextures(1, &textTextureID);
    glBindTexture(GL_TEXTURE_2D, textTextureID);
    // std::cout << textSurface->w << ", " << textSurface->h << "\n";
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

    extern int windowWidth;
    extern int windowHeight;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // std::cout << windowWidth << "\n";

    // Draw the text using OpenGL quads
    glColor3f(1.0f, 1.0f, 1.0f); // Set text color
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(x, y);
    glTexCoord2f(1, 0);
    glVertex2f((x + textSurface->w), y);
    glTexCoord2f(1, 1);
    glVertex2f((x + textSurface->w), y + textSurface->h);
    glTexCoord2f(0, 1);
    glVertex2f(x, y + textSurface->h);
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

GLuint GameRenderer::loadTexture(const std::string &filePath)
{
    // Load the image using SDL_image
    SDL_Surface *surface = IMG_Load((GamePath + filePath).c_str());
    if (!surface)
    {
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