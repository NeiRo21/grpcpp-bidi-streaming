#pragma once

#include <grpcpp/grpcpp.h>


namespace FileExchange {

namespace RequestStatus {

extern const grpc::Status FileHeaderExpected;
extern const grpc::Status FileNameEmpty;
extern const grpc::Status FileChunkExpected;
extern const grpc::Status FileNotFound;
extern const grpc::Status FileLocked;
extern const grpc::Status FileIOError;
extern const grpc::Status UnknownError;

}

}

