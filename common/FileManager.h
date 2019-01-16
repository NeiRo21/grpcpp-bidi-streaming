#pragma once

#include "FileLock.h"
#include "FileReader.h"
#include "FileWriter.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>


namespace FileExchange {

class FileManager final
{
public:
    FileManager() = default;

    FileManager(const std::string& filename);

    template<class InputIt>
    FileManager(InputIt first, InputIt last);

    ~FileManager() = default;

    // FileManager is not copyable
    FileManager(const FileManager&)            = delete;
    FileManager& operator=(const FileManager&) = delete;

    // FileManager is movable
    FileManager(FileManager&&)                 = default;
    FileManager& operator=(FileManager&&)      = default;

    bool isManaged(const std::string& filename) const;

    std::unique_ptr<FileReader> readFile(const std::string& filename) const;
    std::unique_ptr<FileWriter> writeFile(const std::string& filename);

private:
    void manageFile(const std::string& filename);

    mutable std::unordered_map<std::string, std::shared_ptr<FileMutex>> fileMutexes_;
    mutable std::mutex                                                  mgrMutex_;
};


template<class InputIt>
FileManager::FileManager(InputIt first, InputIt last)
{
    while (first != last) {
        const auto& filename = *first;
        this->manageFile(filename);
        ++first;
    }
}

}

