#include "FileExchange.grpc.pb.h"
#include "FileManager.h"
#include "HandlerManager.hpp"

#include <grpcpp/grpcpp.h>
#include <signal.h>

#include <atomic>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>


namespace FileExchange {

class FileExchangeServer final
{
public:
    FileExchangeServer(const std::string& address,
               unsigned long threadNumber,
               FileManager& fileManager)
        : address_(address)
        , shutdownFlag_(false)
        , fileManager_(fileManager)
    {
        if (threadNumber == 0) {
            throw std::logic_error("Worker threads number has to be greater than 0");
        }

        workerThreads_.resize(threadNumber);
    }
    
    ~FileExchangeServer();

    FileExchangeServer(const FileExchangeServer&)            = delete;
    FileExchangeServer& operator=(const FileExchangeServer&) = delete;
    FileExchangeServer(FileExchangeServer&&)                 = delete;
    FileExchangeServer& operator=(FileExchangeServer&&)      = delete;

    void start();
    void stop();

    void processMessages();

private:
    SimpleFileServer::AsyncService               service_;
    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    std::unique_ptr<grpc::Server>                server_;
    std::string                                  address_;

    std::vector<std::thread>                     workerThreads_;
    std::atomic_bool                             shutdownFlag_;

    HandlerManager                               handlerManager_;
    FileManager&                                 fileManager_;
};

}

