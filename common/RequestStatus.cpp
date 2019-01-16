#include "RequestStatus.h"


namespace FileExchange {

namespace RequestStatus {

const grpc::Status FileHeaderExpected (grpc::INVALID_ARGUMENT, "File header expected");
const grpc::Status FileNameEmpty(grpc::INVALID_ARGUMENT, "File name is empty");
const grpc::Status FileChunkExpected(grpc::INVALID_ARGUMENT, "File chunk expected");
const grpc::Status FileNotFound(grpc::NOT_FOUND, "File not found");
const grpc::Status FileLocked(grpc::UNAVAILABLE, "File is locked");
const grpc::Status FileIOError(grpc::ABORTED, "File IO error");
const grpc::Status UnknownError(grpc::UNKNOWN, "Unknown error");

}

}

