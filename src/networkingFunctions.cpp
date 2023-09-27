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
#include <string>
#include <cstdint>
#include <vector>
#include <iostream>

#include "../include/networking/networkingFunctions.h"

void net::appendString16(const std::string &input, std::vector<uint8_t> &data)
{
    std::vector<uint8_t> ucs2Data;

    // Calculate the length of the input string in characters
    uint16_t length = static_cast<uint16_t>(input.length());

    // Add the length as a big-endian short to the UCS-2 data
    ucs2Data.push_back(static_cast<uint8_t>((length >> 8) & 0xFF)); // High byte
    ucs2Data.push_back(static_cast<uint8_t>(length & 0xFF));        // Low byte

    // Convert each character to UCS-2 and add to the UCS-2 data
    for (char c : input)
    {
        ucs2Data.push_back(0);                       // High byte (0 for UCS-2)
        ucs2Data.push_back(static_cast<uint8_t>(c)); // Low byte (ASCII value of character)
    }

    // Append the additional data
    data.insert(data.end(), ucs2Data.begin(), ucs2Data.end());
}

std::string net::convertFromUCS2(std::vector<uint8_t> &ucs2Data)
{
    if (ucs2Data.size() < 2 || (ucs2Data.size() - 2) % 2 != 0)
    {
        if (ucs2Data.size() < 2)
        {
            ucs2Data.push_back(0x00);
        }
        ucs2Data.push_back(0x00);
    }

    uint16_t length = (static_cast<uint16_t>(ucs2Data[0]) << 8) | static_cast<uint16_t>(ucs2Data[1]);

    std::string output;
    for (size_t i = 2; i < ucs2Data.size(); i += 2)
    {
        // char c = static_cast<char>((static_cast<uint16_t>(ucs2Data[i]) << 8) | static_cast<uint16_t>(ucs2Data[i + 1]));
        char c = static_cast<char>(ucs2Data[i + 1]);
        output += c;
    }

    return output;
}

int32_t net::convertInt(int8_t byte1, int8_t byte2, int8_t byte3, int8_t byte4)
{
    int32_t littleEndianInt = 0;

    littleEndianInt |= static_cast<uint8_t>(byte1) << 24;
    littleEndianInt |= static_cast<uint8_t>(byte2) << 16;
    littleEndianInt |= static_cast<uint8_t>(byte3) << 8;
    littleEndianInt |= static_cast<uint8_t>(byte4);

    return littleEndianInt;
}

int16_t net::convertShort(int8_t byte1, int8_t byte2)
{
    int16_t littleEndianInt = 0;

    littleEndianInt |= static_cast<uint8_t>(byte1) << 8;
    littleEndianInt |= static_cast<uint8_t>(byte2);

    return littleEndianInt;
}

bool net::decompress(std::vector<uint8_t> &compressedData, std::vector<uint8_t> &decompressedData)
{
    // Initialize zlib stream
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = compressedData.size();
    stream.next_in = compressedData.data();

    // Initialize inflate process
    if (inflateInit(&stream) != Z_OK)
    {
        std::cerr << "inflateInit failed" << std::endl;
        return 0;
    }

    while (stream.avail_in > 0)
    {
        unsigned char outBuffer[81920];
        stream.avail_out = sizeof(outBuffer);
        stream.next_out = outBuffer;

        int ret = inflate(&stream, Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT || ret == Z_DATA_ERROR)
        {
            std::cerr << "inflate error" << std::endl;
            inflateEnd(&stream);
            return 0;
        }

        int bytesRead = sizeof(outBuffer) - stream.avail_out;
        if (bytesRead > 0)
        {
            decompressedData.insert(decompressedData.end(), outBuffer, outBuffer + bytesRead);
        }

        if (ret == Z_STREAM_END)
        {
            return 1;
        }
    }

    // Clean up
    inflateEnd(&stream);
    return 1;
}

double net::convertDouble(const uint8_t bytes[8])
{
    static_assert(sizeof(double) == 8, "Size of double must be 8 bytes");
    uint64_t longValue = 0;
    for (int i = 0; i < 8; ++i)
    {
        longValue |= static_cast<uint64_t>(bytes[i]) << ((7 - i) * 8);
    }
    double result;
    std::memcpy(&result, &longValue, sizeof(double));
    return result;
}

// Appends a short to the vector
void net::appendShort(int16_t value, std::vector<uint8_t> &vec)
{
    vec.push_back(static_cast<uint8_t>((value >> 8) & 0xFF)); // High byte
    vec.push_back(static_cast<uint8_t>(value & 0xFF));        // Low byte
}

// Appends an int to the vector
void net::appendInt(int32_t value, std::vector<uint8_t> &vec)
{
    for (int i = 3; i >= 0; --i)
    {
        vec.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
    }
}

// Appends a long to the vector
void net::appendLong(int64_t value, std::vector<uint8_t> &vec)
{
    for (int i = 7; i >= 0; --i)
    {
        vec.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
    }
}

// Appends a float to the vector
void net::appendFloat(float value, std::vector<uint8_t> &vec)
{
    static_assert(sizeof(float) == 4, "Size of float must be 4 bytes");
    uint32_t intValue;
    std::memcpy(&intValue, &value, sizeof(float));
    appendInt(intValue, vec);
}

// Appends a double to the vector
void net::appendDouble(double value, std::vector<uint8_t> &vec)
{
    static_assert(sizeof(double) == 8, "Size of double must be 8 bytes");
    uint64_t longValue;
    std::memcpy(&longValue, &value, sizeof(double));
    appendLong(longValue, vec);
}

// Appends a string8 to the vector
void net::appendString8(const std::string &value, std::vector<uint8_t> &vec)
{
    uint16_t length = static_cast<uint16_t>(value.length());
    appendShort(length, vec);
    for (char c : value)
    {
        vec.push_back(static_cast<uint8_t>(c));
    }
}


int net::init()
{
    #ifdef _WIN32 // Windows
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            std::cerr << "WSAStartup failed" << std::endl;
            return -1;
        }
    #endif
    // Create a socket
    int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDescriptor == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
    }
    #ifdef _WIN32
        // Windows setup
        int timeoutMillis = 10000; // 10 seconds in milliseconds
        setsockopt(socketDescriptor, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeoutMillis, sizeof(timeoutMillis));
    #else
        // Unix-like setup
        struct timeval timeout;
        timeout.tv_sec = 10; // 10 seconds
        timeout.tv_usec = 0;
        setsockopt(socketDescriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    #endif
    return socketDescriptor;
}


int net::connect(const std::string &ipAddress, const int port, int socketDescriptor)
{
    // Configure the server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);                         // Replace with actual port
    serverAddress.sin_addr.s_addr = inet_addr(ipAddress.c_str()); // Replace with actual IP

    // Connect to the server
    if (connect(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cerr << "Failed to connect to server" << std::endl;
        #ifdef _WIN32
            closesocket(socketDescriptor);
        #else
            close(socketDescriptor);
        #endif
        return 1;
    }
    return 0;
}


void net::sendpacket(std::vector<uint8_t> &packet, int socketDescriptor)
{
    send(socketDescriptor, reinterpret_cast<char *>(packet.data()), packet.size(), 0);
}


bool net::receive(int &bytesRead, uint8_t (&data)[bufferSize], int socketDescriptor)
{
    memset(data, 0, sizeof(data));
    bytesRead = recv(socketDescriptor, data, sizeof(data), 0);
    if (bytesRead <= 0)
    {
        std::cerr << "Connection closed or error occurred" << std::endl;
        return 0;
    }
    return 1;
}


void net::shutdown(int socketDescriptor)
{
    #ifdef _WIN32
        closesocket(socketDescriptor);
        WSACleanup();
    #else
        close(socketDescriptor);
    #endif
}