#include "HandlerInterface.h"
#include "FileExchange.grpc.pb.h"
#include "FileManager.h"

#include <grpcpp/grpcpp.h>


namespace FileExchange {

class HandlerManager;


class DownloadRequestHandler : public HandlerInterface
{
public:
    DownloadRequestHandler(HandlerTag tag,
                           HandlerManager* handlerManager,
                           FileManager* fileManager,
                           SimpleFileServer::AsyncService* service,
                           grpc::ServerCompletionQueue* cq);

    ~DownloadRequestHandler() override;

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
        ExpectingRequest,
        SendingFile,
        CallComplete
    };

    void handleNewCallState();
    void handleExpectingRequestState();
    void handleSendingFileState();

    HandlerTag                      tag_;
    HandlerManager*                 handlerManager_;

    FileManager*                    fileManager_;
    std::unique_ptr<FileReader>     fileReader_;

    CallState                       state_;
    const grpc::Status*             errorStatus_;

    SimpleFileServer::AsyncService* service_;
    grpc::ServerCompletionQueue*    cq_;
    grpc::ServerContext             ctx_;

    DownloadRequest                 request_;
    DownloadResponse                response_;

    grpc::ServerAsyncWriter<DownloadResponse> responder_;

    unsigned long long              bytesToSend_;
};

}

