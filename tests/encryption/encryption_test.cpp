#include "gtest/gtest.h"

#include "encryption_test.h"

#include "file_handler.h"

using namespace std;
using namespace testing;

//To delete
#include <iostream>

#include "cryptographer.h"

TEST(test_encryption, test_char_array) {

    // Client side
    Packet packetClient;
    FileHandler fileReader;
    fileReader.open("../../data/Somedata.txt", "rb");
    CryptoPacket cryptoPacket;
    Cryptographer cryptoClient;
    if (!cryptoClient.setKey("myKey")) {
        std::cerr << "Set a key for encryption" << std::endl;
        exit(1);
    }
    // Server side
//    Packet packetServer;
    FileHandler fileWriter;
    fileWriter.open("recovered.txt", "w");
    Cryptographer cryptoServer;
    if (!cryptoServer.setKey("myKey")) {
        std::cerr << "Set a key for encryption" << std::endl;
        exit(1);
    }
    Packet packetServer;
    do {
        fileReader.read(packetClient);
        cryptoClient.encrypt(packetClient, cryptoPacket);
        cryptoServer.decrypt(cryptoPacket, packetServer);
        fileWriter.write(packetServer);
        if (packetClient.header.length == 0) {
            break;
        }

    } while (true);
}

TEST(test_encryption, test_file_name) {
    std::string fileName("tmp.txt");
    // encrypt plain packet into crypto packet
    CryptoPacket cryptoPacket;
    {
        Packet packetClient;
        packetClient.header.type = Packet::Type::FileName;
        packetClient.header.length = fileName.size();
        memcpy(packetClient.payload, fileName.c_str(), fileName.size());
        Cryptographer crypto_impl_client;

        if (!crypto_impl_client.setKey("myKey")) {
            std::cerr << "Set a key for encryption" << std::endl;
            exit(1);
        }
        // Generate and save the key
        crypto_impl_client.encrypt(packetClient, cryptoPacket);
    }
    // decrypt crypto packet into plain packet
    {
        Packet packetServer;
        Cryptographer crypto_impl_server;
        if (!crypto_impl_server.setKey("myKey")) {
            std::cerr << "Set a key for encryption" << std::endl;
            exit(1);
        }
        crypto_impl_server.decrypt(cryptoPacket, packetServer);
        string res(packetServer.payload, packetServer.header.length);
        EXPECT_TRUE(res == fileName);
    }
}
