#pragma once

#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

#include "networkingFunctions.h"
#include "sendPacket.h"
#include "handlePacket.h"
#include "../chunk.h"

enum GamePackets
{
    packet_Heartbeat,
    packet_Login,
    packet_Handshake,
    packet_Chat,
    packet_Time,
    packet_EntityInventory,
    packet_SpawnPoint,
    packet_ClickEntity,
    packet_Health,
    packet_Respawn,
    packet_PlayerFlying,
    packet_PlayerPosition,
    packet_PlayerLook,
    packet_PlayerPositionAndLook,
    packet_Mine,
    packet_PlaceBlock
};

int mySocket = -1;
bool loggedIn = 0;

extern bool quit;

class Entity;
extern Entity *commonEntities[1024];

void runMultiplayer()
{
    uint8_t receivedData[net::bufferSize];
    int bytesRead = 0;
    if (!net::receive(bytesRead, receivedData, mySocket))
    {
        std::cerr << "[error] Connection closed or error occurred\n";
        quit = 1;
        return;
    }
    sendPacket::alive();
    sendPacket::position();

    for (int packetCounter = 0; packetCounter < net::bufferSize; packetCounter++)
    {
        uint16_t length = 0;
        uint8_t currentPacket = receivedData[packetCounter];

        int compressedSize = 0;
        int16_t tempShort = 0;

        uint8_t tempbytes[8];

        switch (currentPacket)
        {
        case packet_Heartbeat:
            break;
        case packet_Login:
            handlePacket::login();
            break;
        case packet_Chat:
            handlePacket::chat();
            break;
        case packet_PlayerPositionAndLook:
            handlePacket::playerPositionAndLook();
            break;
        case 0x14:
            std::cout << "Packet : Spawn Player\n";
            // TODO
            length = (static_cast<uint16_t>(receivedData[packetCounter + 1 + 4]) << 8) |
                     static_cast<uint16_t>(receivedData[packetCounter + 2 + 4]);
            packetCounter += length;
            break;
        case 0x18:
            std::cerr << "Broken Packet : Spawn Mob\n";
            break;
        case 0x28:
            std::cerr << "Broken Packet : Mob Metadata\n";
            break;
        case 0x32: // Pre-Chunk
            handlePacket::preChunk();
            break;
        case 0x33:
            handlePacket::chunk();
            break;
        case 0x34:
            handlePacket::multiBlockChange();
            break;
        case 0x35:
            handlePacket::blockChange();
            break;
        case 0x3c:
            std::cerr << "Broken Packet : Explosion\n";
            break;
        case 0x64:
            std::cerr << "Broken Packet : Open Inventory Window\n";
            break;
        case 0x68:
            std::cerr << "Broken Packet : Inventory Window Payload\n";
            // TODO
            compressedSize = net::convertShort(
                receivedData[packetCounter + 9],
                receivedData[packetCounter + 10]);
            for (int j = 0; j < compressedSize; j++)
            {
                tempShort = net::convertShort(
                    receivedData[packetCounter + 3],
                    receivedData[packetCounter + 4]);
                packetCounter += 1;
                if (tempShort == -1)
                {
                    packetCounter += 2;
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
        }

        // If program quits, exit this thread
        if (quit)
        {
            return;
        }

        // Move on to the next packet
        try
        {
            packetCounter += net::packetSizes.at(currentPacket) - 1;
        }
        catch (std::exception &e)
        {
            // Packet is invalid and nonexistent
            std::cout << "Invalid Packet: 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(currentPacket) << std::endl;
            // Skip the current buffer of packets. Read the next one
            packetCounter = net::bufferSize;
        }
    }
    memset(receivedData, 0, sizeof(receivedData));
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
}