#include <iostream>
#include <cstdio>
#include <exception>

#include "networkingFunctions.h"

namespace networkChunks
{

    void loadChunk()
    {
        int chunkX = net::convertInt(
            receivedData[i + 1],
            receivedData[i + 2],
            receivedData[i + 3],
            receivedData[i + 4]);
        short chunkY = net::convertShort(
            receivedData[i + 5],
            receivedData[i + 6]);
        int chunkZ = net::convertInt(
            receivedData[i + 7],
            receivedData[i + 8],
            receivedData[i + 9],
            receivedData[i + 10]);

        if (chunkY != 0)
        {
            std::cout << chunkY << ", ";
            printf("%d\n", receivedData[i + 12]);
            throw std::runtime_error("Chunk is vertically misaligned");
            return;
        }

        compressedSize = net::convertInt(
            receivedData[i + 14],
            receivedData[i + 15],
            receivedData[i + 16],
            receivedData[i + 17]);

        std::cout << "started chunk " << compressedSize << "\n";
        if (compressedSize > 1000000)
        {
            std::cerr << "invalid size chunk " << compressedSize << "\n";
            i = net::bufferSize;
            return;
        }
        i += 18;
        for (int j = 0; j < compressedSize; j++)
        {
            if (i >= bytesRead)
            {
                std::cout << "Cut up chunk, reading next packet "
                          << "\n";
                sendPacket::alive();
                if (quit)
                {
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(8));
                i = 0;
                memset(receivedData, 0, sizeof(receivedData));
                if (!net::receive(bytesRead, receivedData, mySocket))
                {
                    std::cerr << "[error] Connection closed or error occurred while loading chunk\n";
                    quit = 1;
                    return;
                }
            }
            compressedData.push_back(receivedData[i]);
            i++;
        }
        i -= 18;

        if (!net::decompress(compressedData, decompressedData))
        {
            std::cout << "[error] Broken chunk data. Could not be decompressed.\n";
            return;
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
                    if (index < int(decompressedData.size()))
                    {
                        chunkMap[chunkCoord].blockData[((chunkX + cx) % 32 + 32) % 32]
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
            chunkMap[thisChunk].updateMesh();
            chunk::updateNeighbours(chunkTempX, cyy, chunkTempZ);
            chunkMap[thisChunk].lock = 0;
        }
    }

}