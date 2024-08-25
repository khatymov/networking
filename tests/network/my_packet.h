//
// Created by Renat_Khatymov on 8/6/2024.
//

#ifndef NETWORKING_MY_PACKET_H
#define NETWORKING_MY_PACKET_H

#include "pch.h"

// should be at least 64 bytes, because of hash size
#define PACKET_DATA_SIZE 65535

namespace network {
struct Header {
    enum class Type : uint32_t {
        Ack,      // Confirm that packet received
        Nack,      // Notify that packet didn't receive
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

} // namespace network
#endif  // NETWORKING_MY_PACKET_H
