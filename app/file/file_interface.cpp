/*! \file file_interface.cpp
 * \brief FileInterface class implementation.
 */

#include "file_interface.h"

#include <iostream>
#include <stdexcept>

using namespace std;

void FileInterface::open(const string& name, const string& mode) {
    m_fileName = name;
    if (mode == "w") {
        // If in some threads the names of files are identical - and all of them try to
        // create a file for writing with the same name - give a unique name
        auto* preopen = std::fopen(name.c_str(), "r");
        if (preopen) {
            int num = rand()%1000;
            m_fileName.insert(m_fileName.find('.'), to_string(num)) ;
            cout << "Old name: " << name << " new name: " << m_fileName << endl;
        }
    }

    m_file = std::fopen(m_fileName.c_str(), mode.c_str());
    if (m_file == nullptr) {
        throw std::runtime_error("Can't open a file" + name);
    }
}

FileInterface::~FileInterface() {
    if (m_file != nullptr) {
        close();
    }
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
