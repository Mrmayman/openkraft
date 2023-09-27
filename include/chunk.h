#pragma once

#include <GL/glew.h>
#include <GL/glu.h>

#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <mutex>
// #include <random>
#include <memory>

#include "../lib/FastNoiseLite.h"

#include "../include/facedraw.h"
#include "../include/blocktexdef.h"

extern std::mutex chunkMapMutex;

extern int seed;
extern FastNoiseLite terrainNoise;

bool isArrayFilledWithZeroes(const int16_t (&array)[32][32][32]);

using std::int64_t;

class ChunkCoordinate {
  public:
    ChunkCoordinate(int64_t inx, int64_t iny, int64_t inz) : x(inx), y(iny), z(inz) {}
    ChunkCoordinate() : x(0), y(0), z(0) {}

    int64_t x = 0;
    int64_t y = 0;
    int64_t z = 0;

    bool operator==(const ChunkCoordinate& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const ChunkCoordinate& other) const {
        return !(x == other.x && y == other.y && z == other.z);
    }

    friend std::ostream& operator<<(std::ostream& os, const ChunkCoordinate& coord) {
        os << "(" << coord.x << ", " << coord.y << ", " << coord.z << ")";
        return os;
    }
};

class Chunk {
  public:
    int16_t blockData[32][32][32];

    int64_t x = 0;
    int64_t y = 0;
    int64_t z = 0;

    bool lock = 0;
    bool meshUpdated = 0;

    void gen();
    void fill(int16_t block);
    void draw();
    void updateMesh();
    void destroyBuffers();
    void updateBuffers();

    Chunk() {
        fill(0);
        //std::cout << "Constructed\n";
        lock = 1;
        vertices.clear();
        indices.clear();
    }

    Chunk(int64_t inx, int64_t iny, int64_t inz) : x(inx), y(iny), z(inz)
    {
        gen();
        updateMesh();
    }

    Chunk(int64_t inx, int64_t iny, int64_t inz, int16_t tile) : x(inx), y(iny), z(inz)
    {
        fill(tile);
        updateMesh();
    }

    ~Chunk()
    {
        destroyBuffers();
    }

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    GLuint VBO, IBO;
  private:
    int indicesNum = 0;
};

struct ChunkCoordinateHash {
    std::size_t operator()(const ChunkCoordinate& coordinate) const {
        // Combine the hash of x, y, and z using bitwise XOR
        std::size_t hash = 0;
        hash ^= std::hash<int64_t>{}(coordinate.x);
        hash ^= std::hash<int64_t>{}(coordinate.y);
        hash ^= std::hash<int64_t>{}(coordinate.z);
        return hash;
    }
};

extern std::unordered_map<ChunkCoordinate, std::shared_ptr<Chunk>, ChunkCoordinateHash> chunkMap;

namespace chunk {

extern int renderDistance;

inline const Chunk& read(int64_t x, int64_t y, int64_t z);
int16_t getTile(int64_t x, int64_t y, int64_t z);
int16_t getTile(const float x, const float y, const float z);
void setTile(int64_t getx, int64_t gety, int64_t getz, int16_t tile);
void write(int64_t x, int64_t y, int64_t z, const Chunk& chunk);
void write(const Chunk& chunk);
void remove(int64_t x, int64_t y, int64_t z);
void create(int64_t x, int64_t y, int64_t z);
void loadChunk(int64_t x, int64_t y, int64_t z);
void updateNeighbours(int64_t x, int64_t y, int64_t z);
void manage();
void unload();

}

extern std::mutex vectorMutex;
extern std::vector<ChunkCoordinate> chunksToUpdate;