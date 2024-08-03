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

struct IPacket {
    virtual ~IPacket() = default;
//    data;
};

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
    ~CryptoPacket() {
        std::cout << "~CryptoPacket" << std::endl;
    }
};

#include <queue>

//TODO Should work with usual packets as well
class PacketRotator {
public:

    enum class Mode: int {
        NETWORK,
        CRYPTO,
        FILE
    };

    PacketRotator() {
        networkStageQueue_.push(&cryptoPacket[0]);
        networkStageQueue_.push(&cryptoPacket[1]);
    }

    CryptoPacket* getPacket(const Mode& mode) {
        CryptoPacket* packet = nullptr;
        auto* const currentQueue = getPacketFromQueue_(mode);
        std::lock_guard<std::mutex> lockGuard(_mutex);
        if (not currentQueue->empty()) {
            packet = currentQueue->front();
            currentQueue->pop();
        }
        return packet;
    }

    //set packet for the next mode
    void setPacket(const Mode& mode, CryptoPacket* packet) {
        auto* const nextQueue = getPacketFromQueue_(mode);
        std::lock_guard<std::mutex> lockGuard(_mutex);
        nextQueue->push(packet);
    }

protected:

    std::queue<CryptoPacket*>* getPacketFromQueue_(const Mode& mode) {
        if (mode == Mode::NETWORK) {
            return &networkStageQueue_;
        } else if (mode == Mode::CRYPTO) {
            return &cryptoStageQueue_;
            //        } else if (mode == Mode::FILE) {
            //            return &cryptoStageQueue_;
        } else {
            return nullptr;
        }
    }

private:
    std::queue<CryptoPacket*> networkStageQueue_;
    std::queue<CryptoPacket*> cryptoStageQueue_;
    std::queue<Packet*> fileStageQueue_;
    CryptoPacket cryptoPacket[2];

    //! \brief thread synchronization primitives
    //! // to prevent from access to queue
    std::mutex _mutex;
};

extern PacketRotator packetRotator;
1. я пишу исключительно интерфейсы! не логику. ЧТО Я ОЖИДАЮ ОТ КЛАССА
2. я пишу в расчете на то, что я переиспользую этот класс
3. опиши все параметры своих методов + граничные случаи своих параметров.
        что сделает set если пришел null,

    https://github.com/zproksi/bpatch/blob/master/srcbpatch/streamreplacer.h#L34
             
https://github.com/zproksi/bpatch/blob/master/srcbpatch/actionscollection.cpp#L279
class DataFlow {
public:
    //
    как ждать, сколько ждать, вне зависимости от типа пакета
    std::unique_ptr<IPacket> getPacket() {
        return std::unique_ptr<IPacket>;
    }

    //set packet for the next mode
    закидывает в очередь
    void setPacket(std::unique_ptr<IPacket>&& packet) {
    }

private:
//    std::unique_ptr<IPacket> targetOfCryptoPacket;
    std::queue<std::unique_ptr<IPacket>> queue;
    std::mutex _mutex;
};
