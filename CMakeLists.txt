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

find_package(gRPC CONFIG QUIET)
if(gRPC_FOUND)
    message(STATUS "Found gRPC++: ${gRPC_VERSION}")
    set(GRPCPP_INCLUDE_DIRS $<TARGET_PROPERTY:gRPC::grpc++,INTERFACE_INCLUDE_DIRECTORIES>)
    set(GRPCPP_LIBRARIES $<TARGET_PROPERTY:gRPC::grpc++_unsecure,INTERFACE_LINK_LIBRARIES>)
    set(GRPCPP_PLUGIN_EXEC $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GRPCPP REQUIRED QUIET grpc++_unsecure)
    set(GRPCPP_PLUGIN_EXEC ${GRPCPP_PREFIX}/bin/grpc_cpp_plugin)
    message(STATUS "Found gRPC++: ${GRPCPP_LINK_LIBRARIES} (found version \"${GRPCPP_VERSION}\")")
endif()

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

get_filename_component(PROTO_FILE "${SRC_DIR}/common/FileExchange.proto" ABSOLUTE)
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
        --plugin=protoc-gen-grpc="${GRPCPP_PLUGIN_EXEC}"
        "${PROTO_FILE}"
    DEPENDS "${PROTO_FILE}")


include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(FileExchangeServer
               ${SRC_DIR}/server/main.cpp
               ${SRC_DIR}/server/Server.cpp
               ${SRC_DIR}/common/FileReader.cpp
               ${SRC_DIR}/common/FileWriter.cpp
               ${SRC_DIR}/common/FileManager.cpp
               ${SRC_DIR}/server/DownloadRequestHandler.cpp
               ${SRC_DIR}/server/UploadRequestHandler.cpp
               ${SRC_DIR}/common/RequestStatus.cpp
               ${PROTO_SRCS}
               ${GRPC_SRCS})
target_include_directories(FileExchangeServer PRIVATE ${SRC_DIR}/server ${SRC_DIR}/common)
target_link_libraries(FileExchangeServer
                      ${GRPCPP_LIBRARIES}
                      ${Protobuf_LIBRARIES})

add_executable(FileExchangeClient
               ${SRC_DIR}/client/main.cpp
               ${SRC_DIR}/client/Client.cpp
               ${SRC_DIR}/client/DownloadRequestHandler.cpp
               ${SRC_DIR}/client/UploadRequestHandler.cpp
               ${SRC_DIR}/common/FileReader.cpp
               ${SRC_DIR}/common/FileWriter.cpp
               ${PROTO_SRCS}
               ${GRPC_SRCS})
target_include_directories(FileExchangeClient PRIVATE ${SRC_DIR}/client ${SRC_DIR}/common)
target_link_libraries(FileExchangeClient
                      ${GRPCPP_LIBRARIES}
                      ${Protobuf_LIBRARIES})

