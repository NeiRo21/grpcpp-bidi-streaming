#pragma once

#include <mutex>
#include <shared_mutex>


namespace FileExchange {

using FileMutex     = std::shared_timed_mutex;
using FileReadLock  = std::shared_lock<FileMutex>;
using FileWriteLock = std::unique_lock<FileMutex>;

}

