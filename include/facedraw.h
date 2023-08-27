#ifndef FACEDRAW_H
#define FACEDRAW_H

#include <vector>
#include <ostream>
#include <iostream>

extern GLuint texAtlas;
const int atlasSize = 16;

namespace face {

inline void top(float x, float y, float z, int textureIDtemp, std::vector<GLfloat> &verts, std::vector<GLuint> &inds, float colr, float colg, float colb, float cola)
{
    verts.reserve(36);
    inds.reserve(6);
    float tempTexX = float((textureIDtemp % atlasSize) - 1) / atlasSize;
    float tempTexY = float(textureIDtemp / atlasSize) / atlasSize;
    size_t currentVertSize = verts.size() / 9;
    // close right
    verts.insert(verts.end(), {x + 1.0f, y, z, tempTexX + 0.0f, tempTexY + 0.0f, colr, colg, colb, cola});
    // close left
    verts.insert(verts.end(), {x, y, z, tempTexX + (1.0f/atlasSize), tempTexY + 0.0f, colr, colg, colb, cola});
    // far left
    verts.insert(verts.end(), {x, y, z + 1.0f, tempTexX + (1.0f/atlasSize), tempTexY + (1.0f/atlasSize), colr, colg, colb, cola});
    // far right
    verts.insert(verts.end(), {x + 1.0f, y, z + 1.0f, tempTexX + 0.0f, tempTexY + (1.0f/atlasSize), colr, colg, colb, cola});

    // Triangle 1 (top-left, top-right, bottom-right)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize), static_cast<GLuint>(currentVertSize + 1), static_cast<GLuint>(currentVertSize + 2)});
    // Triangle 2 (bottom-right, bottom-left, top-left)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize + 2), static_cast<GLuint>(currentVertSize + 3), static_cast<GLuint>(currentVertSize)});
}

inline void bottom(float x, float y, float z, int textureIDtemp, std::vector<GLfloat> &verts, std::vector<GLuint> &inds, float colr, float colg, float colb, float cola)
{
    verts.reserve(36);
    inds.reserve(6);
    float tempTexX = float((textureIDtemp % atlasSize) - 1) / atlasSize;
    float tempTexY = float(textureIDtemp / atlasSize) / atlasSize;
    size_t currentVertSize = verts.size() / 9;
    // close right
    verts.insert(verts.end(), {x + 1.0f, y, z + 1.0f, tempTexX + 0.0f, tempTexY + 0.0f, colr, colg, colb, cola});
    // close left
    verts.insert(verts.end(), {x, y, z + 1.0f, tempTexX + (1.0f / atlasSize), tempTexY + 0.0f, colr, colg, colb, cola});
    // far left
    verts.insert(verts.end(), {x, y, z, tempTexX + (1.0f / atlasSize), tempTexY + (1.0f / atlasSize), colr, colg, colb, cola});
    // far right
    verts.insert(verts.end(), {x + 1.0f, y, z, tempTexX + 0.0f, tempTexY + (1.0f / atlasSize), colr, colg, colb, cola});

    // Triangle 1 (top-left, top-right, bottom-right)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize), static_cast<GLuint>(currentVertSize + 1), static_cast<GLuint>(currentVertSize + 2)});
    // Triangle 2 (bottom-right, bottom-left, top-left)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize + 2), static_cast<GLuint>(currentVertSize + 3), static_cast<GLuint>(currentVertSize)});
}

inline void front(float x, float y, float z, int textureIDtemp, std::vector<GLfloat> &verts, std::vector<GLuint> &inds, float colr, float colg, float colb, float cola)
{
    verts.reserve(36);
    inds.reserve(6);
    float tempTexX = float((textureIDtemp % atlasSize) - 1) / atlasSize;
    float tempTexY = float(textureIDtemp / atlasSize) / atlasSize;
    size_t currentVertSize = verts.size() / 9;
    // top-left
    verts.insert(verts.end(), {x + 1.0f, y + 1.0f, z, tempTexX + 0.0f, tempTexY + 0.0f, colr, colg, colb, cola});
    // top-right
    verts.insert(verts.end(), {x, y + 1.0f, z, tempTexX + (1.0f / atlasSize), tempTexY + 0.0f, colr, colg, colb, cola});
    // bottom-right
    verts.insert(verts.end(), {x, y, z, tempTexX + (1.0f / atlasSize), tempTexY + (1.0f / atlasSize), colr, colg, colb, cola});
    // bottom-left
    verts.insert(verts.end(), {x + 1.0f, y, z, tempTexX + 0.0f, tempTexY + (1.0f / atlasSize), colr, colg, colb, cola});

    // Triangle 1 (top-left, top-right, bottom-right)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize), static_cast<GLuint>(currentVertSize + 1), static_cast<GLuint>(currentVertSize + 2)});
    // Triangle 2 (bottom-right, bottom-left, top-left)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize + 2), static_cast<GLuint>(currentVertSize + 3), static_cast<GLuint>(currentVertSize)});
}

inline void back(float x, float y, float z, int textureIDtemp, std::vector<GLfloat> &verts, std::vector<GLuint> &inds, float colr, float colg, float colb, float cola)
{
    verts.reserve(36);
    inds.reserve(6);
    float tempTexX = float((textureIDtemp % atlasSize) - 1) / atlasSize;
    float tempTexY = float(textureIDtemp / atlasSize) / atlasSize;
    size_t currentVertSize = verts.size() / 9;
    // top-left
    verts.insert(verts.end(), {x, y + 1.0f, z + 1.0f, tempTexX + 0.0f, tempTexY + 0.0f, colr, colg, colb, cola});
    // top-right
    verts.insert(verts.end(), {x + 1.0f, y + 1.0f, z + 1.0f, tempTexX + (1.0f / atlasSize), tempTexY + 0.0f, colr, colg, colb, cola});
    // bottom-right
    verts.insert(verts.end(), {x + 1.0f, y, z + 1.0f, tempTexX + (1.0f / atlasSize), tempTexY + (1.0f / atlasSize), colr, colg, colb, cola});
    // bottom-left
    verts.insert(verts.end(), {x, y, z + 1.0f, tempTexX + 0.0f, tempTexY + (1.0f / atlasSize), colr, colg, colb, cola});

    // Triangle 1 (top-left, top-right, bottom-right)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize), static_cast<GLuint>(currentVertSize + 1), static_cast<GLuint>(currentVertSize + 2)});
    // Triangle 2 (bottom-right, bottom-left, top-left)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize + 2), static_cast<GLuint>(currentVertSize + 3), static_cast<GLuint>(currentVertSize)});
}

inline void left(float x, float y, float z, int textureIDtemp, std::vector<GLfloat> &verts, std::vector<GLuint> &inds, float colr, float colg, float colb, float cola)
{
    verts.reserve(36);
    inds.reserve(6);
    float tempTexX = float((textureIDtemp % atlasSize) - 1) / atlasSize;
    float tempTexY = float(textureIDtemp / atlasSize) / atlasSize;
    size_t currentVertSize = verts.size() / 9;
    // top-left
    verts.insert(verts.end(), {x, y + 1.0f, z, tempTexX + 0.0f, tempTexY + 0.0f, colr, colg, colb, cola});
    // top-right
    verts.insert(verts.end(), {x, y + 1.0f, z + 1.0f, tempTexX + (1.0f / atlasSize), tempTexY + 0.0f, colr, colg, colb, cola});
    // bottom-right
    verts.insert(verts.end(), {x, y, z + 1.0f, tempTexX + (1.0f / atlasSize), tempTexY + (1.0f / atlasSize), colr, colg, colb, cola});
    // bottom-left
    verts.insert(verts.end(), {x, y, z, tempTexX + 0.0f, tempTexY + (1.0f / atlasSize), colr, colg, colb, cola});

    // Triangle 1 (top-left, top-right, bottom-right)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize), static_cast<GLuint>(currentVertSize + 1), static_cast<GLuint>(currentVertSize + 2)});
    // Triangle 2 (bottom-right, bottom-left, top-left)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize + 2), static_cast<GLuint>(currentVertSize + 3), static_cast<GLuint>(currentVertSize)});
}

inline void right(float x, float y, float z, int textureIDtemp, std::vector<GLfloat> &verts, std::vector<GLuint> &inds, float colr, float colg, float colb, float cola)
{
    verts.reserve(36);
    inds.reserve(6);
    float tempTexX = float((textureIDtemp % atlasSize) - 1) / atlasSize;
    float tempTexY = float(textureIDtemp / atlasSize) / atlasSize;
    size_t currentVertSize = verts.size() / 9;
    // top-left
    verts.insert(verts.end(), {x + 1.0f, y + 1.0f, z + 1.0f,
        tempTexX + 0.0f, tempTexY + 0.0f,
        colr, colg, colb, cola});
    // top-right
    verts.insert(verts.end(), {x + 1.0f, y + 1.0f, z,
        tempTexX + (1.0f / atlasSize), tempTexY + 0.0f,
        colr, colg, colb, cola});
    // bottom-right
    verts.insert(verts.end(), {x + 1.0f, y, z,
        tempTexX + (1.0f / atlasSize), tempTexY + (1.0f / atlasSize),
        colr, colg, colb, cola});
    // bottom-left
    verts.insert(verts.end(), {x + 1.0f, y, z + 1.0f,
        tempTexX + 0.0f, tempTexY + (1.0f / atlasSize),
        colr, colg, colb, cola});

    // Triangle 1 (top-left, top-right, bottom-right)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize), static_cast<GLuint>(currentVertSize + 1), static_cast<GLuint>(currentVertSize + 2)});
    // Triangle 2 (bottom-right, bottom-left, top-left)
    inds.insert(inds.end(), {static_cast<GLuint>(currentVertSize + 2), static_cast<GLuint>(currentVertSize + 3), static_cast<GLuint>(currentVertSize)});
}


}

#endif
