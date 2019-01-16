#include "UploadRequestHandler.h"

#include <grpc/support/log.h>

#include <algorithm>
#include <iostream>


using namespace FileExchange;


UploadRequestHandler::UploadRequestHandler(HandlerTag tag,
                                           SimpleFileServer::Stub* stub,
                                           grpc::CompletionQueue* cq,
                                           const std::string& filename)
    : tag_(tag)
    , stub_(stub)
    , cq_(cq)
    , state_(CallState::NewCall)
    , filename_(filename)
    , bytesToSend_(0)
{
    this->onNext(true);
}


bool UploadRequestHandler::onNext(bool ok)
{
    try {
        if (state_ == CallState::CallComplete) {
            this->handleCallCompleteState();
            return false; //TODO comment
        }
        else if (ok) {
            if (state_ == CallState::NewCall) {
                this->handleNewCallState();
            }
            else if (state_ == CallState::SendingHeader) {
                this->handleSendingHeaderState();
            }
            else if (state_ == CallState::SendingFile) {
                this->handleSendingFileState();
            }
            else if (state_ == CallState::ExpectingResponse) {
                this->handleExpectingResponseState();
            }
        }
        else {
            state_ = CallState::CallComplete;
            rpc_->Finish(&status_, tag_);
        }

        return true;
    }
    catch (std::exception& e) {
        gpr_log(GPR_ERROR, "Upload processing error: %s", e.what());
    }
    catch (...) {
        gpr_log(GPR_ERROR, "Upload processing error: unknown exception caught");
    }

    if (state_ == CallState::NewCall) {
        //TODO comment
        return false;
    }

    ctx_.TryCancel();

    return true;
}


void UploadRequestHandler::cancel()
{
    ctx_.TryCancel();
}


void UploadRequestHandler::handleNewCallState()
{
    fileReader_ = std::make_unique<FileReader>(filename_);

    rpc_ = stub_->PrepareAsyncUpload(&ctx_, &response_, cq_);

    state_ = CallState::SendingHeader;
    rpc_->StartCall(tag_);
}


void UploadRequestHandler::handleSendingHeaderState()
{
    request_.mutable_header()->set_name(filename_);

    bytesToSend_ = fileReader_->fileSize();
    request_.mutable_header()->set_size(bytesToSend_);

    if (bytesToSend_ > 0) {
        state_ = CallState::SendingFile;
    }
    else {
        state_ = CallState::ExpectingResponse;
    }

    rpc_->Write(request_, tag_);
}


void UploadRequestHandler::handleSendingFileState()
{
    const unsigned long long DefaultChunkSize = 4 * 1024; // 4K

    auto chunkSize = std::min(DefaultChunkSize, bytesToSend_);

    request_.Clear();
    auto buffer = request_.mutable_chunk()->mutable_data();
    buffer->resize(chunkSize);

    fileReader_->read(*buffer);
    bytesToSend_ -= chunkSize;

    grpc::WriteOptions writeOptions;
    if (bytesToSend_ > 0) {
        state_ = CallState::SendingFile;
    }
    else {
        state_ = CallState::ExpectingResponse;
        writeOptions.set_last_message();
    }

    rpc_->Write(request_, writeOptions, tag_);
}


void UploadRequestHandler::handleExpectingResponseState()
{
    state_ = CallState::CallComplete;
    rpc_->Finish(&status_, tag_);
}


void UploadRequestHandler::handleCallCompleteState()
{
    switch (status_.error_code()) {
    case grpc::OK:
        {
            auto bytesSent = fileReader_ ? fileReader_->fileSize() : 0;
            std::cout << "[" << filename_ << "]: upload complete: " << bytesSent << " bytes sent" << std::endl;
        }
        break;

    case grpc::CANCELLED:
        std::cout << "[" << filename_ << "]: upload cancelled" << std::endl;
        break;

    default:
        std::cout << "[" << filename_ << "]: upload failed: " << status_.error_message() << std::endl;
        break;
    }
}

