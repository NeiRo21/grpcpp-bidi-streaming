#include "Server.h"
#include "FileManager.h"

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>
#include <signal.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <thread>


using namespace std::chrono_literals;
using namespace FileExchange;


std::atomic_bool shutdownRequested(false);


void signalHandler(int signo)
{
    shutdownRequested.store(true, std::memory_order_release);
}


void setSignalHandler()
{
        struct sigaction sa;
        sa.sa_handler = signalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);
}


int main(int argc, char** argv)
{
    try {
        gpr_log_verbosity_init();
        gpr_set_log_verbosity(GPR_LOG_SEVERITY_ERROR);

        gpr_log(GPR_DEBUG, "Setting up signal handlers");
        setSignalHandler();

        gpr_log(GPR_DEBUG, "Starting server");
        FileManager fileManager(argv + 1, argv + argc);
        FileExchangeServer server("0.0.0.0:55555", 2, fileManager);
        server.start();

        std::cout << "Press Ctrl-C to terminate..." << std::endl;
        while (!shutdownRequested.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(1s);
        }
        gpr_log(GPR_DEBUG, "Main thread: graceful shutdown requested");

        return EXIT_SUCCESS;
    }
    catch (std::exception& e) {
        gpr_log(GPR_ERROR, "Caught exception: %s", e.what());
    }
    catch (...) {
        gpr_log(GPR_ERROR, "Caught unknown exception");
    }

    gpr_log(GPR_DEBUG, "Main thread is shutting down abnormally");
    return EXIT_FAILURE;
}

