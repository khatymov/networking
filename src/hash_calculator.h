//
// Created by Renat_Khatymov on 9/1/2024.
//

#ifndef NETWORKING_HASHCALCULATOR_H
#define NETWORKING_HASHCALCULATOR_H

#include "pch.h"
namespace network {

class HashCalculator {
public:
    static std::string getFileHash(const std::string& filename);
};

} // namespace network

#endif  // NETWORKING_HASHCALCULATOR_H
