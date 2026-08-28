#include "foundry_host/host.hpp"

#include <httplib.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <thread>

namespace {

std::atomic_bool stopRequested{false};

extern "C" void HandleSignal(int)
{
    stopRequested.store(true, std::memory_order_relaxed);
}

} // namespace

int main()
{
    try {
        const int port = foundry_host::ResolvePort(std::getenv("PORT"));
        foundry_host::Host host{foundry_host::CreateDefaultAgentRunner};
        httplib::Server server;
        host.Configure(server);

        std::signal(SIGINT, HandleSignal);
        std::signal(SIGTERM, HandleSignal);

        if (!server.bind_to_port("0.0.0.0", port)) {
            std::cerr << "Failed to bind to port " << port << ".\n";
            return EXIT_FAILURE;
        }

        std::jthread shutdownMonitor{[&server](std::stop_token token) {
            while (!token.stop_requested() &&
                   !stopRequested.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::milliseconds{100});
            }
            if (stopRequested.load(std::memory_order_relaxed)) {
                server.stop();
            }
        }};

        std::cout << "Listening on 0.0.0.0:" << port << '\n';
        if (!server.listen_after_bind()) {
            std::cerr << "Server stopped unexpectedly.\n";
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "Hosted agent failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
