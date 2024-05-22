#include "gtest/gtest.h"

#include "encryption_test.h"

#include "file_handler.h"

using namespace std;
using namespace testing;

//To delete
#include <iostream>

#include "cryptographer.h"

TEST(test_encryption, test_char_array) {
    std::string keyFile = "key.hex";
    // encrypt plain packet into crypto packet
    CryptoPacket cryptoPacket;
    {
        FileHandler fileReader;
        fileReader.open("../../data/Somedata.txt", "rb");
        Packet packetClient;
        fileReader.read(packetClient);
        CryptographerImpl crypto_impl_client;

        // Generate and save the key
        crypto_impl_client.encrypt(packetClient, cryptoPacket);
    }
    // decrypt crypto packet into plain packet
    {
        Packet packetServer;
        FileHandler fileWriter;
        fileWriter.open("recovered.txt", "w");
        CryptographerImpl crypto_impl_server;
        packetServer = crypto_impl_server.decrypt(cryptoPacket);
        fileWriter.write(packetServer);
        fileWriter.close();
    }
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
        CryptographerImpl crypto_impl_client;

        // Generate and save the key
        crypto_impl_client.encrypt(packetClient, cryptoPacket);
    }
    // decrypt crypto packet into plain packet
    {
        Packet packetServer;
        CryptographerImpl crypto_impl_server;
        packetServer = crypto_impl_server.decrypt(cryptoPacket);
        string res(packetServer.payload, packetServer.header.length);
//        memcpy(&res, packetServer.payload , packetServer.header.length);
        EXPECT_TRUE(res == fileName);
    }
}