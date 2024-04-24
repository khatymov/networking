/*! \file file_interface.h
 * \brief FileInterface class interface.
 *
 */


#pragma once

#include <cstdio>
#include <string>
/*! \class FileInterface
 * \brief Provide minimal functionality to work with a file
 */
class FileInterface {
    FileInterface(const FileInterface&) = delete;
    FileInterface(FileInterface&&) = delete;
    FileInterface operator=(const FileInterface&) = delete;
    FileInterface operator=(FileInterface&&) = delete;
public:

    //! \brief default constructor.
    FileInterface() = default;
    //! \brief virtual destructor - close opened file.
    virtual ~FileInterface();

    /**
     * \brief Open a file with specific mode.
     *
     * \param name File name.
     * \param mode The mode in which the file will be opened(read - 'rb'/write - 'w').
     */
    void open(const std::string& name, const std::string& mode);
    //! \brief close file
    void close();
    //! \brief get a file name that was opened
    std::string getFilename() const;
    //! \brief get raw pointer to file instance
    std::FILE* get();

private:
    //! \brief raw pointer to file instance
    std::FILE* m_file;
    //! \brief the name of opened file
    std::string m_fileName;
};
