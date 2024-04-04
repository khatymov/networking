/*! \file main.cpp
 * \brief Entry point.
 */

#include <iostream>
#include <span>
#include <boost/asio.hpp>

#include "server.h"
#include "common.h"

using boost::asio::ip::tcp;

using namespace std;

#include <cstdio>

int main(int argc, char* argv[]) {

    spdlog::default_logger()->set_pattern("[SERVER] %+ [thread %t]");
    spdlog::default_logger()->set_level(spdlog::level::debug);

    const auto args = std::span(argv, size_t(argc));
    if (args.size() < 2) {
        cout << "Usage:\n";
        cout << "If server:  ./networking 127.0.0.1\n";
        cout << "If client:  ./networking 127.0.0.1 /path/to/file\n";
    }

    const string_view ip{args[1]};

    if (argc == 1) {
        Server server;
    } else {
        //client code
    }

    return 0;
}