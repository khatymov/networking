/*! \file file_handler.cpp
 * \brief FileHandler class implementation.
 */


#include "file_handler.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <openssl/sha.h>
#include <sstream>

using namespace std;


void FileHandler::read(Packet& packet) {
    packet.header.type = Packet::Type::FileData;
    packet.header.length = std::fread(&packet.payload, sizeof(char), DATA_SIZE, get());
}

size_t FileHandler::write(Packet& packet) {
    packet.header.type = Packet::Type::FileData;
    return std::fwrite(&packet.payload,  sizeof(char), packet.header.length, get());
}

bool FileHandler::isFileExist(const string& fileName) {
    return std::filesystem::exists(fileName);
}

std::string FileHandler::getUniqueName(const string& fileName) {
    std::string uniqueName;
    auto ts = to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    auto dotPos = fileName.find('.');

    if (dotPos != std::string::npos) {
        uniqueName = fileName.substr(0, dotPos) + ts + fileName.substr(dotPos, fileName.size());
    } else {
        uniqueName = fileName + ts;
    }

    return uniqueName;
}

std::string FileHandler::getFileHash(const std::string& filename) {
    std::ifstream file(filename, std::ifstream::binary);

    if (!file) {
        std::cerr << "File " << filename << " cannot be opened." << std::endl;
        return "";
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    const size_t bufferSize = 1 << 12; // 4096 bytes
    char buffer[bufferSize];
    while (file.good()) {
        file.read(buffer, bufferSize);
        SHA256_Update(&sha256, buffer, file.gcount());
    }

    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return ss.str();
}
