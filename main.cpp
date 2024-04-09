/*! \file main.cpp
 * \brief Entry point.
 */

#include <iostream>
#include <span>
#include <boost/asio.hpp>

#include "common.h"
#include "server.h"
#include "client.h"

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
        Server server(ip.data(), uint(std::stoul(port_str.data(),nullptr,0)));
        server.start();
    } else if (argc == 4) {
        //client code
        spdlog::default_logger()->set_pattern("[CLIENT] %+ [thread %t]");
        const string_view filePath{args[3]};
        Client client(ip.data(), uint(std::stoul(port_str.data(),nullptr,0)));
//        client.sendFile(filePath.data());
        if (client.connect()) {
//            return client.doPingPing();
            return client.sendFile(filePath.data());
        } else {
            return EXIT_FAILURE;
        }
    }

    return 0;
}