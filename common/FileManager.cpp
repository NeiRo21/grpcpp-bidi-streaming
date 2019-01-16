#include "FileManager.h"
#include "FileError.h"


using namespace FileExchange;


using FileMutexSP = std::shared_ptr<FileMutex>;


FileManager::FileManager(const std::string& filename)
{
    this->manageFile(filename);
}


bool FileManager::isManaged(const std::string& filename) const
{
    std::lock_guard<std::mutex> lock(mgrMutex_);

    return fileMutexes_.count(filename) > 0;
}


std::unique_ptr<FileReader> FileManager::readFile(const std::string& filename) const
{
    FileMutexSP* mutexPtr = nullptr;

    {
        std::lock_guard<std::mutex> lock(mgrMutex_);

        auto it = fileMutexes_.find(filename);
        if (it == fileMutexes_.end()) {
            std::string message = "File is not managed by the server: " + filename;
            throw FileNotManagedError(message);
        }
        else {
            mutexPtr = &it->second;
        }
    }

    return std::make_unique<FileReader>(filename, *mutexPtr);
}


std::unique_ptr<FileWriter> FileManager::writeFile(const std::string& filename)
{
    FileMutexSP* mutexPtr = nullptr;

    {
        std::lock_guard<std::mutex> lock(mgrMutex_);

        auto it = fileMutexes_.find(filename);
        if (it == fileMutexes_.end()) {
            it = fileMutexes_.emplace(filename, std::make_shared<FileMutex>()).first;
        }

        mutexPtr = &it->second;
    }

    return std::make_unique<FileWriter>(filename, *mutexPtr);
}


void FileManager::manageFile(const std::string& filename)
{
    if (fileMutexes_.count(filename) == 0) {
        fileMutexes_.emplace(filename, std::make_shared<FileMutex>());
    }
}

