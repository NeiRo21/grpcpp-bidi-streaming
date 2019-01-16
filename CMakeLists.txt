cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(FileExchange CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

add_compile_options(-Wall -Werror)


find_package(Protobuf REQUIRED)


find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(GRPC_CPP_LIB gRPC::grpc++_unsecure)
set(GRPC_PLUGIN_EXEC $<TARGET_FILE:gRPC::grpc_cpp_plugin>)


get_filename_component(PROTO_FILE "./common/FileExchange.proto" ABSOLUTE)
get_filename_component(PROTO_PATH "${PROTO_FILE}" PATH)
get_filename_component(PROTO_NAME "${PROTO_FILE}" NAME_WE)

#generated sources
set(PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
set(PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
set(GRPC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
set(GRPC_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")

add_custom_command(
    OUTPUT "${PROTO_SRCS}" "${PROTO_HDRS}" "${GRPC_SRCS}" "${GRPC_HDRS}"
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${PROTO_PATH}"
        --plugin=protoc-gen-grpc="${GRPC_PLUGIN_EXEC}"
        "${PROTO_FILE}"
    DEPENDS "${PROTO_FILE}")


include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(FileExchangeServer
               server/main.cpp
               server/Server.cpp
               common/FileReader.cpp
               common/FileWriter.cpp
               common/FileManager.cpp
               server/DownloadRequestHandler.cpp
               server/UploadRequestHandler.cpp
               common/RequestStatus.cpp
               ${PROTO_SRCS}
               ${GRPC_SRCS})
target_include_directories(FileExchangeServer PRIVATE ./server ./common)
target_link_libraries(FileExchangeServer
                      ${GRPC_CPP_LIB}
                      ${Protobuf_LIBRARIES})

add_executable(FileExchangeClient
               client/main.cpp
               client/Client.cpp
               client/DownloadRequestHandler.cpp
               client/UploadRequestHandler.cpp
               common/FileReader.cpp
               common/FileWriter.cpp
               ${PROTO_SRCS}
               ${GRPC_SRCS})
target_include_directories(FileExchangeClient PRIVATE ./client ./common)
target_link_libraries(FileExchangeClient
                      ${GRPC_CPP_LIB}
                      ${Protobuf_LIBRARIES})


#    add_executable(GTestMain
#                   ${TEST_DIR}/field_test.cpp
#                   ${TEST_DIR}/record_test.cpp
#                   ${TEST_DIR}/file_test.cpp)
#
#    target_include_directories(GTestMain SYSTEM
#                               PRIVATE ${GTEST_INCLUDE_DIR})

