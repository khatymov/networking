/*! \file packet.h
 * \brief Packet struct is a template struct that has header and body
 *
 */


#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <assert.h>

#include <cryptopp/base64.h>
#include <cryptopp/osrng.h>

//should be at least 64 bytes, because of hash size
#define DATA_SIZE 65535

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
};
#include "iostream"
struct CryptoPacket {
    Packet::Header header{};
    std::array<CryptoPP::byte, DATA_SIZE + CryptoPP::AES::BLOCKSIZE> payload;
//    ~CryptoPacket() {
//        std::cout << "~CryptoPacket" << std::endl;
//    }
};

#include <queue>

//TODO Should work with usual packets as well
//class PacketRotator {
//public:
//
//    enum class Stage: int {
//        NETWORK,
//        CRYPTO,
//        FILE
//    };
//
//    PacketRotator() {
//        networkStageQueue_.push(&cryptoPacket[0]);
//        networkStageQueue_.push(&cryptoPacket[1]);
//    }
//
//    CryptoPacket* getPacket(const Stage& mode) {
//        CryptoPacket* packet = nullptr;
//        auto* const currentQueue = getPacketFromQueue_(mode);
//        std::lock_guard<std::mutex> lockGuard(_mutex);
//        if (not currentQueue->empty()) {
//            packet = currentQueue->front();
//            currentQueue->pop();
//        }
//        return packet;
//    }
//
//    //set packet for the next mode
//    void setPacket(const Stage& mode, CryptoPacket* packet) {
//        auto* const nextQueue = getPacketFromQueue_(mode);
//        std::lock_guard<std::mutex> lockGuard(_mutex);
//        nextQueue->push(packet);
//    }
//
//protected:
//
//    std::queue<CryptoPacket*>* getPacketFromQueue_(const Stage& mode) {
//        if (mode == Stage::NETWORK) {
//            return &networkStageQueue_;
//        } else if (mode == Stage::CRYPTO) {
//            return &cryptoStageQueue_;
//            //        } else if (mode == Stage::FILE) {
//            //            return &cryptoStageQueue_;
//        } else {
//            return nullptr;
//        }
//    }
//
//private:
//    std::queue<CryptoPacket*> networkStageQueue_;
//    std::queue<CryptoPacket*> cryptoStageQueue_;
//    std::queue<Packet*> fileStageQueue_;
//    CryptoPacket cryptoPacket[2];
//
//    //! \brief thread synchronization primitives
//    //! // to prevent from access to queue
//    std::mutex _mutex;
//};
//
//extern PacketRotator packetRotator;

