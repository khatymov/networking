//
// Created by Renat_Khatymov on 8/22/2024.
//

#include "server.h"
#include "client_handler.h"

namespace network {

using namespace boost::asio::ip;

//create acceptor
Server::Server(const ConsoleParams& params)
    : acceptor_(ioContext_, tcp::endpoint(make_address(params.ip.data()), params.port)) {

}

void Server::handleConnections() {

}

}