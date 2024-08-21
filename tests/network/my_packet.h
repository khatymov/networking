//
// Created by Renat_Khatymov on 8/6/2024.
//

#ifndef NETWORKING_MY_PACKET_H
#define NETWORKING_MY_PACKET_H

#include <array>
#include <cstdint>
#include <cstdio>

#include <cryptopp/aes.h>
#include <cryptopp/osrng.h>
//#include <cryptopp/base64.h>
//#include <cryptopp/cryptlib.h>
//#include <cryptopp/files.h>
//#include <cryptopp/filters.h>
//#include <cryptopp/hex.h>
#include <cryptopp/modes.h>

// should be at least 64 bytes, because of hash size
#define PACKET_DATA_SIZE 65535

struct Header {
    enum class Type : uint32_t {
        Ack,      // Confirm that packet received
        FileName, // name of a transferred file
        FileData, // File data
        CryptoData, // Encrypted data
        Hash,     // Hash of a file to compare server and files hash
        Exit      // Notify - we are done
    };

    Type type;
    size_t length = 0;
};


template <typename T = char>
struct MyPacket {
    Header header;
    static constexpr std::size_t packetDataSize = PACKET_DATA_SIZE;
    std::array<T, packetDataSize> data;
};

template<>
struct MyPacket<CryptoPP::byte> {
    Header header;
    static constexpr std::size_t packetDataSize = PACKET_DATA_SIZE + CryptoPP::AES::BLOCKSIZE;
    std::array<CryptoPP::byte, packetDataSize> data;
};


#endif  // NETWORKING_MY_PACKET_H
