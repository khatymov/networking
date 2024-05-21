/*! \file packet.h
 * \brief Packet struct is a template struct that has header and body
 *
 */


#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <assert.h>

//TODO check what is the perfect size for a socket
#define DATA_SIZE 32
//#define DATA_SIZE 8



/*! \struct Packet
 * \brief Holds a header and the data
 */
// maybe make a template?
struct Packet {
    enum class Type : uint32_t {
        Ack,      // Confirm that packet received
        FileName, // name of a transferred file
        FileData, // File data
        Hash,     // Hash of a file to compare server and files hash
        Exit      // Notify - we are done
    };

    struct Header {
        Type type{};
        size_t length = 0;
    };

    Header header{};
    char payload[DATA_SIZE];

    //TODO: add method to check header
};

#include <cryptopp/base64.h>
#include <cryptopp/osrng.h>

struct CryptoPacket {
    enum class Type : uint32_t {
        Ack,      // Confirm that packet received
        FileName, // name of a transferred file
        FileData, // File data
        Hash,     // Hash of a file to compare server and files hash
        Exit      // Notify - we are done
    };

    struct Header {
        Type type{};
        size_t length = 0;
    };

    Header header{};
    std::array<CryptoPP::byte, DATA_SIZE + CryptoPP::AES::BLOCKSIZE> payload;
    //TODO: add method to check header
};