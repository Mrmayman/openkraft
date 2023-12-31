#include <GL/glew.h>
#include <GL/glu.h>

#include <cmath>

#include "../lib/FastNoiseLite.h"

#include "../include/facedraw.h"
#include "../include/blocktexdef.h"
#include "../include/chunk.h"
#include "../include/gameRenderer/main.h"

extern GameRenderer myRenderer;

std::mutex chunkMapMutex;
int chunk::renderDistance = 4;

std::unordered_map<ChunkCoordinate, std::shared_ptr<Chunk>, ChunkCoordinateHash> chunkMap;

std::mutex vectorMutex;
std::vector<ChunkCoordinate> chunksToUpdate;

extern bool quit;
extern float mouseX, mouseY;

bool isArrayFilledWithZeroes(const int16_t (&array)[32][32][32])
{
    for (int i = 0; i < 32; ++i)
    {
        for (int j = 0; j < 32; ++j)
        {
            for (int k = 0; k < 32; ++k)
            {
                if (array[i][j][k] != 0)
                {
                    return false; // If any element is not zero, return false.
                }
            }
        }
    }
    return true; // All elements are zero, return true.
}

using std::int64_t;

void Chunk::gen()
{
    lock = 1;
    const int genRes = 8;
    const int genResInv = 32.0f / float(genRes);
    std::cout << "[info] Generating chunk " << ChunkCoordinate(x, y, z) << "\n";
    float noises[genRes + 2][genRes + 2][genRes + 2];
    float noiseVal = 0;

    double currentX = 0;
    double currentY = 0;
    double currentZ = 0;
    for (int i = -1; i < genRes + 1; i++)
    {
        currentX = double((x * 32) + (i * genResInv));
        for (int j = -1; j < genRes + 1; j++)
        {
            currentY = double((y * 32) + (j * genResInv)) / 1.5f;
            for (int k = -1; k < genRes + 1; k++)
            {
                currentZ = double((z * 32) + (k * genResInv));
                noiseVal = 0;
                noiseVal += myRenderer.terrainNoise.GetNoise(
                                currentX * 1.0f,
                                currentY * 1.0f,
                                currentZ * 1.0f) *
                            1.0f;
                noiseVal += myRenderer.terrainNoise.GetNoise(
                                currentX * 2.0f,
                                currentY * 2.0f,
                                currentZ * 2.0f) *
                            0.5f;
                noiseVal += myRenderer.terrainNoise.GetNoise(
                                currentX * 4.0f,
                                currentY * 4.0f,
                                currentZ * 4.0f) *
                            0.25f;
                noiseVal += myRenderer.terrainNoise.GetNoise(
                                currentX * 8.0f,
                                currentY * 8.0f,
                                currentZ * 8.0f) *
                            0.2f;
                if ((y * 32) + (j * genResInv) > 64)
                {
                    noiseVal += float(64 - ((y * 32) + (j * genResInv))) / 112.0f;
                }
                else
                {
                    noiseVal += float(64 - ((y * 32) + (j * genResInv))) / 48.0f;
                }

                if (noiseVal > 1.0)
                {
                    noiseVal = 1.0;
                }
                noises[i + 1][j + 1][k + 1] = noiseVal;
            }
        }
    }
    // Precompute the inverse of 4.0f outside the loop
    const float inv4 = 1.0f / float(genResInv);

    for (int i = 0; i < 32; i++)
    {
        int xIdx0 = (i / genResInv) + 1;
        int xIdx1 = xIdx0 + 1;
        float xFrac = static_cast<float>(i % genResInv) * inv4;

        for (int j = 0; j < 32; j++)
        {
            int yIdx0 = (j / genResInv) + 1;
            int yIdx1 = yIdx0 + 1;
            float yFrac = static_cast<float>(j % genResInv) * inv4;

            for (int k = 0; k < 32; k++)
            {
                int zIdx0 = (k / genResInv) + 1;
                int zIdx1 = zIdx0 + 1;
                float zFrac = static_cast<float>(k % genResInv) * inv4;

                // Interpolate along each dimension
                float noiseVal0 = noises[xIdx0][yIdx0][zIdx0] * (1.0f - xFrac) + noises[xIdx1][yIdx0][zIdx0] * xFrac;
                float noiseVal1 = noises[xIdx0][yIdx1][zIdx0] * (1.0f - xFrac) + noises[xIdx1][yIdx1][zIdx0] * xFrac;
                float noiseVal2 = noises[xIdx0][yIdx0][zIdx1] * (1.0f - xFrac) + noises[xIdx1][yIdx0][zIdx1] * xFrac;
                float noiseVal3 = noises[xIdx0][yIdx1][zIdx1] * (1.0f - xFrac) + noises[xIdx1][yIdx1][zIdx1] * xFrac;

                // Interpolate along the y dimension
                float noiseVal01 = noiseVal0 * (1.0f - yFrac) + noiseVal1 * yFrac;
                float noiseVal23 = noiseVal2 * (1.0f - yFrac) + noiseVal3 * yFrac;

                // Final interpolation along the z dimension
                float interpolatedValue = noiseVal01 * (1.0f - zFrac) + noiseVal23 * zFrac;

                /*const float caveNoiseScale = 4.0;

                float caveNoiseVal = myRenderer.caveNoise.GetNoise(caveNoiseScale * float((x * 32) + i),
                                                                   caveNoiseScale * float((y * 32) + j),
                                                                   caveNoiseScale * float((z * 32) + k));
                caveNoiseVal = (caveNoiseVal + 1.0) / 2.0;

                interpolatedValue -= caveNoiseVal;

                std::cout << caveNoiseVal << "/" << interpolatedValue << "\n";*/

                // Set block data based on interpolated noise value
                if (interpolatedValue > 0.0)
                {
                    blockData[i][j][k] = 1;
                }
                else
                {
                    blockData[i][j][k] = 0;
                }
            }
        }
    }
}

void Chunk::fill(int16_t block)
{
    lock = 1;
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            for (int k = 0; k < 32; k++)
            {
                blockData[i][j][k] = block;
            }
        }
    }
}

void Chunk::draw()
{
    //    std::cout << meshUpdated << "\n";
    if ((!meshUpdated) || lock == 1 || indicesNum == 0)
    {
        return;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    GLint colorAttributeLocation = glGetAttribLocation(myRenderer.shaderProgram, "inColor");
    glVertexAttribPointer(colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(colorAttributeLocation);

    // Set up vertex and texture coordinate pointers
    glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), 0);
    glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));

    // Bind the myRenderer.texAtlas texture before rendering
    glBindTexture(GL_TEXTURE_2D, myRenderer.texAtlas);

    // std::cout << ChunkCoordinate(x,y,z) << "\n";
    glPushMatrix();
    glRotatef(myRenderer.camera.rotY, 1.0f, 0.0f, 0.0f); // Rotate the scene to match the camera's pitch (vertical rotation)
    glRotatef(myRenderer.camera.rotX, 0.0f, 1.0f, 0.0f); // Rotate the scene to match the camera's yaw (horizontal rotation)
    glTranslatef(double(x * 32.0d) - myRenderer.camera.X, double(y * 32.0d) - myRenderer.camera.Y, double(z * 32.0d) - myRenderer.camera.Z);
    glDrawElements(GL_TRIANGLES, indicesNum, GL_UNSIGNED_INT, 0);
    glPopMatrix();

    glDisableVertexAttribArray(colorAttributeLocation);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        const char *errorStr = reinterpret_cast<const char *>(gluErrorString(error));
        printf("OpenGL rendering error: %s\n", errorStr);
    }

    // Unbind the VBO and IBO for the red square
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Chunk &getEmptyChunk()
{
    static Chunk emptyChunk; // Static local variable for the empty chunk
    emptyChunk.fill(0);
    emptyChunk.lock = 0;
    return emptyChunk;
}

inline const Chunk &chunk::read(int64_t x, int64_t y, int64_t z)
{
    ChunkCoordinate coordinate(x, y, z);
    try
    {
        return *chunkMap.at(coordinate).get();
    }
    catch (const std::out_of_range &e)
    {
        // std::cout << "Reading...\n";
        return getEmptyChunk(); // Return a const reference to the emptyChunk
    }
}

int16_t chunk::getTile(int64_t getx, int64_t gety, int64_t getz)
{
    try
    {
        Chunk &tempChunk = *chunkMap.at(ChunkCoordinate(floor(getx / 32), floor(gety / 32), floor(getz / 32))).get();
        return tempChunk.blockData[(getx % 32 + 32) % 32][(gety % 32 + 32) % 32][(getz % 32 + 32) % 32];
    }
    catch (const std::out_of_range &e)
    {
        return 0;
    }
    return 0;
}

void chunk::setTile(int64_t getx, int64_t gety, int64_t getz, int16_t tile)
{
    ChunkCoordinate chunkCoord(floor(float(getx) / 32.0f), floor(float(gety) / 32.0f), floor(float(getz) / 32.0f));
    auto chunkIt = chunkMap.find(chunkCoord);
    if (chunkIt != chunkMap.end())
    {
        chunkMap[chunkCoord]->blockData[(getx % 32 + 32) % 32][(gety % 32 + 32) % 32][(getz % 32 + 32) % 32] = tile;
    }
    else if (myRenderer.isMultiplayer)
    {
        // std::cerr << "[error] Missing chunk for placing " << chunkCoord << "\n";
        chunkMap[chunkCoord] = std::make_shared<Chunk>(floor(float(getx) / 32.0f), floor(float(gety) / 32.0f), floor(float(getz) / 32.0f), 0);
        chunkMap[chunkCoord]->blockData[(getx % 32 + 32) % 32][(gety % 32 + 32) % 32][(getz % 32 + 32) % 32] = tile;
    }
    chunkMap[chunkCoord]->updateMesh();
}

// Write to a chunk at a coordinate
void chunk::write(int64_t x, int64_t y, int64_t z, const Chunk &chunk)
{
    chunkMap[ChunkCoordinate(x, y, z)] = std::make_shared<Chunk>(chunk);
}

void chunk::write(const Chunk &chunk)
{
    chunkMap[ChunkCoordinate(chunk.x, chunk.y, chunk.z)] = std::make_shared<Chunk>(chunk);
}

// Delete a chunk based on its coordinate
void chunk::remove(int64_t x, int64_t y, int64_t z)
{
    ChunkCoordinate coordinate(x, y, z);
    chunkMap.erase(coordinate);
}

// Create a new chunk with its unique coordinate
void chunk::create(int64_t x, int64_t y, int64_t z)
{
    ChunkCoordinate coordinate(x, y, z);
    chunkMap[coordinate] = std::make_shared<Chunk>(x, y, z);
}

void chunk::updateNeighbours(int64_t x, int64_t y, int64_t z)
{
    if (chunkMap.count(ChunkCoordinate(x + 1, y, z)) > 0)
    {
        chunkMap[ChunkCoordinate(x + 1, y, z)]->updateMesh();
    }
    if (chunkMap.count(ChunkCoordinate(x - 1, y, z)) > 0)
    {
        chunkMap[ChunkCoordinate(x - 1, y, z)]->updateMesh();
    }
    if (chunkMap.count(ChunkCoordinate(x, y + 1, z)) > 0)
    {
        chunkMap[ChunkCoordinate(x, y + 1, z)]->updateMesh();
    }
    if (chunkMap.count(ChunkCoordinate(x, y - 1, z)) > 0)
    {
        chunkMap[ChunkCoordinate(x, y - 1, z)]->updateMesh();
    }
    if (chunkMap.count(ChunkCoordinate(x, y, z + 1)) > 0)
    {
        chunkMap[ChunkCoordinate(x, y, z + 1)]->updateMesh();
    }
    if (chunkMap.count(ChunkCoordinate(x, y, z - 1)) > 0)
    {
        chunkMap[ChunkCoordinate(x, y, z - 1)]->updateMesh();
    }
}

void chunk::loadChunk(long x, long y, long z)
{
    /*std::cout << (chunkMap.count(ChunkCoordinate(x, y, z)) == 0 &&
                  std::abs(x - (myRenderer.camera.X / 32.0)) <= renderDistance &&
                  std::abs(y - (myRenderer.camera.Y / 32.0)) <= renderDistance &&
                  std::abs(z - (myRenderer.camera.Z / 32.0)) <= renderDistance)
              << "\n";
    std::cout << std::abs(x - (myRenderer.camera.X / 32.0)) << ", " << std::abs(y - (myRenderer.camera.Y / 32.0)) << ", " << std::abs(z - (myRenderer.camera.Z / 32.0)) << "\n";*/
    if (chunkMap.count(ChunkCoordinate(x, y, z)) == 0 &&
        std::abs(x - (myRenderer.camera.X / 32.0)) <= renderDistance &&
        std::abs(y - (myRenderer.camera.Y / 32.0)) <= renderDistance &&
        std::abs(z - (myRenderer.camera.Z / 32.0)) <= renderDistance)
    {
        Chunk genTempChunk = Chunk(x, y, z);
        genTempChunk.lock = 1;
        write(genTempChunk);
        if (!isArrayFilledWithZeroes(genTempChunk.blockData))
        {
            // std::cout << "[info] Started updating neighbour meshes\n";
            chunk::updateNeighbours(x, y, z);
            // std::cout << "[info] Finished updating neighbour meshes\n";
        }
        chunkMap[ChunkCoordinate(x, y, z)]->lock = 0;
        if (quit)
        {
            return;
        }
    }
}

void chunk::manage()
{
    // std::lock_guard<std::mutex> lock(chunkMapMutex);
    for (long i = ((myRenderer.camera.X / 32.0) - renderDistance); i < ((myRenderer.camera.X / 32.0) + renderDistance); i++)
    {
        for (long j = ((myRenderer.camera.Y / 32.0) - renderDistance); j < ((myRenderer.camera.Y / 32.0) + renderDistance); j++)
        {
            for (long k = ((myRenderer.camera.Z / 32.0) - renderDistance); k < ((myRenderer.camera.Z / 32.0) + renderDistance); k++)
            {
                loadChunk(i, j, k);
                if (quit)
                {
                    return;
                }
            }
        }
    }
}

void chunk::unload()
{
    std::lock_guard<std::mutex> lock(chunkMapMutex);
    for (auto it = chunkMap.begin(); it != chunkMap.end();)
    {
        ChunkCoordinate tempCoord = it->first;
        /*std::cout << std::abs(tempCoord.x - long(myRenderer.camera.X / 32.0)) << ", " <<
            std::abs(tempCoord.y - long(myRenderer.camera.Y / 32.0)) << ", " <<
            std::abs(tempCoord.z - long(myRenderer.camera.Z / 32.0)) << "\n";*/

        if (std::abs(tempCoord.x - long(myRenderer.camera.X / 32.0)) > renderDistance ||
            std::abs(tempCoord.y - long(myRenderer.camera.Y / 32.0)) > renderDistance ||
            std::abs(tempCoord.z - long(myRenderer.camera.Z / 32.0)) > renderDistance)
        {
            // TODO Saving to disk
            chunkMap[tempCoord]->lock = 1;
            chunkMap[tempCoord].reset();
            it = chunkMap.erase(it); // Erase and get the next valid iterator
            std::cout << "[info] Unloading Chunk " << tempCoord << "\n";
        }
        else
        {
            ++it; // Move to the next element in the map
        }
    }
}

void Chunk::destroyBuffers()
{
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
}

void Chunk::updateBuffers()
{
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static inline int8_t isEqualTo31(int8_t value)
{
    if (value == 31)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

inline bool shouldBeDrawn(int16_t (&blockData)[32][32][32], const Chunk &compareChunk, int8_t cullX, int8_t cullY, int8_t cullZ, int8_t whichCoordinate, int8_t value)
{
    int16_t currentBlock = blockData[cullX][cullY][cullZ];
    BlockConfig currentConfig(1, 1, 1);
    if (currentBlock > 0 && currentBlock < int16_t(sizeof(blockConfig) / sizeof(BlockConfig)))
    {
        currentConfig = blockConfig[currentBlock];
    }
    bool result = currentConfig.renderModel == 2 ||
                  currentConfig.renderModel == 3;
    int8_t tempValue;
    if (value == 0)
    {
        tempValue = 31;
    }
    else
    {
        tempValue = 0;
    }
    switch (whichCoordinate)
    {
    case 1:
        if (cullX == value)
        {
            result = result || (blockConfig[compareChunk.blockData[tempValue][cullY][cullZ]].renderModel != currentConfig.renderModel);
        }
        else
        {
            result = result || (blockConfig[blockData[cullX + isEqualTo31(value)][cullY][cullZ]].renderModel != currentConfig.renderModel);
        }
        break;
    case 2:
        if (cullY == value)
        {
            result = result || (blockConfig[compareChunk.blockData[cullX][tempValue][cullZ]].renderModel != currentConfig.renderModel);
        }
        else
        {
            result = result || (blockConfig[blockData[cullX][cullY + isEqualTo31(value)][cullZ]].renderModel != currentConfig.renderModel);
        }
        break;
    case 3:
        if (cullZ == value)
        {
            result = result || (blockConfig[compareChunk.blockData[cullX][cullY][tempValue]].renderModel != currentConfig.renderModel);
        }
        else
        {
            result = result || (blockConfig[blockData[cullX][cullY][cullZ + isEqualTo31(value)]].renderModel != currentConfig.renderModel);
        }
        break;
    /*case 1:
        if (cullX == value)
        {
            result = result || (compareChunk.blockData[tempValue][cullY][cullZ] == 0);
        }
        else
        {
            result = result || (blockData[cullX + isEqualTo31(value)][cullY][cullZ] == 0);
        }
        break;
    case 2:
        if (cullY == value)
        {
            result = result || (compareChunk.blockData[cullX][tempValue][cullZ] == 0);
        }
        else
        {
            result = result || (blockData[cullX][cullY + isEqualTo31(value)][cullZ] == 0);
        }
        break;
    case 3:
        if (cullZ == value)
        {
            result = result || (compareChunk.blockData[cullX][cullY][tempValue] == 0);
        }
        else
        {
            result = result || (blockData[cullX][cullY][cullZ + isEqualTo31(value)] == 0);
        }
        break;*/
    default:
        std::cerr << "[error] Invalid use of function shouldBeDrawn()\n";
        break;
    }
    return result;
}

void Chunk::updateMesh()
{
    lock = 1;
    meshUpdated = 0;
    vertices.clear();
    indices.clear();
    indicesNum = 0;
    if (isArrayFilledWithZeroes(blockData))
    {
        return;
    }

    // std::cout << "started chunkmesh\n";
    const Chunk &bottomChunk = chunk::read(x, y - 1, z);
    const Chunk &rightChunk = chunk::read(x - 1, y, z);
    const Chunk &backChunk = chunk::read(x, y, z - 1);
    const Chunk &topChunk = chunk::read(x, y + 1, z);
    const Chunk &leftChunk = chunk::read(x + 1, y, z);
    const Chunk &frontChunk = chunk::read(x, y, z + 1);

    // std::cout << "finished neighbour read\n";

    for (int8_t i = 0; i < 32; i++)
    {
        for (int8_t j = 0; j < 32; j++)
        {
            for (int8_t k = 0; k < 32; k++)
            {
                if (blockData[i][j][k] != 0)
                {
                    // std::cout << "rendering block " << ChunkCoordinate(i,j,k) << ", " << blockData[i][j][k]  << "\n";
                    int16_t currentBlock = blockData[i][j][k];
                    BlockConfig currentConfig(1, 1, 1);
                    if (currentBlock > 0 && currentBlock < int16_t(sizeof(blockConfig) / sizeof(BlockConfig)))
                    {
                        currentConfig = blockConfig[currentBlock];
                    }
                    float grassr, grassg, grassb;
                    if (currentBlock == 2 ||
                        currentConfig.renderModel == 3)
                    {
                        float grasstemperature = 0.5;
                        float grasshumidity = 1;
                        grassr = (129 + ((191 - 129) * grasstemperature));
                        grassg = (180 + ((183 - 180) * grasstemperature));
                        grassb = (150 + ((85 - 150) * grasstemperature));
                        grassr = float(grassr + ((73 - grassr) * grasshumidity * grasstemperature)) / 256;
                        grassg = float(grassg + ((205 - grassg) * grasshumidity * grasstemperature)) / 256;
                        grassb = float(grassb + ((51 - grassb) * grasshumidity * grasstemperature)) / 256;
                    }
                    else
                    {
                        grassr = 1;
                        grassg = 1;
                        grassb = 1;
                    }
                    if (shouldBeDrawn(blockData, bottomChunk, i, j, k, 2, 0))
                    {
                        face::top(
                            i, j, k,
                            currentConfig.texDown,
                            vertices, indices,
                            0.65 * float((grassr * (currentBlock != 2)) + (currentBlock == 2)),
                            0.65 * float((grassg * (currentBlock != 2)) + (currentBlock == 2)),
                            0.65 * float((grassb * (currentBlock != 2)) + (currentBlock == 2)),
                            1);
                    }
                    if (shouldBeDrawn(blockData, topChunk, i, j, k, 2, 31))
                    {
                        face::bottom(
                            i, j + 1, k,
                            currentConfig.texUp,
                            vertices, indices,
                            1.0 * grassr,
                            1.0 * grassg,
                            1.0 * grassb,
                            1);
                    }
                    if (shouldBeDrawn(blockData, rightChunk, i, j, k, 1, 0))
                    {
                        face::left(
                            i, j, k,
                            currentConfig.texRight,
                            vertices, indices,
                            0.8 * float((grassr * (currentBlock != 2)) + (currentBlock == 2)),
                            0.8 * float((grassg * (currentBlock != 2)) + (currentBlock == 2)),
                            0.8 * float((grassb * (currentBlock != 2)) + (currentBlock == 2)),
                            1);
                    }
                    if (shouldBeDrawn(blockData, leftChunk, i, j, k, 1, 31))
                    {
                        face::right(
                            i, j, k,
                            currentConfig.texLeft,
                            vertices, indices,
                            0.8 * float((grassr * (currentBlock != 2)) + (currentBlock == 2)),
                            0.8 * float((grassg * (currentBlock != 2)) + (currentBlock == 2)),
                            0.8 * float((grassb * (currentBlock != 2)) + (currentBlock == 2)),
                            1);
                    }
                    if (shouldBeDrawn(blockData, backChunk, i, j, k, 3, 0))
                    {
                        face::front(
                            i, j, k,
                            currentConfig.texBack,
                            vertices, indices,
                            0.9 * float((grassr * (currentBlock != 2)) + (currentBlock == 2)),
                            0.9 * float((grassg * (currentBlock != 2)) + (currentBlock == 2)),
                            0.9 * float((grassb * (currentBlock != 2)) + (currentBlock == 2)),
                            1);
                    }
                    if (shouldBeDrawn(blockData, frontChunk, i, j, k, 3, 31))
                    {
                        face::back(
                            i, j, k,
                            currentConfig.texFront,
                            vertices, indices,
                            0.7 * float((grassr * (currentBlock != 2)) + (currentBlock == 2)),
                            0.7 * float((grassg * (currentBlock != 2)) + (currentBlock == 2)),
                            0.7 * float((grassb * (currentBlock != 2)) + (currentBlock == 2)),
                            1);
                    }
                    if (currentBlock == 2)
                    {
                        if ((i == 0 && rightChunk.blockData[31][j][k] == 0) ||
                            (i > 0 && blockData[i - 1][j][k] == 0))
                        {
                            face::left(
                                i, j, k, 39,
                                vertices, indices,
                                grassr * 0.85,
                                grassg * 0.85,
                                grassb * 0.85,
                                1);
                        }
                        if ((i == 31 && leftChunk.blockData[0][j][k] == 0) ||
                            (i < 31 && blockData[i + 1][j][k] == 0))
                        {
                            face::right(
                                i, j, k, 39,
                                vertices, indices,
                                grassr * 0.85,
                                grassg * 0.85,
                                grassb * 0.85,
                                1);
                        }
                        if ((k == 0 && backChunk.blockData[i][j][31] == 0) ||
                            (k > 0 && blockData[i][j][k - 1] == 0))
                        {
                            face::front(
                                i, j, k, 39,
                                vertices, indices,
                                grassr * 0.9,
                                grassg * 0.9,
                                grassb * 0.9,
                                1);
                        }
                        if ((k == 31 && frontChunk.blockData[i][j][0] == 0) ||
                            (k < 31 && blockData[i][j][k + 1] == 0))
                        {
                            face::back(
                                i, j, k, 39,
                                vertices, indices,
                                grassr * 0.8,
                                grassg * 0.8,
                                grassb * 0.8,
                                1);
                        }
                    }
                    // std::cout << "finished rendering block\n";
                }
            }
        }
    }
    indicesNum = indices.size();
    lock = 0;
    std::lock_guard<std::mutex> lock(vectorMutex);
    chunksToUpdate.push_back(ChunkCoordinate(x, y, z));
}
