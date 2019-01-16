#pragma once

#include <stdexcept>
#include <string>


namespace FileExchange {

class FileLockError : public std::runtime_error
{
public:
    FileLockError(const std::string& message)
        : std::runtime_error(message)
    {
    }
};


class FileIOError : public std::runtime_error
{
public:
    FileIOError(const std::string& message)
        : std::runtime_error(message)
    {
    }
};


class FileNotManagedError : public std::runtime_error
{
public:
    FileNotManagedError(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

}

