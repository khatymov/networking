/*! \file file_interface.cpp
 * \brief FileInterface class implementation.
 */


#include "file_interface.h"

#include <stdexcept>

using namespace std;

void FileInterface::open(const string& name, const string& mode) {
    m_fileName = name;
    m_file = std::fopen(name.c_str(), mode.c_str());
    if (m_file == nullptr) {
        throw std::runtime_error("Can't open a file" + name);
    }
}

FileInterface::~FileInterface() {
    close();
}
std::FILE* FileInterface::get() {
    return m_file;
}
std::string FileInterface::getFilename() const {
    return m_fileName;
}
void FileInterface::close() {
    if (m_file != nullptr) {
        std::fclose(m_file);
        m_file = nullptr;
    }
}
