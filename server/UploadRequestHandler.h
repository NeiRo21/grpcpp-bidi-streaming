#include "HandlerInterface.h"
#include "FileExchange.grpc.pb.h"
#include "FileManager.h"

#include <grpcpp/grpcpp.h>


namespace FileExchange {

class HandlerManager;


class UploadRequestHandler : public HandlerInterface
{
public:
    UploadRequestHandler(HandlerTag tag,
                         HandlerManager* handlerManager,
                         FileManager* fileManager,
                         SimpleFileServer::AsyncService* service,
                         grpc::ServerCompletionQueue* cq);

    ~UploadRequestHandler() override;

    UploadRequestHandler(const UploadRequestHandler&)            = delete;
    UploadRequestHandler& operator=(const UploadRequestHandler&) = delete;
    UploadRequestHandler(UploadRequestHandler&&)                 = delete;
    UploadRequestHandler& operator=(UploadRequestHandler&&)      = delete;

    bool onNext(bool ok) override;

    void cancel() override;

protected:
    enum class CallState
    {
        NewCall,
        ExpectingHeader,
        ReceivingHeader,
        ReceivingFile,
        CallComplete
    };

    void handleNewCallState();
    void handleExpectingHeaderState();
    void handleReceivingHeaderState();
    void handleReceivingFileState();

    HandlerTag                      tag_;
    HandlerManager*                 handlerManager_;

    FileManager*                    fileManager_;
    std::unique_ptr<FileWriter>     fileWriter_;

    CallState                       state_;
    const grpc::Status*             errorStatus_;

    SimpleFileServer::AsyncService* service_;
    grpc::ServerCompletionQueue*    cq_;
    grpc::ServerContext             ctx_;

    UploadRequest                   request_;
    UploadResponse                  response_;

    grpc::ServerAsyncReader<UploadResponse, UploadRequest> responder_;
};

}

