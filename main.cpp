/*! \file main.cpp
 * \brief Entry point.
 */

#include <iostream>
#include <span>

#include "pch.h"

#include "consoleParams.h"

#include "client_handler.h"
#include "server.h"

using namespace std;
using namespace network;

int main(int argc, char* argv[]) {

    spdlog::default_logger()->set_level(spdlog::level::debug);

    const auto args = std::span(argv, size_t(argc));
    if (args.size() < 2) {
        cout << "Usage:\n";
        cout << "If server:  ./networking 127.0.0.1 1234\n";
        cout << "If client:  ./networking 127.0.0.1 1234 /path/to/file\n";
    }
    // arg1 = ip | arg2 = port number
    ConsoleParams consoleParams(args[1], args[2]);

    // only client has file as a parameter
    if (args.size() == 4) {
        consoleParams.targetFile = args[3];
    }

    try {
        if (consoleParams.isClient()) {
            ClientHandler clientHandler(consoleParams);
            clientHandler.handle();
        } else {
            Server server(consoleParams);
            server.handleConnections();
        }
    } catch (const std::runtime_error& error) {
        cerr << "Runtime error in main: " << error.what() << endl;
    } catch (const std::exception& error) {
        cerr << "Exception in main: " << error.what() << endl;
    }


    return 0;
}