#include <vector>
#include <iostream>
#include <thread>
#include <chrono>

#include "networkingFunctions.h"
#include "entities.h"
#include "chunk.h"

int mySocket = -1;
bool loggedIn = 0;

extern bool quit;
extern float cameraX, cameraY, cameraZ;

extern Entity *commonEntities[1024];

namespace sendPacket
{
    void alive()
    {
        std::vector<uint8_t> aliveData = {0x00};
        net::sendpacket(aliveData, mySocket);
    }

    void login()
    {
        std::vector<uint8_t> loginData = {0x01, 0x00, 0x00, 0x00, 0x0e};
        net::appendString16("mrmayman", loginData);
        loginData.insert(loginData.end(), {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
        net::sendpacket(loginData, mySocket);
    }

    void position()
    {
        if (loggedIn)
        {
            std::vector<uint8_t> positionData = {0x0D};
            net::appendDouble(commonEntities[0]->x, positionData);
            net::appendDouble(commonEntities[0]->y, positionData);
            net::appendDouble(commonEntities[0]->y + 1.64, positionData);
            net::appendDouble(commonEntities[0]->z, positionData);
            net::appendFloat(0.0, positionData);
            net::appendFloat(0.0, positionData);
            positionData.push_back(0x0);
            net::sendpacket(positionData, mySocket);
            positionData = {0x0D};
        }
    }
}

void runMultiplayer()
{
    uint8_t receivedData[net::bufferSize];
    int bytesRead = 0;
    memset(receivedData, 0, sizeof(receivedData));
    if (!net::receive(bytesRead, receivedData, mySocket))
    {
        std::cerr << "[error] Connection closed or error occurred\n";
        quit = 1;
        return;
    }
    sendPacket::alive();
    sendPacket::position();

    for (int i = 0; i < net::bufferSize; i++)
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
            std::cout << "Packet : Login\n";
            if (!loggedIn)
            {
                if (receivedData[i + 4] != 0x2d)
                {
                    std::cerr << "[error] Account authentication required! Not supported.\nTry joining a cracked server\n";
                    quit = 1;
                    return;
                }
                sendPacket::login();
            }
            loggedIn = 1;
            i += 2;
            break;
        case 0x03: // Chat
            std::cout << "Packet : Chat\n";
            length = (static_cast<uint16_t>(receivedData[i + 1]) << 8) |
                     static_cast<uint16_t>(receivedData[i + 2]);
            if (length > 254)
            {
                break;
            }
            for (int j = 1; j < (length * 2) && j < net::bufferSize; j += 2)
            {
                stringbuffer += receivedData[i + 3 + j];
            }
            std::cout << "[chat] " << stringbuffer << "\n";
            i += length;
            break;
        case 0x0d:
            std::cout << "Packet : Position\n";
            for (int j = 0; j < 8; j++)
            {
                tempbytes[j] = receivedData[i + 1 + j];
            }
            doubleX = net::convertDouble(tempbytes);
            for (int j = 0; j < 8; j++)
            {
                tempbytes[j] = receivedData[i + 9 + j];
            }
            doubleW = net::convertDouble(tempbytes);
            for (int j = 0; j < 8; j++)
            {
                tempbytes[j] = receivedData[i + 17 + j];
            }
            doubleY = net::convertDouble(tempbytes);
            for (int j = 0; j < 8; j++)
            {
                tempbytes[j] = receivedData[i + 25 + j];
            }
            doubleZ = net::convertDouble(tempbytes);
            std::cout << doubleX << ", " << doubleY << ", " << doubleZ << "\n";
            if (abs(doubleX) > 32000000 || abs(doubleZ) > 32000000 || (doubleW - doubleY) > 1.66 || (doubleW - doubleY) < 0.1)
            {
                std::cerr << "Illegal Stance\n";
                break;
            }
            commonEntities[0]->x = float(doubleX);
            commonEntities[0]->y = float(doubleY);
            commonEntities[0]->z = float(doubleZ);

            sendPacket::position();
            break;
        case 0x14:
            std::cout << "Packet : Spawn Player\n";
            length = (static_cast<uint16_t>(receivedData[i + 1 + 4]) << 8) |
                     static_cast<uint16_t>(receivedData[i + 2 + 4]);
            i += length;
            break;
        case 0x18:
            std::cerr << "Broken Packet : Spawn Mob\n";
            break;
        case 0x28:
            std::cerr << "Broken Packet : Mob Metadata\n";
            break;
        case 0x32: // Pre-Chunk
            std::cout << "Packet : Pre-Chunk\n";
            if (!receivedData[i + 9])
            {
                break;
            }
            chunkX = net::convertInt(
                receivedData[i + 1],
                receivedData[i + 2],
                receivedData[i + 3],
                receivedData[i + 4]);
            chunkZ = net::convertInt(
                receivedData[i + 5],
                receivedData[i + 6],
                receivedData[i + 7],
                receivedData[i + 8]);
            break;
        case 0x33:
            std::cout << "Packet : Chunk\n";
            networkChunks::loadChunk();
            break;
        case 0x34:
            compressedSize = net::convertShort(
                receivedData[i + 9],
                receivedData[i + 10]);
            std::cout << "Packet : Multi Block Change " << static_cast<int>(compressedSize) << "\n";
            chunkX = net::convertInt(
                receivedData[i + 1],
                receivedData[i + 2],
                receivedData[i + 3],
                receivedData[i + 4]);
            chunkZ = net::convertInt(
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
                tempShort = net::convertShort(receivedData[i + 11 + (j * 2)], receivedData[i + 12 + (j * 2)]);
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
            chunkX = net::convertInt(
                receivedData[i + 1],
                receivedData[i + 2],
                receivedData[i + 3],
                receivedData[i + 4]);
            chunkZ = net::convertInt(
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
            compressedSize = net::convertShort(
                receivedData[i + 9],
                receivedData[i + 10]);
            for (int j = 0; j < compressedSize; j++)
            {
                chunkX = net::convertShort(
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
            i += net::packetSizes.at(currentPacket) - 1;
            // std::cout << "Hex value: 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(currentPacket) << std::endl;
        }
        catch (std::exception &e)
        {
            std::cout << "Invalid Packet: 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(currentPacket) << std::endl;
            i = net::bufferSize;
        }
    }
    memset(receivedData, 0, sizeof(receivedData));
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
}