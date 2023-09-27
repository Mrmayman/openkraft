#pragma once

#ifdef _WIN32 // Windows
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else // Unix-like systems
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#include <zlib.h>

#include <unordered_map>
#include <cstring>
#include <cstdint>
#include <vector>

namespace net
{
    const std::unordered_map<uint8_t, int> packetSizes = {
        {0x00, 1},
        {0x01, 16},
        {0x02, 3},
        {0x03, 3},
        {0x04, 9},
        {0x05, 11},
        {0x06, 13},
        {0x07, 10},
        {0x08, 3},
        {0x09, 2},
        {0x0a, 2},
        {0x0b, 34},
        {0x0c, 10},
        {0x0d, 42},
        {0x0e, 12},
        {0x0f, 13},
        {0x10, 3},
        {0x11, 15},
        {0x12, 6},
        {0x13, 6},
        {0x14, 23},
        {0x15, 25},
        {0x16, 9},
        {0x17, 22},
        {0x18, 20},
        {0x19, 23},
        {0x1b, 15},
        {0x1c, 11},
        {0x1d, 5},
        {0x1e, 5},
        {0x1f, 8},
        {0x20, 7},
        {0x21, 10},
        {0x22, 19},
        {0x26, 6},
        {0x27, 9},
        {0x28, 5},
        {0x32, 10},
        {0x33, 18},
        {0x34, 11},
        {0x35, 12},
        {0x36, 13},
        {0x3c, 33},
        {0x3d, 18},
        {0x46, 49},
        {0x47, 18},
        {0x64, 6},
        {0x65, 2},
        {0x66, 10},
        {0x67, 6},
        {0x68, 4},
        {0x69, 6},
        {0x6a, 5},
        {0x82, 11},
        {0x83, 6},
        {0xc8, 6},
        {0xff, 1}};

    const int bufferSize = 4096;

    void appendString16(const std::string &input, std::vector<uint8_t> &data);

    std::string convertFromUCS2(std::vector<uint8_t> &ucs2Data);

    int32_t convertInt(int8_t byte1, int8_t byte2, int8_t byte3, int8_t byte4);

    int16_t convertShort(int8_t byte1, int8_t byte2);

    bool decompress(std::vector<uint8_t> &compressedData, std::vector<uint8_t> &decompressedData);

    double convertDouble(const uint8_t bytes[8]);

    // Appends a short to the vector
    void appendShort(int16_t value, std::vector<uint8_t> &vec);

    // Appends an int to the vector
    void appendInt(int32_t value, std::vector<uint8_t> &vec);

    // Appends a long to the vector
    void appendLong(int64_t value, std::vector<uint8_t> &vec);

    // Appends a float to the vector
    void appendFloat(float value, std::vector<uint8_t> &vec);

    // Appends a double to the vector
    void appendDouble(double value, std::vector<uint8_t> &vec);

    // Appends a string8 to the vector
    void appendString8(const std::string &value, std::vector<uint8_t> &vec);

    int init();

    int connect(const std::string &ipAddress, const int port, int socketDescriptor);

    void sendpacket(std::vector<uint8_t> &packet, int socketDescriptor);

    bool receive(int &bytesRead, uint8_t (&data)[bufferSize], int socketDescriptor);

    void shutdown(int socketDescriptor);

}