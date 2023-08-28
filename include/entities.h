#ifndef ENTITIES_H
#define ENTITIES_H

#include <vector>
// #include <cstddef>

class Entity
{
  public:
    float x = 0;
    float y = 0;
    float z = 0;

    float rotationX = 0;
    float rotationY = 0;

    float speedX = 0;
    float speedY = 0;
    float speedZ = 0;

    float sizeX = 0.5;
    float sizeY = 1;
    float sizeZ = 0.5;

    int id = 0;

    Entity() { buildBuffer(); }

    virtual void buildBuffer();
    virtual void draw();
    virtual void move();
    virtual void tick();
    virtual bool colliding();

    virtual ~Entity() {}

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    GLuint entityVBO, entityIBO;
};

class EntityPlayer : public Entity {
  public:
    EntityPlayer() {
        sizeX = 0.4;
        sizeY = 1.7;
        sizeZ = 0.4;
    }

    void buildBuffer() override;
    void draw() override;
    void tick() override;
};

void tickEntities();

#endif
