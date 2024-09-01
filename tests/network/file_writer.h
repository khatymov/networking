//
// Created by Renat_Khatymov on 8/22/2024.
//

#ifndef NETWORKING_FILE_WRITER_H
#define NETWORKING_FILE_WRITER_H

#include "data_processor_interface.h"

namespace network {

template <typename DataType>
class FileWriter: public DataProcessor<FileWriter<DataType>, DataType> {
    FileWriter(const FileWriter&) = delete;
    FileWriter(FileWriter&&) = delete;
    FileWriter& operator=(const FileWriter&) = delete;
    FileWriter& operator=(FileWriter&&) = delete;
public:
    FileWriter(std::shared_ptr<ThreadSafeQueue<DataType>> curQueue,
               std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue);

    // open/close file & write data/calculate hash of a file
    void processDataImpl();

    // generate unique name: file name + timestamp as a postfix
    static std::string getUniqueName(const std::string& fileName);

protected:
    std::FILE* file_;
    std::string fileName_;
};


template<typename DataType>
FileWriter<DataType>::FileWriter(std::shared_ptr<ThreadSafeQueue<DataType>> curQueue,
                                 std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
    : DataProcessor<FileWriter<DataType>, DataType>(curQueue, nextQueue) {}


template <typename DataType>
void FileWriter<DataType>::processDataImpl() {
    switch (this->data_->header.type) {
        case (Header::Type::FileName): {

            std::filesystem::path filePath(std::string(this->data_->data.data(),
                                                       this->data_->header.length));
            auto fileName = filePath.filename().string();

            if (std::filesystem::exists(fileName)) {

#ifdef DEBUG
                spdlog::debug("File with the name {} exists. Generate unique name.", fileName);
#endif
                fileName = getUniqueName(fileName);
            }
#ifdef DEBUG
            spdlog::debug("Open a file:  {}", fileName);
#endif
            fileName_ = fileName;
            file_ = std::fopen(fileName.c_str(), "w");
            break;
        };
        case (Header::Type::FileData): {
            auto bytesWritten = std::fwrite(this->data_->data,
                                            sizeof(this->data_->data[0]),
                                            this->data_->header.length,
                                            file_);

            if (bytesWritten != this->data_->header.length) {
                spdlog::debug("Some data wasn't written to a file");
            }

            break;
        };
        case (Header::Type::Hash): {

        };
        default: {
            spdlog::error("Unknown packet");
            break;
        }
    }

    // add isDone assign
}

template <typename DataType>
std::string FileWriter<DataType>::getUniqueName(const std::string& fileName) {
    std::string uniqueName;

    using namespace std::chrono;
    const auto ts = std::to_string(duration_cast<milliseconds>
                            (system_clock::now().time_since_epoch()).count());

    // tmp.txt -> 4 - dot pos
    const auto dotPos = fileName.find('.');

    if (dotPos != std::string::npos) {
        uniqueName = fileName.substr(0, dotPos) + ts + fileName.substr(dotPos, fileName.size());
    } else {
        // file doesn't have explicit extension - just append ts
        uniqueName = fileName + ts;
    }

    return uniqueName;
}


} // namespace network

#endif  // NETWORKING_FILE_WRITER_H
