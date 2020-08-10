#include "UploadRequestHandler.h"
#include "FileError.h"
#include "HandlerManager.hpp"
#include "RequestStatus.h"

#include <grpc/support/log.h>


using namespace FileExchange;


UploadRequestHandler::UploadRequestHandler(HandlerTag tag,
                                           HandlerManager* handlerManager,
                                           FileManager* fileManager,
                                           SimpleFileServer::AsyncService* service,
                                           grpc::ServerCompletionQueue* cq)
    : tag_(tag)
    , handlerManager_(handlerManager)
    , fileManager_(fileManager)
    , state_(CallState::NewCall)
    , errorStatus_(nullptr)
    , service_(service)
    , cq_(cq)
    , responder_(&ctx_)
{
    this->onNext(true);
}


UploadRequestHandler::~UploadRequestHandler()
{
    gpr_log(GPR_DEBUG, "[%p] The handler is destroyed", tag_);
}


bool UploadRequestHandler::onNext(bool ok)
{
    try {
        if (state_ == CallState::CallComplete) {
            gpr_log(GPR_DEBUG, "[%p] The call has been completed", tag_);
            return false;
        }
        else if (ok) {
            if (state_ == CallState::NewCall) {
                this->handleNewCallState();
            }
            else if (state_ == CallState::ExpectingHeader) {
                this->handleExpectingHeaderState();
            }
            else if (state_ == CallState::ReceivingHeader) {
                this->handleReceivingHeaderState();
            }
            else if (state_ == CallState::ReceivingFile) {
                this->handleReceivingFileState();
            }
        }
        else {
            if (state_ == CallState::ExpectingHeader) {
                // server has been shut down before receiving a matching UploadRequest
                gpr_log(GPR_DEBUG, "[%p] Server has been shut down before receiving a matching UploadRequest", tag_);
                return false;
            }
            else {
                auto& rpcStatus = (state_ == CallState::ReceivingFile) ? grpc::Status::OK
                                                                       : grpc::Status::CANCELLED;
                state_ = CallState::CallComplete;
                responder_.Finish(response_, rpcStatus, tag_);
            }
        }
    }
    catch (FileLockError& e) {
        errorStatus_ = &RequestStatus::FileLocked;
        gpr_log(GPR_ERROR, "[%p] Upload processing error: %s", tag_, e.what());
    }
    catch (FileIOError& e) {
        errorStatus_ = &RequestStatus::FileIOError;
        gpr_log(GPR_ERROR, "[%p] Upload processing error: %s", tag_, e.what());
    }
    catch (std::exception& e) {
        errorStatus_ = &RequestStatus::UnknownError;
        gpr_log(GPR_ERROR, "[%p] Upload processing error: %s", tag_, e.what());
    }
    catch (...) {
        errorStatus_ = &RequestStatus::UnknownError;
        gpr_log(GPR_ERROR, "[%p] Upload processing error: unknown exception caught", tag_);
    }

    if (errorStatus_ != nullptr) {
        state_ = CallState::CallComplete;
        responder_.FinishWithError(*errorStatus_, tag_);
    }

    return true;
}


void UploadRequestHandler::cancel()
{
    //TODO
}


void UploadRequestHandler::handleNewCallState()
{
    state_ = CallState::ExpectingHeader;
    service_->RequestUpload(&ctx_, &responder_, cq_, cq_, tag_);
}


void UploadRequestHandler::handleExpectingHeaderState()
{
    try {
        handlerManager_->addHandler<UploadRequestHandler>(handlerManager_, fileManager_, service_, cq_);
    }
    catch (...) {
        gpr_log(GPR_ERROR, "[%p] Failed to create UploadRequest handler, no new UploadRequest's can be processed", tag_);
    }

    state_ = CallState::ReceivingHeader;
    responder_.Read(&request_, tag_);
}


void UploadRequestHandler::handleReceivingHeaderState()
{
    if (request_.has_header()) {
        auto& filename = request_.header().name();

        gpr_log(GPR_INFO, "[%p] Received UploadRequest: file name [%s], file size [%ld]",
                          tag_, filename.c_str(), request_.header().size());

        if (!filename.empty()) {
            fileWriter_ = fileManager_->writeFile(filename);

            response_.set_bytesreceived(0);

            state_ = CallState::ReceivingFile;
            request_.Clear();
            responder_.Read(&request_, tag_);
        }
        else {
            errorStatus_ = &RequestStatus::FileNameEmpty;
        }
    }
    else {
        errorStatus_ = &RequestStatus::FileHeaderExpected;
    }
}


void UploadRequestHandler::handleReceivingFileState()
{
    if (request_.has_chunk()) {
        auto& chunkData = request_.chunk().data();
        if (!chunkData.empty()) {
            fileWriter_->write(chunkData);
            response_.set_bytesreceived(response_.bytesreceived() + chunkData.size());
        }

        request_.Clear();
        responder_.Read(&request_, tag_);
    }
    else {
        errorStatus_ = &RequestStatus::FileChunkExpected;
    }
}

