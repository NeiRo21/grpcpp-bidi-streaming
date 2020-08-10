#include "HandlerInterface.h"
#include "FileExchange.grpc.pb.h"
#include "FileWriter.h"

#include <grpcpp/grpcpp.h>


namespace FileExchange {

class DownloadRequestHandler : public HandlerInterface
{
public:
    DownloadRequestHandler(HandlerTag tag,
                           SimpleFileServer::Stub* stub,
                           grpc::CompletionQueue* cq,
                           const std::string& filename);

    ~DownloadRequestHandler() override = default;

    DownloadRequestHandler(const DownloadRequestHandler&)            = delete;
    DownloadRequestHandler& operator=(const DownloadRequestHandler&) = delete;
    DownloadRequestHandler(DownloadRequestHandler&&)                 = delete;
    DownloadRequestHandler& operator=(DownloadRequestHandler&&)      = delete;

    bool onNext(bool ok) override;

    void cancel() override;

protected:
    enum class CallState
    {
        NewCall,
        SendingRequest,
        ExpectingHeader,
        ReceivingFile,
        CallComplete
    };

    void handleNewCallState();
    void handleSendingRequestState();
    void handleExpectingHeaderState();
    void handleReceivingFileState();
    void handleCallCompleteState();

    HandlerTag              tag_;

    SimpleFileServer::Stub* stub_;
    grpc::CompletionQueue*  cq_;
    grpc::ClientContext     ctx_;

    std::unique_ptr<grpc::ClientAsyncReader<DownloadResponse>> rpc_;

    DownloadRequest             request_;
    DownloadResponse            response_;
    grpc::Status                status_;

    CallState                   state_;

    std::unique_ptr<FileWriter> fileWriter_;

    std::string                 filename_;

    unsigned long long          bytesReceived_;
};

}

