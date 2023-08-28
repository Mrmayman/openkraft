#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glu.h>

#include "networkingFunctions.h"
#include "graphics.h"

extern bool isMultiplayer;
extern int mySocket;

void cleanup()
{
    if (isMultiplayer)
    {
        std::vector<uint8_t> quitData = {0xff};
        net::appendString16("Quitting", quitData);
    }

    for (int i = 0; i < int(sizeof(entities) / (sizeof(Entity))); i++) {
        delete entities[i];
    }
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void buildShader()
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

    /*const float SkyColorR = 133.0f / 256.0f;
    const float SkyColorG = 174.0f / 256.0f;
    const float SkyColorB = 256.0f / 256.0f;
    const float SkyColorA = 1.0f;

    const float VoidColorR = 56.0f / 256.0f;
    const float VoidColorG = 62.0f / 256.0f;
    const float VoidColorB = 189.0f / 256.0f;
    const float VoidColorA = 1.0f;

    face::top(-0.5f,0.5f,-0.5f,0,SkyVertices,SkyIndices,SkyColorR,SkyColorG,SkyColorB,SkyColorA);
    face::bottom(-0.5f,-0.5f,-0.5f,0,SkyVertices,SkyIndices,VoidColorR,VoidColorG,VoidColorB,VoidColorA);
    glGenBuffers(1, &SkyVBO);
    glBindBuffer(GL_ARRAY_BUFFER, SkyVBO);
    glBufferData(GL_ARRAY_BUFFER, SkyVertices.size() * sizeof(GLfloat), &SkyVertices[0], GL_STATIC_DRAW);
    // Generate and bind the IBO
    glGenBuffers(1, &SkyIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SkyIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, SkyIndices.size() * sizeof(GLuint), &SkyIndices[0], GL_STATIC_DRAW);
    // Unbind the VBO and IBO after creating them
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);*/
}

bool initMultiplayer()
{
    if (isMultiplayer)
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

    return true;
}

bool initSDL()
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

    return true;
}

bool initFont()
{
    if(TTF_Init() == -1) {
        std::cerr << "Error - Failed to start up SDL_ttf font renderer\n";
        return false;
    }

    font = TTF_OpenFont((GamePath + std::string("fonts/minecraft.otf")).c_str(), 28);
    if(font == nullptr) {
        std::cerr << "Error loading font: " << TTF_GetError() << "\n";
        return false;
    }
    return true;
}

bool initGame()
{
    if(!(initSDL() && initOpenGL() && initFont() && initMultiplayer()))
    {
        return false;
    }

    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetSeed(seed);
    texAtlas = loadTexture("terrain.png");

    buildShader();

    return true;
}