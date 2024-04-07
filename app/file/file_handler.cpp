/*! \file file_handler.cpp
 * \brief FileHandler class implementation.
 */


#include "file_handler.h"
#include <filesystem>

using namespace std;

FileHandler::FileHandler() {

}
FileHandler::~FileHandler() {
}

void FileHandler::read(Packet& packet) {
    packet.header.length = std::fread(&packet.payload, sizeof(char), DATA_SIZE, get());
}

size_t FileHandler::write(const Packet& packet) {
    return std::fwrite(&packet.payload,  sizeof(char), packet.header.length, get());
}
bool FileHandler::isFileExist(const string& fileName) {
    return std::filesystem::exists(fileName);
}
std::string FileHandler::getUniqueName(const string& fileName) {
    std::string uniqueName;
    auto ts = to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    auto dotPos = fileName.find('.');

    if (dotPos != std::string::npos) {
        uniqueName = fileName.substr(0, dotPos) + ts + fileName.substr(dotPos, fileName.size());
    } else {
        uniqueName = fileName + ts;
    }

    return uniqueName;
}
