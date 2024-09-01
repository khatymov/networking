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
};


template<typename DataType>
FileWriter<DataType>::FileWriter(std::shared_ptr<ThreadSafeQueue<DataType>> curQueue,
                                 std::shared_ptr<ThreadSafeQueue<DataType>> nextQueue)
    : DataProcessor<FileWriter<DataType>, DataType>(curQueue, nextQueue) {}


template <typename DataType>
void FileWriter<DataType>::processDataImpl() {

    // add isDone assign
}

} // namespace network

#endif  // NETWORKING_FILE_WRITER_H
