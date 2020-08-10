#pragma once

#include "FileLock.h"

#include <fstream>
#include <memory>
#include <string>


namespace FileExchange {

class FileWriter final
{
public:
    FileWriter(const std::string& filename);
    FileWriter(const std::string& filename, const std::shared_ptr<FileMutex>& mutexPtr);

    ~FileWriter() = default;

    FileWriter(const FileWriter&)            = delete;
    FileWriter& operator=(const FileWriter&) = delete;

    FileWriter(FileWriter&&)            = default;
    FileWriter& operator=(FileWriter&&) = default;

    void write(const std::string& buffer);

private:
    void openFile();

    std::ofstream              stream_;
    std::string                filename_;

    std::shared_ptr<FileMutex> mutexPtr_;
    FileWriteLock              lock_;
};

}

