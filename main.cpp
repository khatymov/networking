/*! \file main.cpp
 * \brief Entry point.
 */

#include <iostream>
#include <span>
#include <boost/asio.hpp>

#include "common.h"
#include "server.h"
#include "client.h"
#include "timer.h"
#include "cryptographer.h"

using boost::asio::ip::tcp;

using namespace std;


int main(int argc, char* argv[]) {

    spdlog::default_logger()->set_level(spdlog::level::debug);

    const auto args = std::span(argv, size_t(argc));
    if (args.size() < 2) {
        cout << "Usage:\n";
        cout << "If server:  ./networking 127.0.0.1 1234\n";
        cout << "If client:  ./networking 127.0.0.1 1234 /path/to/file\n";
    }

    const string_view ip{args[1]};
    const string_view port_str{args[2]};

    if (argc == 3) {

        spdlog::default_logger()->set_pattern("[SERVER] %+ [thread %t]");
        spdlog::info("Server starts work");

        boost::asio::io_context io_context;

        // create a server object and starts waiting for a new client
        Server server(io_context, ip.data(), uint(std::stoul(port_str.data(), nullptr, 0)));

        // Number of threads you want to run io_context in.
        const std::size_t num_threads = 2;// std::thread::hardware_concurrency();

        std::vector<std::thread> threads;
        for(std::size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([&io_context]() {
                io_context.run();
            });
        }

        for(auto& thrd : threads) {
            if(thrd.joinable()) {
                thrd.join();
            }
        }

    } else if (argc == 4) {
        spdlog::default_logger()->set_pattern("[CLIENT] %+ [thread %t]");
        spdlog::info("Client starts work");

        const string_view filePath{args[3]};
        Client client(ip.data(), uint(std::stoul(port_str.data(),nullptr,0)));

        if (client.connect()) {
            Timer t;
            return client.sendFile(filePath.data());
        } else {
            spdlog::info("Client could not connect to a server");
            return EXIT_FAILURE;
        }
    }

    return 0;
}