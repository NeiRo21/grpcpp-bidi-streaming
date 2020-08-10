#include "Client.h"
#include "DownloadRequestHandler.h"
#include "UploadRequestHandler.h"

#include <grpc/support/log.h>


using namespace FileExchange;


FileExchangeClient::~FileExchangeClient()
{
    this->cancel();

    cq_.Shutdown();

    // drain the queue
    void* ignoredTag = nullptr;
    bool ok = false;
    while (cq_.Next(&ignoredTag, &ok));
}


void FileExchangeClient::upload(const std::string& filename)
{
    handler_ = std::make_unique<UploadRequestHandler>(&handler_, stub_.get(), &cq_, filename);

    this->processMessages();
}


void FileExchangeClient::download(const std::string& filename)
{
    handler_ = std::make_unique<DownloadRequestHandler>(&handler_, stub_.get(), &cq_, filename);

    this->processMessages();
}


void FileExchangeClient::cancel()
{
    if (handler_) {
        handler_->cancel();
    }
}


void FileExchangeClient::processMessages()
{
    try {
        void* tag = nullptr;
        bool ok = false;
        while (true) {
            if (cq_.Next(&tag, &ok)) {
                if (tag) {
                    //TODO assert
                    auto res = handler_->onNext(ok);
                    if (!res) {
                        //TODO comment
                        handler_.reset();
                        break;
                    }
                }
                else {
                    gpr_log(GPR_ERROR, "Invalid tag delivered by notification queue");
                }
            }
            else {
                gpr_log(GPR_ERROR, "Notification queue has been shut down unexpectedly");
                break;
            }
        }    
    }
    catch (std::exception& e) {
        gpr_log(GPR_ERROR, "Caught exception: %s", e.what());
    }
    catch (...) {
        gpr_log(GPR_ERROR, "Caught unknown exception");
    }
}

