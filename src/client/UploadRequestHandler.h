#include "HandlerInterface.h"
#include "FileExchange.grpc.pb.h"
#include "FileManager.h"

#include <grpcpp/grpcpp.h>


namespace FileExchange {

class UploadRequestHandler : public HandlerInterface
{
public:
    UploadRequestHandler(HandlerTag tag,
                         SimpleFileServer::Stub* stub,
                         grpc::CompletionQueue* cq,
                         const std::string& filename);

    ~UploadRequestHandler() override = default;

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
        SendingHeader,
        SendingFile,
        ExpectingResponse,
        CallComplete
    };

    void handleNewCallState();
    void handleSendingHeaderState();
    void handleSendingFileState();
    void handleExpectingResponseState();
    void handleCallCompleteState();

    HandlerTag              tag_;

    SimpleFileServer::Stub* stub_;
    grpc::CompletionQueue*  cq_;
    grpc::ClientContext     ctx_;

    std::unique_ptr<grpc::ClientAsyncWriter<UploadRequest>> rpc_;

    UploadRequest               request_;
    UploadResponse              response_;
    grpc::Status                status_;

    CallState                   state_;

    std::unique_ptr<FileReader> fileReader_;

    std::string                 filename_;

    unsigned long long          bytesToSend_;
};

}

