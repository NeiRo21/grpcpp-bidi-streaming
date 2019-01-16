#include "HandlerInterface.h"
#include "FileExchange.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>


namespace FileExchange {

class FileExchangeClient final
{
public:
    FileExchangeClient(const std::shared_ptr<grpc::Channel>& channel)
        : stub_(SimpleFileServer::NewStub(channel))
    {
    }
    
    ~FileExchangeClient();

    FileExchangeClient(const FileExchangeClient&)            = delete;
    FileExchangeClient& operator=(const FileExchangeClient&) = delete;
    FileExchangeClient(FileExchangeClient&&)                 = delete;
    FileExchangeClient& operator=(FileExchangeClient&&)      = delete;

    void upload(const std::string& filename);
    void download(const std::string& filename);

    void cancel();

private:
    void processMessages();

    std::unique_ptr<SimpleFileServer::Stub> stub_;
    grpc::CompletionQueue                   cq_;

    std::unique_ptr<HandlerInterface>       handler_;
};

}

