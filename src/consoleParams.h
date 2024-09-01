//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_CONSOLEPARAMS_H
#define NETWORKING_CONSOLEPARAMS_H

#include "pch.h"

namespace network {

struct ConsoleParams{
    std::string_view ip;
    uint port;
    std::string_view targetFile;
    bool isClient() const {
        return !targetFile.empty();
    }

    ConsoleParams() = default;

    ConsoleParams(std::string_view ip, std::string_view port, std::string_view targetFile = {})
        :ip(ip), port(std::stoul(std::string(port))), targetFile(targetFile) {}

};

}
#endif  // NETWORKING_CONSOLEPARAMS_H
