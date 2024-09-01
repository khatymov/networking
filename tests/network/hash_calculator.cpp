//
// Created by Renat_Khatymov on 9/1/2024.
//

#include "hash_calculator.h"

namespace network {

std::string network::HashCalculator::getFileHash(const std::string& filename) {
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

} // namespace network