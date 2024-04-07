/*! \file file_interface.cpp
 * \brief FileInterface class implementation.
 */


#include "file_interface.h"

using namespace std;

void FileInterface::open(const string& name, const string& mode) {
    m_file = std::fopen(name.c_str(), mode.c_str());
}

FileInterface::~FileInterface() {
    std::fclose(m_file);
    m_file = nullptr;
}
std::FILE* FileInterface::get() {
    return m_file;
}
