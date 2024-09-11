//
// Created by Renat_Khatymov on 9/1/2024.
//

#ifndef NETWORKING_FILE_READER_H
#define NETWORKING_FILE_READER_H

#include "data_processor_interface.h"

#include "hash_calculator.h"

namespace network {

const std::string FileReaderKey("FileReader");

template <typename DataType>
class FileReader: public DataProcessor<FileReader<DataType>, DataType> {
    FileReader(const FileReader&) = delete;
    FileReader(FileReader&&) = delete;
    FileReader& operator=(const FileReader&) = delete;
    FileReader& operator=(FileReader&&) = delete;
public:
    FileReader(const std::string& name,
               std::shared_ptr<ThreadSafeQueue<DataType>> curQueue,
               std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue);

    ~FileReader();
    // open/close file & read data
    void processDataImpl();

    // set file name to data_
    void setFileName();

    // set file;s hash to data_
    void setHash();

    void setExitPack();

protected:
    std::FILE* file_;
    std::string fileName_;
};


template <typename DataType>
FileReader<DataType>::FileReader(const std::string& name,
                                 std::shared_ptr<ThreadSafeQueue<DataType>> curQueue,
                                 std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
    :DataProcessor<FileReader<DataType>, DataType>(curQueue, nextQueue),
     fileName_(name) {
    file_ = std::fopen(name.c_str(), "rb");
}

template <typename DataType>
FileReader<DataType>::~FileReader() {
    if (file_ != nullptr) {
        std::fclose(file_);
        file_ = nullptr;
    }
}

template <typename DataType>
void FileReader<DataType>::setFileName() {
    this->data_->header.type = Header::Type::FileName;
    this->data_->header.length = fileName_.size();
    memcpy(this->data_->data.data(), fileName_.c_str(), fileName_.size());
}

template <typename DataType>
void FileReader<DataType>::processDataImpl() {
    this->data_->header.type = Header::Type::FileData;
    this->data_->header.length = std::fread(&this->data_->data,
                                    sizeof(this->data_->data[0]),
                                    PACKET_DATA_SIZE,
                                    file_);

    if (this->data_->header.length == 0) {
        this->isProcessDone_ = true;
    }
}

template <typename DataType>
void FileReader<DataType>::setHash() {
    if (file_ != nullptr) {
        std::fclose(file_);
        file_ = nullptr;
    }
    auto curHash = HashCalculator::getFileHash(fileName_);
    this->data_->header.type = Header::Type::Hash;
    this->data_->header.length = curHash.size();
    memcpy(this->data_->data.data(), curHash.c_str(), curHash.size());
}

template <typename DataType>
void FileReader<DataType>::setExitPack() {
    this->data_->header.type = Header::Type::Exit;
    this->data_->header.length = 0;
}

} // namespace network

#endif  // NETWORKING_FILE_READER_H
