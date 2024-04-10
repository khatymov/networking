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

using boost::asio::ip::tcp;

using namespace std;

#include <cstdio>

int main(int argc, char* argv[]) {

    spdlog::default_logger()->set_level(spdlog::level::debug);

    const auto args = std::span(argv, size_t(argc));
    if (args.size() < 2) {
        cout << "Usage:\n";
        cout << "If server:  ./networking 127.0.0.1\n";
        cout << "If client:  ./networking 127.0.0.1 /path/to/file\n";
    }

    const string_view ip{args[1]};
    const string_view port_str{args[2]};
    if (argc == 3) {

        spdlog::default_logger()->set_pattern("[SERVER] %+ [thread %t]");

        boost::asio::io_context io_context;

        Server server(io_context, ip.data(), uint(std::stoul(port_str.data(), nullptr, 0)));

        // Number of threads you want to run io_context in.
        const std::size_t num_threads = std::thread::hardware_concurrency();

        std::vector<std::thread> threads;
        for(std::size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([&io_context]() {
                io_context.run();
            });
            //TODO: read https://www.glennklockwood.com/hpc-howtos/process-affinity.html
//#ifdef __linux__
//            // bind cpu affinity for worker thread in linux
//            cpu_set_t cpuset;
//            CPU_ZERO(&cpuset);
//            CPU_SET(i, &cpuset);
//            pthread_setaffinity_np(threads.back().native_handle(), sizeof(cpu_set_t), &cpuset);
//#endif
        }

        for(auto& t : threads) {
            if(t.joinable()) {
                t.join();
            }
        }

    } else if (argc == 4) {
        //client code
        spdlog::default_logger()->set_pattern("[CLIENT] %+ [thread %t]");
        string filePath{args[3]};
        Client client(ip.data(), uint(std::stoul(port_str.data(),nullptr,0)));
//        client.sendFile(filePath.data());
        if (client.connect()) {
//            return client.doPingPing();
            Timer t;
            return client.sendFile(filePath);
        } else {
            return EXIT_FAILURE;
        }
    }

    return 0;
}