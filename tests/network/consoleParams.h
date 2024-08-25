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
};

}
#endif  // NETWORKING_CONSOLEPARAMS_H