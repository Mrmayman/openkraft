#pragma once

#include <iostream>

#include "sendPacket.h"
#include "networkingFunctions.h"

#include "../chunk.h"
#include "../gameRenderer/main.h"

uint8_t receivedData[net::bufferSize];
int bytesRead = 0;
int packetCounter = 0;

extern bool quit;

namespace handlePacket
{
    void login()
    {
        std::cout << "[packet] Login\n";

        if (loggedIn)
        {
            return;
        }
        if (receivedData[packetCounter + 4] != 0x2d)
        {
            std::cerr << "[error] Account authentication required! Not supported.\nTry joining a cracked server\n";
            quit = 1;
            return;
        }
        sendPacket::login();
        loggedIn = 1;
        packetCounter += 2;
    }

    void chat()
    {
        // Read message length
        short messageLength = (static_cast<uint16_t>(receivedData[packetCounter + 1]) << 8) |
                              static_cast<uint16_t>(receivedData[packetCounter + 2]);

        // Message too big
        if (messageLength > 254)
        {
            return;
        }

        // Read message contents
        std::string messageContents;
        for (int j = 1; j < (messageLength * 2) && j < net::bufferSize; j += 2)
        {
            messageContents += receivedData[packetCounter + 3 + j];
        }

        std::cout << "[chat] " << messageContents << "\n";
        packetCounter += messageLength;
    }

    void playerPositionAndLook()
    {
        std::cout << "[packet] Player position (";

        uint8_t tempbytes[8];

        // Read X
        for (int j = 0; j < 8; j++)
        {
            tempbytes[j] = receivedData[packetCounter + 1 + j];
        }
        double playerX = net::convertDouble(tempbytes);

        // Read Stance (Y + 1.65)
        for (int j = 0; j < 8; j++)
        {
            tempbytes[j] = receivedData[packetCounter + 9 + j];
        }
        double playerStance = net::convertDouble(tempbytes);

        // Read Y
        for (int j = 0; j < 8; j++)
        {
            tempbytes[j] = receivedData[packetCounter + 17 + j];
        }
        double playerY = net::convertDouble(tempbytes);

        // Read Z
        for (int j = 0; j < 8; j++)
        {
            tempbytes[j] = receivedData[packetCounter + 25 + j];
        }
        double playerZ = net::convertDouble(tempbytes);

        // Log the position
        std::cout << playerX << ", " << playerY << ", " << playerZ << ")\n";

        // Check if position is valid
        if (abs(playerX) > 32000000 || abs(playerZ) > 32000000 || (playerStance - playerY) > 1.66 || (playerStance - playerY) < 0.1)
        {
            std::cerr << "Illegal Stance\n";
            return;
        }

        // Set player position
        commonEntities[0]->x = float(playerX);
        commonEntities[0]->y = float(playerY);
        commonEntities[0]->z = float(playerZ);

        // Send position back to server
        sendPacket::position();
    }

    void preChunk()
    {
        std::cout << "[packet] Pre-Chunk\n";

        // Read data
        int chunkX = net::convertInt(
            receivedData[packetCounter + 1],
            receivedData[packetCounter + 2],
            receivedData[packetCounter + 3],
            receivedData[packetCounter + 4]);
        int chunkZ = net::convertInt(
            receivedData[packetCounter + 5],
            receivedData[packetCounter + 6],
            receivedData[packetCounter + 7],
            receivedData[packetCounter + 8]);

        for (int chunkLoadCounter = 0; chunkLoadCounter < (128 / 32); chunkLoadCounter += (32 / 32))
        {
            if (receivedData[packetCounter + 9])
            {
                // Initialize the chunk, not required
            }
            else
            {
                // Unload the chunk
                ChunkCoordinate tempCoord(chunkX, chunkLoadCounter, chunkZ);
                std::cout << "Server unloaded location " << tempCoord << "\n";
                chunkMap[tempCoord]->lock = 1;
                chunkMap[tempCoord].reset();
            }
        }
    }

    void multiBlockChange()
    {
        std::cout << "Packet : Multi Block Change ";

        // Read data
        short numberOfBlocks = net::convertShort(
            receivedData[packetCounter + 9],
            receivedData[packetCounter + 10]);
        int chunkX = net::convertInt(
            receivedData[packetCounter + 1],
            receivedData[packetCounter + 2],
            receivedData[packetCounter + 3],
            receivedData[packetCounter + 4]);
        int chunkZ = net::convertInt(
            receivedData[packetCounter + 5],
            receivedData[packetCounter + 6],
            receivedData[packetCounter + 7],
            receivedData[packetCounter + 8]);

        std::cout << numberOfBlocks << "\n";

        // Validate data
        if (numberOfBlocks > 4096)
        {
            std::cerr << "[error@networking/recievedPackets/multiBlockChange] Invalid number of blocks";
            return;
        }

        // Place the blocks
        for (int j = 0; j < numberOfBlocks; j++)
        {
            short tempShort = net::convertShort(receivedData[packetCounter + 11 + (j * 2)], receivedData[packetCounter + 12 + (j * 2)]);

            int tileX = ((tempShort >> 12) & 0xF) + chunkX;
            int tileY = tempShort & 0xFF;
            int tileZ = ((tempShort >> 8) & 0xF) + chunkZ;

            int tileId = receivedData[packetCounter + 11 + (numberOfBlocks * 2) + j];

            // TODO - I moved the tempShort & 0xFF one line above. Not sure if it will work
            chunk::setTile(tileX, tileY, tileZ, tileId);

            // Print the id of the placed block
            // std::cout << static_cast<int>(receivedData[packetCounter + 11 + (numberOfBlocks * 2) + j]) << "\n";
        }

        packetCounter += numberOfBlocks * 4;
    }

    void blockChange()
    {
        std::cout << "Packet : Block Change. Id: ";

        // Read data
        int blockX = net::convertInt(
            receivedData[packetCounter + 1],
            receivedData[packetCounter + 2],
            receivedData[packetCounter + 3],
            receivedData[packetCounter + 4]);

        int8_t blockY = receivedData[packetCounter + 5];

        int blockZ = net::convertInt(
            receivedData[packetCounter + 6],
            receivedData[packetCounter + 7],
            receivedData[packetCounter + 8],
            receivedData[packetCounter + 9]);

        int8_t blockId = receivedData[packetCounter + 10];

        std::cout << blockId << "\n";

        // Set the block
        chunk::setTile(blockX, blockY, blockZ, blockId);
    }

    void chunk()
    {
        std::cout << "[packet] Chunk\n";

        std::vector<uint8_t> decodeVec;
        std::vector<uint8_t> compressedData;
        std::vector<uint8_t> decompressedData;

        // Read data
        int chunkX = net::convertInt(
            receivedData[packetCounter + 1],
            receivedData[packetCounter + 2],
            receivedData[packetCounter + 3],
            receivedData[packetCounter + 4]);
        short chunkY = net::convertShort(
            receivedData[packetCounter + 5],
            receivedData[packetCounter + 6]);
        int chunkZ = net::convertInt(
            receivedData[packetCounter + 7],
            receivedData[packetCounter + 8],
            receivedData[packetCounter + 9],
            receivedData[packetCounter + 10]);

        int8_t sizeX = receivedData[packetCounter + 11];
        int8_t sizeY = receivedData[packetCounter + 12];
        int8_t sizeZ = receivedData[packetCounter + 13];

        int compressedSize = net::convertInt(
            receivedData[packetCounter + 14],
            receivedData[packetCounter + 15],
            receivedData[packetCounter + 16],
            receivedData[packetCounter + 17]);

        // Validate data
        if (chunkY != 0)
        {
            std::cout << "[error] Chunk is at Y: " << chunkY << ", is ";
            printf("%d\n", sizeY);
            std::cout << " blocks tall.\nExpected it to be at Y: 0, and 128 blocks tall\nThis is an unimplemented feature\n";

            throw std::runtime_error("Chunk is vertically misaligned");
            return;
        }

        if (sizeX != 15 || sizeY != 127 || sizeZ != 15)
        {
            std::cout << "[error] Chunk is weirdly sized: " << sizeX << ", " << sizeY << ", " << sizeZ << "\n"
                      << "Expected it to be 15, 127, 15\nThis is an unimplemented feature\n";
            throw std::runtime_error("Chunk is vertically misaligned");
        }

        if (compressedSize > 1000000 || compressedSize < 1)
        {
            std::cerr << "[error@networking/chunkLoading] Chunk size is invalid: " << compressedSize << "\n";
            packetCounter = net::bufferSize;
            return;
        }

        std::cout << "[info@networking/chunkLoading] Started loading chunk of size (bytes): " << compressedSize << "\n";

        packetCounter += 18;
        for (int j = 0; j < compressedSize; j++)
        {
            if (packetCounter >= bytesRead)
            {
                std::cout << "Cut up chunk, reading next packet "
                          << "\n";
                sendPacket::alive();
                if (quit)
                {
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(8));
                packetCounter = 0;
                memset(receivedData, 0, sizeof(receivedData));
                if (!net::receive(bytesRead, receivedData, mySocket))
                {
                    std::cerr << "[error] Connection closed or error occurred while loading chunk\n";
                    quit = 1;
                    return;
                }
            }
            compressedData.push_back(receivedData[packetCounter]);
            packetCounter++;
        }
        packetCounter -= 18;

        if (!net::decompress(compressedData, decompressedData))
        {
            std::cout << "[error@networking/chunkLoading] Broken compressed data\n";
            return;
        }
        for (int cyy = 0; cyy < 8; cyy++)
        {
            int64_t chunkTempX = floor(float(chunkX) / 32.0f);
            int64_t chunkTempY = cyy;
            int64_t chunkTempZ = floor(float(chunkZ) / 32.0f);
            if (chunkMap.count(ChunkCoordinate(chunkTempX, chunkTempY, chunkTempZ)) == 0 &&
                abs(chunkTempX - (myRenderer.camera.X / 32)) <= chunk::renderDistance &&
                abs(chunkTempY - (myRenderer.camera.Y / 32)) <= chunk::renderDistance &&
                abs(chunkTempZ - (myRenderer.camera.Z / 32)) <= chunk::renderDistance)
            {
                Chunk genTempChunk = Chunk(chunkTempX, chunkTempY, chunkTempZ, 0);
                genTempChunk.lock = 1;
                chunk::write(genTempChunk);
                if (!isArrayFilledWithZeroes(genTempChunk.blockData))
                {
                    chunk::updateNeighbours(chunkTempX, chunkTempY, chunkTempZ);
                }
                chunkMap[ChunkCoordinate(chunkTempX, chunkTempY, chunkTempZ)]->lock = 0;
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
                    if (index < int(decompressedData.size()))
                    {
                        chunkMap[chunkCoord]->blockData[((chunkX + cx) % 32 + 32) % 32]
                                                       [((cy) % 32 + 32) % 32]
                                                       [((chunkZ + cz) % 32 + 32) % 32] = int(decompressedData[index]);
                    }
                }
            }
        }
        for (int cyy = 0; cyy < 8; cyy++)
        {
            int64_t chunkTempX = floor(float(chunkX) / 32.0f);
            int64_t chunkTempZ = floor(float(chunkZ) / 32.0f);
            ChunkCoordinate thisChunk = ChunkCoordinate(chunkTempX, cyy, chunkTempZ);
            if (chunkMap.count(thisChunk) > 0)
            {
                chunkMap[thisChunk]->updateMesh();
                chunk::updateNeighbours(chunkTempX, cyy, chunkTempZ);
                chunkMap[thisChunk]->lock = 0;
            }
        }
    }
}