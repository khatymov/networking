
#pragma once


#include "common.h"

using namespace boost::asio;

using boost::asio::buffer;
using boost::asio::read;
using boost::asio::streambuf;
using boost::asio::transfer_exactly;
using boost::asio::write;
using boost::system::error_code;


[[nodiscard]] static bool writeToSocket(ip::tcp::socket& socket, const Packet& packet, boost::system::error_code& ec) {
    //send header
    auto sentHeaderSize = write(socket, boost::asio::buffer(&packet.header, sizeof(packet.header)), transfer_exactly(sizeof(packet.header)), ec);
    if (ec)
    {
        spdlog::error("Send packet header error: {}", ec.message());
        return false;
    }
    //send payload
    if (packet.header.length > 0) {
        auto sentPayloadSize = write(socket, boost::asio::buffer(&packet.payload, packet.header.length), transfer_exactly(packet.header.length), ec);
        if (ec)
        {
            spdlog::error("Send packet payload error: {}", ec.message());
            return false;
        }
        std::string str(packet.payload, sentPayloadSize);
        spdlog::info("Send packet: {}, size: {}", str, sentPayloadSize);
    }


    return true;
}

[[nodiscard]] static bool readFromSocket(ip::tcp::socket& socket, Packet& packet, boost::system::error_code& ec) {
    //read header
    auto head_size = read(socket, buffer(&packet.header, sizeof(packet.header)), transfer_exactly(sizeof(packet.header)), ec);

    if (ec)
    {
        spdlog::error("Read packet header error: {}", ec.message());
        return false;
    }
    //read payload
    auto payload_size = read(socket, boost::asio::buffer(&packet.payload, packet.header.length), transfer_exactly(packet.header.length), ec);
    if (ec)
    {
        spdlog::error("Read packet payload error: {}", ec.message());
        return false;
    }
    return true;
}