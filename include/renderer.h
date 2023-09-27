#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glu.h>

struct Color
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
};

void clearScreen()
{
    glClearColor((172.0f / 256.0f), (202.0f / 256.0f), (256.0f / 256.0f), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void setupShaders()
{
    glUseProgram(myRenderer.shaderProgram);

    GLint location = glGetUniformLocation(myRenderer.shaderProgram, "renderDistanceUniform");
    glUniform1i(location, chunk::renderDistance);
}

static void setupProjection()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(myRenderer.camera.FOV, aspectRatio, 0.1f, (chunk::renderDistance * 32) + 32);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Enable Z-clipping by setting the depth function
    glDepthFunc(GL_LEQUAL);
}

static void renderChunks()
{
    std::lock_guard<std::mutex> lock(chunkMapMutex);
    for (std::pair<const ChunkCoordinate, std::shared_ptr<Chunk>> &chunk : chunkMap)
    {
        if (abs((chunk.first.x * 32) - myRenderer.camera.X) / 32 > chunk::renderDistance ||
            abs((chunk.first.y * 32) - myRenderer.camera.Y) / 32 > chunk::renderDistance ||
            abs((chunk.first.z * 32) - myRenderer.camera.Z) / 32 > chunk::renderDistance)
        {
            // Chunk is outside render distance
        }
        else if (chunk.second->lock)
        {
            // Chunk is being generated/updated. Skip rendering
        }
        else
        {
            chunk.second->draw();
        }
    }
}

void drawText(int x, int y, const std::string &text, Color textColor)
{
    // TODO
}

static void drawDebugText()
{
    myRenderer.drawText(13, 13,
                        std::to_string(int(1 / delta)) + " FPS\nx: " + std::to_string(myRenderer.camera.X) +
                            "\ny: " + std::to_string(myRenderer.camera.Y) +
                            "\nz: " + std::to_string(myRenderer.camera.Z) +
                            "\n\nplayerX: " + std::to_string(commonEntities[0]->x) +
                            "\nplayerY: " + std::to_string(commonEntities[0]->y) +
                            "\nplayerZ: " + std::to_string(commonEntities[0]->z),
                        {255, 50, 50, 50});
    myRenderer.drawText(10, 10,
                        std::to_string(int(1 / delta)) + " FPS\nx: " + std::to_string(myRenderer.camera.X) +
                            "\ny: " + std::to_string(myRenderer.camera.Y) +
                            "\nz: " + std::to_string(myRenderer.camera.Z) +
                            "\n\nplayerX: " + std::to_string(commonEntities[0]->x) +
                            "\nplayerY: " + std::to_string(commonEntities[0]->y) +
                            "\nplayerZ: " + std::to_string(commonEntities[0]->z),
                        {255, 255, 255, 255});
}

void finishRender()
{
    glUseProgram(0);
    SDL_GL_SwapWindow(myRenderer.window);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void render()
{
    static Uint32 prevTime = SDL_GetTicks();    // Variable to store the previous frame time
    Uint32 currentTime = SDL_GetTicks();        // Current frame time
    delta = (currentTime - prevTime) / 1000.0f; // Delta time in seconds

    clearScreen();
    setupShaders();
    setupProjection();

    renderChunks();
    drawDebugText();

    tickAndDrawEntities();

    finishRender();

    prevTime = currentTime; // Update previous frame time
}