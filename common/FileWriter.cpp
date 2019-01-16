#include "FileWriter.h"
#include "FileError.h"


using namespace FileExchange;


FileWriter::FileWriter(const std::string& filename)
    : filename_(filename)
{
    this->openFile();
}


FileWriter::FileWriter(const std::string& filename, const std::shared_ptr<FileMutex>& mutexPtr)
    : filename_(filename)
    , mutexPtr_(mutexPtr)
{
    this->openFile();
}


void FileWriter::openFile()
{
    if (mutexPtr_) {
        lock_ = FileWriteLock(*mutexPtr_, std::try_to_lock);
        if (!lock_) {
            throw FileLockError("Can't lock file for writing: " + filename_);
        }
    }

    stream_.open(filename_, std::ios::binary | std::ios::trunc);
    if (!stream_) {
        throw FileIOError("Can't open file for writing: " + filename_);
    }
}


void FileWriter::write(const std::string& buffer)
{
    auto bufferSize = buffer.size();
    if (bufferSize > 0) {
        if (!stream_.write(buffer.data(), bufferSize)) {
            throw FileIOError("Can't write file: " + filename_);
        }
    }
}

