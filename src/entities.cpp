#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#include "../include/entities.h"
#include "../include/facedraw.h"
#include "../include/chunk.h"
#include "../include/gameRenderer/main.h"

extern GameRenderer myRenderer;

extern float mouseX, mouseY, delta;

const Uint8 *keyboard_state;

Entity *entities[1024];
Entity *commonEntities[1024];

const bool collisionEnabled = 0;

void tickAndDrawEntities()
{
    for (int e = 0; e < 1024; e++)
    {
        if (entities[e] != nullptr)
        {
            entities[e]->tick();
        }
    }
    for (int e = 0; e < 1024; e++)
    {
        if (commonEntities[e] != nullptr)
        {
            commonEntities[e]->tick();
        }
    }
}

void Entity::buildBuffer()
{
    vertices.clear();
    indices.clear();

    // face code here
    face::front(
        0,
        0,
        0,
        6,
        vertices,
        indices,
        1, 1, 1, 1);
    face::back(
        0,
        0,
        0,
        6,
        vertices,
        indices,
        1, 1, 1, 1);
    face::top(
        0,
        0,
        0,
        6,
        vertices,
        indices,
        1, 1, 1, 1);
    face::bottom(
        0,
        0 + 1,
        0,
        6,
        vertices,
        indices,
        1, 1, 1, 1);
    face::left(
        0,
        0,
        0,
        6,
        vertices,
        indices,
        1, 1, 1, 1);
    face::right(
        0,
        0,
        0,
        6,
        vertices,
        indices,
        1, 1, 1, 1);
    // Generate and bind the VBO
    glGenBuffers(1, &entityVBO);
    glBindBuffer(GL_ARRAY_BUFFER, entityVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    // Generate and bind the IBO
    glGenBuffers(1, &entityIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entityIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    // Unbind the VBO and IBO after creating them
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Entity::draw()
{
    // Bind the VBO and IBO for drawing
    glBindBuffer(GL_ARRAY_BUFFER, entityVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entityIBO);

    GLint colorAttributeLocation = glGetAttribLocation(myRenderer.shaderProgram, "inColor");
    glVertexAttribPointer(colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(colorAttributeLocation);

    // Set up vertex and texture coordinate pointers
    glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), 0);
    glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));

    // Enable vertex color array
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, 9 * sizeof(GLfloat), (GLvoid *)(5 * sizeof(GLfloat)));

    // Bind the myRenderer.texAtlas texture before rendering
    glBindTexture(GL_TEXTURE_2D, myRenderer.texAtlas);

    glPushMatrix();
    glTranslatef(x, y, z);

    // Draw the entity using the IBO
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glPopMatrix();
    glDisableVertexAttribArray(colorAttributeLocation);
    // Unbind the VBO and IBO after drawing
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Entity::move()
{
    for (int i = 0; i < floor(abs(speedX * 1024.0f)); i++)
    {
        x += (60.0 / 1024.0) * delta * (abs(speedX) / speedX);
        if (collisionEnabled && colliding() > 0)
        {
            x -= (60.0 / 1024.0) * delta * (abs(speedX) / speedX);
            speedX = 0;
            break;
        }
    }
    float friction = 0.546 + ((!collisionEnabled) * 0.364);
    float multiplier = (1.0f / 3.0f);
    speedX *= 1.0 - (60.0 * multiplier * delta * (1.0 - friction));
    float jumpheight = 0.17;
    if (collisionEnabled)
    {
        if (colliding())
        {
            speedY = jumpheight * keyboard_state[SDL_SCANCODE_SPACE];
        }
    }
    else
    {
        speedY *= 1 - (60.0 * multiplier * delta * (1.0f - friction));
        if (keyboard_state[SDL_SCANCODE_SPACE])
        {
            speedY = 0.4;
        }
        else if (keyboard_state[SDL_SCANCODE_LSHIFT])
        {
            speedY = -0.4;
        }
    }
    for (int i = 0; i < floor(abs(speedY * 1024.0f)); i++)
    {
        y += (60.0 / 1024.0) * delta * (abs(speedY) / speedY);
        if (collisionEnabled && colliding())
        {
            y -= (60.0 / 1024.0) * delta * (abs(speedY) / speedY);
            speedY = jumpheight * keyboard_state[SDL_SCANCODE_SPACE];
            break;
        }
    }
    if (collisionEnabled)
    {
        speedY -= (36.0 / 60.0) * delta;
    }
    for (int i = 0; i < floor(abs(speedZ * 1024.0f)); i++)
    {
        z += (60.0 / 1024.0) * delta * (abs(speedZ) / speedZ);
        if (collisionEnabled && colliding() > 0)
        {
            z -= (60.0 / 1024.0) * delta * (abs(speedZ) / speedZ);
            speedZ = 0;
            break;
        }
    }
    speedZ *= 1 - (60.0 * multiplier * delta * (1.0f - friction));
}

void Entity::tick()
{
    draw();
}

bool Entity::colliding()
{
    try
    {
        Chunk &tempChunk = *chunkMap.at(ChunkCoordinate(floor(x / 32), floor(y / 32), floor(z / 32))).get();
        return blockConfig[tempChunk.blockData[(int64_t(floor(x)) % 32 + 32) % 32][(int64_t(floor(y)) % 32 + 32) % 32][(int64_t(floor(z)) % 32 + 32) % 32]].collisionType > 0;
    }
    catch (const std::out_of_range &e)
    {
        // std::cerr << "[error] Nonexistent chunk for collision : " << floor(x/32) << ", " << floor(y/32) << ", " << floor(z/32) << "\n";
    }
    return 0;
}

void EntityPlayer::tick()
{
    keyboard_state = SDL_GetKeyboardState(NULL);
    float moveSpeed = 7.0f * (0.098 + (0.0294 * keyboard_state[SDL_SCANCODE_LCTRL]) + ((!collisionEnabled) * 0.392));

    const float weirdDivideValue = 3.14/180.0;
    
    if (keyboard_state[SDL_SCANCODE_W])
    {
        speedZ -= moveSpeed * delta * cos(myRenderer.camera.rotX*weirdDivideValue);
        speedX += moveSpeed * delta * sin(myRenderer.camera.rotX*weirdDivideValue);
    }
    if (keyboard_state[SDL_SCANCODE_S])
    {
        speedZ += moveSpeed * delta * cos(myRenderer.camera.rotX*weirdDivideValue);
        speedX -= moveSpeed * delta * sin(myRenderer.camera.rotX*weirdDivideValue);
    }
    if (keyboard_state[SDL_SCANCODE_A])
    {
        speedZ -= moveSpeed * delta * sin(myRenderer.camera.rotX*weirdDivideValue);
        speedX -= moveSpeed * delta * cos(myRenderer.camera.rotX*weirdDivideValue);
    }
    if (keyboard_state[SDL_SCANCODE_D])
    {
        speedZ += moveSpeed * delta * sin(myRenderer.camera.rotX*weirdDivideValue);
        speedX += moveSpeed * delta * cos(myRenderer.camera.rotX*weirdDivideValue);
    }

    move();

    const float mouseSensitivity = 0.3;
    commonEntities[0]->rotationX += mouseX * mouseSensitivity * (delta * 60.0);
    commonEntities[0]->rotationY += mouseY * mouseSensitivity * (delta * 60.0);

    if (rotationY > 90.0)
    {
        rotationY = 90.0;
    }
    if (rotationY < -90.0)
    {
        rotationY = -90.0;
    }

    // Update Variables
    myRenderer.camera.X = x;
    myRenderer.camera.Y = y + sizeY;
    myRenderer.camera.Z = z;
    myRenderer.camera.rotX = rotationX;
    myRenderer.camera.rotY = rotationY;
}

void EntityPlayer::buildBuffer()
{
    vertices.clear();
    indices.clear();
}

void EntityPlayer::draw()
{
}