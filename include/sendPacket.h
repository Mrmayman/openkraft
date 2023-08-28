#include <vector>
#include <cstdint>

#include "networkingFunctions.h"
#include "entities.h"

extern int mySocket;
extern bool loggedIn;
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
        if (!loggedIn)
        {
            return;
        }
        std::vector<uint8_t> positionData = {0x0D};
        net::appendDouble(commonEntities[0]->x, positionData);
        net::appendDouble(commonEntities[0]->y, positionData);
        net::appendDouble(commonEntities[0]->y + 1.64, positionData);
        net::appendDouble(commonEntities[0]->z, positionData);
        net::appendFloat(0.0, positionData);
        net::appendFloat(0.0, positionData);
        positionData.push_back(0x0);
        net::sendpacket(positionData, mySocket);
    }
}