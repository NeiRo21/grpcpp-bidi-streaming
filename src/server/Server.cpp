#include "Server.h"
#include "DownloadRequestHandler.h"
#include "UploadRequestHandler.h"

#include <grpc/support/log.h>


using namespace FileExchange;


FileExchangeServer::~FileExchangeServer()
{
    this->stop();
}


void FileExchangeServer::start()
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(address_, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();

    gpr_log(GPR_INFO, "Server listening on %s", address_.c_str());

    handlerManager_.addHandler<UploadRequestHandler>(&handlerManager_, &fileManager_, &service_, cq_.get());
    handlerManager_.addHandler<DownloadRequestHandler>(&handlerManager_, &fileManager_, &service_, cq_.get());

    gpr_log(GPR_DEBUG, "Starting worker threads");
    for (auto& worker : workerThreads_) {
        std::thread t(&FileExchangeServer::processMessages, this);
        worker.swap(t);
    }
}


void FileExchangeServer::processMessages()
{
    try {
        void* tag = nullptr;
        bool ok = false;
        while (!shutdownFlag_.load(std::memory_order_acquire)) {
            if (cq_->Next(&tag, &ok)) {
                if (tag) {
                    auto& handler = handlerManager_.getHandler(static_cast<HandlerTag>(tag));
                    if (handler) {
                        auto res = handler->onNext(ok);
                        if (!res) {
                            handler.reset();
                        }
                    }
                }
                else {
                    gpr_log(GPR_ERROR, "Invalid tag delivered by notification queue");
                }
            }
            else {
                // notification queue has been shut down
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

    gpr_log(GPR_DEBUG, "Worker thread is shutting down");
}


void FileExchangeServer::stop()
{
    shutdownFlag_.store(true, std::memory_order_release);

    gpr_log(GPR_DEBUG, "Shutting down server");
    server_->Shutdown();

    gpr_log(GPR_DEBUG, "Shutting down notification queue");
    cq_->Shutdown();

    gpr_log(GPR_DEBUG, "Waiting for worker threads to shut down");
    for (auto& worker : workerThreads_) {
        worker.join();
    }

    // drain the queue
    gpr_log(GPR_DEBUG, "Draining notification queue");
    void* ignoredTag = nullptr;
    bool ok = false;
    while (cq_->Next(&ignoredTag, &ok));
}

