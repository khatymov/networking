/*! \file main.cpp
 * \brief Entry point.
 */

#include <iostream>
#include <span>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

using namespace std;

int main(int argc, char* argv[]) {
    const auto args = std::span(argv, size_t(argc));
    if (args.size() < 2) {
        cout << "Usage:\n";
        cout << "If server:  ./networking 127.0.0.1\n";
        cout << "If client:  ./networking 127.0.0.1 /path/to/file\n";
    }

    const string_view ip{args[1]};

    boost::asio::io_context io_context;

    return 0;
}