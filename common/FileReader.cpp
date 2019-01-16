#include "FileReader.h"
#include "FileError.h"


using namespace FileExchange;


FileReader::FileReader(const std::string& filename)
    : filename_(filename)
    , size_(0)
{
    this->openFile();
}


FileReader::FileReader(const std::string& filename, const std::shared_ptr<FileMutex>& mutexPtr)
    : filename_(filename)
    , size_(0)
    , mutexPtr_(mutexPtr)
{
    this->openFile();
}


void FileReader::openFile()
{
    if (mutexPtr_) {
        lock_ = FileReadLock(*mutexPtr_, std::try_to_lock);
        if (!lock_) {
            throw FileLockError("Can't lock file for reading: " + filename_);
        }
    }

    stream_.open(filename_, std::ios::binary | std::ios::ate);
    if (!stream_) {
        throw FileIOError("Can't open file for reading: " + filename_);
    }

    auto size = stream_.tellg();
    stream_.seekg(0);
    if (size > 0) {
        size_ = static_cast<decltype(size_)>(size);
    }
}


std::size_t FileReader::read(std::string& buffer)
{
    std::size_t bytesRead = 0;

    if (!stream_.eof()) {
        auto bufferSize = buffer.size();
        if (bufferSize > 0) {
            if (!stream_.read(&buffer[0], bufferSize)) {
                throw FileIOError("Can't read file: " + filename_);
            }

            bytesRead = static_cast<std::size_t>(stream_.gcount());
        }
    }

    return bytesRead;
}

