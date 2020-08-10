#pragma once

#include "FileLock.h"

#include <fstream>
#include <memory>
#include <string>


namespace FileExchange {

class FileReader final
{
public:
    FileReader(const std::string& filename);
    FileReader(const std::string& filename, const std::shared_ptr<FileMutex>& mutexPtr);

    ~FileReader() = default;

    FileReader(const FileReader&)            = delete;
    FileReader& operator=(const FileReader&) = delete;

    FileReader(FileReader&&)                 = default;
    FileReader& operator=(FileReader&&)      = default;

    unsigned long long fileSize() const
    {
        return size_;
    }

    std::size_t read(std::string& buffer);

private:
    void openFile();

    std::ifstream              stream_;
    std::string                filename_;
    unsigned long long         size_;

    std::shared_ptr<FileMutex> mutexPtr_;
    FileReadLock               lock_;
};

}

