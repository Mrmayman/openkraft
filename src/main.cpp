#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glu.h>
#include <zlib.h>

#include <iostream>
#include <cstdio>
#include <cmath>
#include <string>
#include <csignal>
#include <exception>
#include <cstdint>
#include <iomanip>

#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>

#include "chunk.h"
#include "entities.h"
#include "facedraw.h"
#include "environment.h"
#include "framework.h"
#include "init.h"
#include "netcode.h"

#include "FastNoiseLite.h"

GLuint SkyVBO, SkyIBO;
std::vector<GLfloat> SkyVertices;
std::vector<GLuint> SkyIndices;
GLuint shaderProgram;

int mySocket = -1;

float cameraX = 0;
float cameraY = 0;
float cameraZ = 0;

float rotX = 0;
float rotY = 0;

bool quit = false;

float mouseX = 0;
float mouseY = 0;

float delta = 1;

bool loggedIn = 0;

void manageChunks()
{
    // std::lock_guard<std::mutex> lock(chunkMapMutex);
    while (!quit)
    {
        if (isMultiplayer)
        {
            uint8_t receivedData[cooc::bufferSize];
            int bytesRead = 0;
            memset(receivedData, 0, sizeof(receivedData));
            if (!cooc::receive(bytesRead, receivedData, mySocket))
            {
                std::cerr << "[error] Connection closed or error occurred\n";
                quit = 1;
                return;
            }
            std::vector<uint8_t> aliveData = {0x00};
            cooc::sendpacket(aliveData, mySocket);

            std::vector<uint8_t> loginData = {0x01, 0x00, 0x00, 0x00, 0x0e};
            cooc::appendString16("mrmayman", loginData);
            loginData.insert(loginData.end(), {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
            std::vector<uint8_t> positionData = {0x0D};
            if (loggedIn)
            {
                positionData = {0x0D};
                cooc::appendDouble(commonEntities[0]->x, positionData);
                cooc::appendDouble(commonEntities[0]->y, positionData);
                cooc::appendDouble(commonEntities[0]->y + 1.64, positionData);
                cooc::appendDouble(commonEntities[0]->z, positionData);
                cooc::appendFloat(0.0, positionData);
                cooc::appendFloat(0.0, positionData);
                positionData.push_back(0x0);
                cooc::sendpacket(positionData, mySocket);
                positionData = {0x0D};
            }

            for (int i = 0; i < cooc::bufferSize; i++)
            {
                uint16_t length = 0;
                uint8_t currentPacket = receivedData[i];
                std::vector<uint8_t> decodeVec;
                std::string stringbuffer;

                int32_t chunkX = 0;
                int32_t chunkZ = 0;

                double doubleX = 0;
                double doubleY = 0;
                double doubleZ = 0;

                double doubleW = 0;

                int compressedSize = 0;
                int16_t tempShort = 0;

                std::vector<uint8_t> compressedData;
                std::vector<uint8_t> decompressedData;

                uint8_t tempbytes[8];

                switch (currentPacket)
                {
                case 0x00: // Heartbeat
                    break;
                case 0x02: // Login
                    std::cerr << "Packet : Login\n";
                    if (!loggedIn)
                    {
                        if (receivedData[i + 4] != 0x2d)
                        {
                            std::cerr << "[error] Account authentication required! Not supported.\nTry joining a cracked server\n";
                            quit = 1;
                            return;
                        }
                        cooc::sendpacket(loginData, mySocket);
                    }
                    loggedIn = 1;
                    i += 2;
                    break;
                case 0x03: // Chat
                    std::cerr << "Packet : Chat\n";
                    length = (static_cast<uint16_t>(receivedData[i + 1]) << 8) |
                             static_cast<uint16_t>(receivedData[i + 2]);
                    if (length > 254)
                    {
                        break;
                    }
                    for (int j = 1; j < (length * 2) && j < cooc::bufferSize; j += 2)
                    {
                        stringbuffer += receivedData[i + 3 + j];
                    }
                    std::cout << "[chat] " << stringbuffer << "\n";
                    std::cerr << "Packet : Chat Done\n";
                    i += length;
                    break;
                case 0x0d:
                    std::cerr << "Packet : Position\n";
                    for (int j = 0; j < 8; j++)
                    {
                        tempbytes[j] = receivedData[i + 1 + j];
                    }
                    doubleX = cooc::convertDouble(tempbytes);
                    for (int j = 0; j < 8; j++)
                    {
                        tempbytes[j] = receivedData[i + 9 + j];
                    }
                    doubleW = cooc::convertDouble(tempbytes);
                    for (int j = 0; j < 8; j++)
                    {
                        tempbytes[j] = receivedData[i + 17 + j];
                    }
                    doubleY = cooc::convertDouble(tempbytes);
                    for (int j = 0; j < 8; j++)
                    {
                        tempbytes[j] = receivedData[i + 25 + j];
                    }
                    doubleZ = cooc::convertDouble(tempbytes);
                    std::cout << doubleX << ", " << doubleY << ", " << doubleZ << "\n";
                    if (abs(doubleX) > 32000000 || abs(doubleZ) > 32000000 || (doubleW - doubleY) > 1.66 || (doubleW - doubleY) < 0.1)
                    {
                        std::cerr << "Illegal Stance\n";
                        break;
                    }
                    commonEntities[0]->x = float(doubleX);
                    commonEntities[0]->y = float(doubleY);
                    commonEntities[0]->z = float(doubleZ);

                    positionData = {0x0D};
                    for (int j = 0; j < 8; j++)
                    {
                        positionData.push_back(receivedData[i + 1 + j]);
                    }
                    for (int j = 0; j < 8; j++)
                    {
                        positionData.push_back(receivedData[i + 17 + j]);
                    }
                    for (int j = 0; j < 8; j++)
                    {
                        positionData.push_back(receivedData[i + 9 + j]);
                    }
                    for (int j = 0; j < 17; j++)
                    {
                        positionData.push_back(receivedData[i + 25 + j]);
                    }
                    cooc::sendpacket(positionData, mySocket);

                    positionData = {0x0D};
                    break;
                case 0x14:
                    std::cerr << "Packet : Spawn Player\n";
                    /*length = (static_cast<uint16_t>(receivedData[i+1+4]) << 8) |
                              static_cast<uint16_t>(receivedData[i+2+4]);
                    i += length;*/
                    break;
                case 0x18:
                    std::cerr << "Broken Packet : Spawn Mob\n";
                    break;
                case 0x28:
                    std::cerr << "Broken Packet : Mob Metadata\n";
                    break;
                case 0x32: // Pre-Chunk
                    std::cerr << "Packet : Pre-Chunk\n";
                    if (!receivedData[i + 9])
                    {
                        break;
                    }
                    chunkX = cooc::convertInt(
                        receivedData[i + 1],
                        receivedData[i + 2],
                        receivedData[i + 3],
                        receivedData[i + 4]);
                    chunkZ = cooc::convertInt(
                        receivedData[i + 5],
                        receivedData[i + 6],
                        receivedData[i + 7],
                        receivedData[i + 8]);
                    break;
                case 0x33:
                    std::cerr << "Packet : Chunk\n";
                    chunkX = cooc::convertInt(
                        receivedData[i + 1],
                        receivedData[i + 2],
                        receivedData[i + 3],
                        receivedData[i + 4]);
                    tempShort = cooc::convertShort(
                        receivedData[i + 5],
                        receivedData[i + 6]);
                    chunkZ = cooc::convertInt(
                        receivedData[i + 7],
                        receivedData[i + 8],
                        receivedData[i + 9],
                        receivedData[i + 10]);

                    if (tempShort != 0)
                    {
                        std::cout << tempShort << ", ";
                        printf("%d\n", receivedData[i + 12]);
                        // throw std::runtime_error("Chunk is vertically misaligned");
                        break;
                    }

                    compressedSize = cooc::convertInt(
                        receivedData[i + 14],
                        receivedData[i + 15],
                        receivedData[i + 16],
                        receivedData[i + 17]);

                    // std::cout << bytesRead << ", " << compressedSize << "\n";
                    std::cout << "started chunk " << compressedSize << "\n";
                    if (compressedSize > 1000000)
                    {
                        std::cerr << "invalid size chunk " << compressedSize << "\n";
                        i = cooc::bufferSize;
                        break;
                    }
                    i += 18;
                    for (int j = 0; j < compressedSize; j++)
                    {
                        if (i >= bytesRead)
                        {
                            // throw std::runtime_error("The chunk is cut up :(");
                            std::cout << "Cut up chunk, reading next packet "
                                      << "\n";
                            cooc::sendpacket(aliveData, mySocket);
                            if (quit)
                            {
                                return;
                            }
                            // break;
                            std::this_thread::sleep_for(std::chrono::milliseconds(8));
                            i = 0;
                            memset(receivedData, 0, sizeof(receivedData));
                            if (!cooc::receive(bytesRead, receivedData, mySocket))
                            {
                                std::cerr << "[error] Connection closed or error occurred while loading chunk\n";
                                quit = 1;
                                return;
                            }
                        }
                        // std::cout << "reading chunk data " << i << "\n";
                        compressedData.push_back(receivedData[i]);
                        i++;
                    }
                    // std::cout << "finished chunk\n";
                    i -= 18;

                    if (!cooc::decompress(compressedData, decompressedData))
                    {
                        std::cout << "you idoit\n";
                        break;
                    }
                    for (int cyy = 0; cyy < 8; cyy++)
                    {
                        int64_t chunkTempX = floor(float(chunkX) / 32.0f);
                        int64_t chunkTempY = cyy;
                        int64_t chunkTempZ = floor(float(chunkZ) / 32.0f);
                        if (chunkMap.count(ChunkCoordinate(chunkTempX, chunkTempY, chunkTempZ)) == 0 &&
                            abs(chunkTempX - (cameraX / 32)) <= chunk::renderDistance &&
                            abs(chunkTempY - (cameraY / 32)) <= chunk::renderDistance &&
                            abs(chunkTempZ - (cameraZ / 32)) <= chunk::renderDistance)
                        {
                            Chunk genTempChunk = Chunk(chunkTempX, chunkTempY, chunkTempZ, 0);
                            // genTempChunk.fill(1);
                            // genTempChunk.updateMesh();
                            genTempChunk.lock = 1;
                            chunk::write(genTempChunk);
                            if (!isArrayFilledWithZeroes(genTempChunk.blockData))
                            {
                                chunk::updateNeighbours(chunkTempX, chunkTempY, chunkTempZ);
                            }
                            chunkMap[ChunkCoordinate(chunkTempX, chunkTempY, chunkTempZ)].lock = 0;
                        }
                    }

                    for (int cy = 0; cy < 128; cy++)
                    {
                        for (int cx = 0; cx < 16; cx++)
                        {
                            for (int cz = 0; cz < 16; cz++)
                            {
                                int index = cy + (cz * 128) + (cx * 128 * 16);
                                int64_t chunkTempX = floor(float(chunkX) / 32.0f);
                                int64_t chunkTempY = floor(float(cy) / 32.0f);
                                int64_t chunkTempZ = floor(float(chunkZ) / 32.0f);
                                ChunkCoordinate chunkCoord(chunkTempX, chunkTempY, chunkTempZ);
                                // std::cout << ((chunkX + cx) % 32 + 32) % 32 << ", " << ((cy) % 32 + 32) % 32 << ", " << ((chunkZ + cz) % 32 + 32) % 32 << "\n";
                                if (index < int(decompressedData.size()))
                                {
                                    chunkMap[chunkCoord].blockData[((chunkX + cx) % 32 + 32) % 32]
                                                                  [((cy) % 32 + 32) % 32]
                                                                  [((chunkZ + cz) % 32 + 32) % 32] = int(decompressedData[index]);
                                }
                                // chunkMap[chunkCoord].updateMesh();
                                // chunk::updateNeighbours(chunkTempX, chunkTempY, chunkTempZ);
                                // std::cout << int(decompressedData[index]) << "\n";
                            }
                        }
                    }
                    for (int cyy = 0; cyy < 8; cyy++)
                    {
                        int64_t chunkTempX = floor(float(chunkX) / 32.0f);
                        int64_t chunkTempZ = floor(float(chunkZ) / 32.0f);
                        ChunkCoordinate thisChunk = ChunkCoordinate(chunkTempX, cyy, chunkTempZ);
                        chunkMap[thisChunk].updateMesh();
                        chunk::updateNeighbours(chunkTempX, cyy, chunkTempZ);
                        chunkMap[thisChunk].lock = 0;
                    }

                    // i += compressedSize;
                    break;
                case 0x34:
                    compressedSize = cooc::convertShort(
                        receivedData[i + 9],
                        receivedData[i + 10]);
                    std::cout << "Packet : Multi Block Change " << static_cast<int>(compressedSize) << "\n";
                    chunkX = cooc::convertInt(
                        receivedData[i + 1],
                        receivedData[i + 2],
                        receivedData[i + 3],
                        receivedData[i + 4]);
                    chunkZ = cooc::convertInt(
                        receivedData[i + 5],
                        receivedData[i + 6],
                        receivedData[i + 7],
                        receivedData[i + 8]);
                    if (compressedSize > 4096)
                    {
                        break;
                    }
                    for (int j = 0; j < compressedSize; j++)
                    {
                        tempShort = cooc::convertShort(receivedData[i + 11 + (j * 2)], receivedData[i + 12 + (j * 2)]);
                        chunk::setTile(
                            ((tempShort >> 12) & 0xF) + chunkX,
                            ((tempShort >> 8) & 0xF) + chunkZ,
                            tempShort & 0xFF,
                            receivedData[i + 11 + (compressedSize * 2) + j]);
                        std::cout << static_cast<int>(receivedData[i + 11 + (compressedSize * 2) + j]) << "\n";
                    }
                    i += compressedSize * 4;
                    break;
                case 0x35:
                    std::cout << "Packet : Block Change\n";
                    chunkX = cooc::convertInt(
                        receivedData[i + 1],
                        receivedData[i + 2],
                        receivedData[i + 3],
                        receivedData[i + 4]);
                    chunkZ = cooc::convertInt(
                        receivedData[i + 6],
                        receivedData[i + 7],
                        receivedData[i + 8],
                        receivedData[i + 9]);
                    std::cout << static_cast<int>(receivedData[i + 10]) << "\n";
                    chunk::setTile(chunkX, receivedData[i + 5], chunkZ, receivedData[i + 10]);
                    // i++;
                    break;
                case 0x3c:
                    std::cerr << "Broken Packet : Explosion\n";
                    break;
                case 0x64:
                    std::cerr << "Broken Packet : Open Inventory Window\n";
                    break;
                case 0x68:
                    std::cerr << "Broken Packet : Inventory Window Payload\n";
                    compressedSize = cooc::convertShort(
                        receivedData[i + 9],
                        receivedData[i + 10]);
                    for (int j = 0; j < compressedSize; j++)
                    {
                        chunkX = cooc::convertShort(
                            receivedData[i + 3],
                            receivedData[i + 4]);
                        i += 1;
                        if (chunkX == -1)
                        {
                            i += 2;
                        }
                    }
                    break;
                case 0x82:
                    std::cerr << "Broken Packet : Update Sign\n";
                    break;
                case 0x83:
                    std::cerr << "Broken Packet : Handheld Map Data\n";
                    break;
                default:;
                    // std::cout << "Hex value: 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(currentPacket) << std::endl;
                }
                if (quit)
                {
                    return;
                }
                try
                {
                    i += cooc::packetSizes.at(currentPacket) - 1;
                    // std::cout << "Hex value: 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(currentPacket) << std::endl;
                }
                catch (std::exception &e)
                {
                    std::cout << "Invalid Packet: 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(currentPacket) << std::endl;
                    i = cooc::bufferSize;
                }
            }
            memset(receivedData, 0, sizeof(receivedData));
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        else
        {
            chunk::unload();
            chunk::manage();
        }
    }
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--version")
        {
            std::cout << "OpenKraft 0.1 pre-1\nA primitive Minecraft clone written in C++ and OpenGL\nFor help, run with the --help argument\n\nCopyright (C) 2023 Mrmayman.\nLicence GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>. This is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\nWritten by Mrmayman\n";
            return 0;
        }
        else if (arg == "--help")
        {
            std::cout << "OpenKraft is a primitive Minecraft clone written in C++ and OpenGL\n\nOptions:\n  --help           Prints this help message\n  --version        Prints the program version and related information\n  --multiplayer    Runs the early multiplayer test. VERY BUGGY\n";
            return 0;
        }
        else if (arg == "--multiplayer")
        {
            isMultiplayer = 1;
        }
    }

    if (!initGame())
    {
        return 1;
    }
    
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetSeed(seed);
    texAtlas = loadTexture("terrain.png");
    glClearColor(39.0f / 256.0f, 25.0f / 256.0f, 17.0f / 256.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    nDrawText(13, 13, "Generating World...", {255, 50, 50, 50});
    nDrawText(10, 10, "Generating World...", {255, 255, 255, 255});
    SDL_GL_SwapWindow(window);

    buildShader();
    if (!isMultiplayer)
    {
        chunk::manage();
    }

    std::thread manageThread(manageChunks);
    commonEntities[0] = new EntityPlayer();
    commonEntities[0]->y = 96;
    commonEntities[0]->x = 0;

    chunk::setTile(-191, 67, -7, 1);

    while (!quit)
    {
        // chunk::setTile(int64_t(commonEntities[0]->x + 1), int64_t(commonEntities[0]->y + 2), int64_t(commonEntities[0]->z + 1), 18);
        handleEvents(quit);
        render();
        buildMeshes();
        nDrawText(13, 13,
                  std::to_string(int(1 / delta)) + " FPS\nx: " + std::to_string(cameraX) +
                      "\ny: " + std::to_string(cameraY) +
                      "\nz: " + std::to_string(cameraZ),
                  {255, 50, 50, 50});
        nDrawText(10, 10,
                  std::to_string(int(1 / delta)) + " FPS\nx: " + std::to_string(cameraX) +
                      "\ny: " + std::to_string(cameraY) +
                      "\nz: " + std::to_string(cameraZ),
                  {255, 255, 255, 255});
        SDL_GL_SwapWindow(window);
        glClear(GL_DEPTH_BUFFER_BIT);
        // chunk::unload(); chunk::manage(0);
    }
    if (isMultiplayer)
    {
        std::vector<uint8_t> quitData = {0xff};
        cooc::appendString16("Quitting", quitData);
    }
    manageThread.join();

    cleanup();
    return 0;
}
