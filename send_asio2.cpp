#include <iostream>
#include <vector>
#include <span>

#include <asio.hpp>

#include "send_recv.h"

size_t packet_number = 0;
char buffer[128];
asio::ip::address host;

void send_next(asio::ip::udp::socket& socket, const asio::error_code& /*ec*/, size_t bytes_transferred)
{
    packet_number++;
    std::memcpy(&buffer[0], &packet_number, sizeof(packet_number));

    socket.async_send_to(
        asio::buffer(buffer, sizeof(buffer)),
        asio::ip::udp::endpoint(host, UDP_DST_PORT),
        std::bind(send_next, std::ref(socket), asio::placeholders::error, asio::placeholders::bytes_transferred));
}

int main()
{
    host = asio::ip::make_address("10.1.2.3");

    asio::io_context io_context;
    asio::ip::udp::socket socket(io_context);

    socket.open(asio::ip::udp::v4());
    socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), UDP_SRC_PORT));

    packet_number++;
    std::memcpy(&buffer[0], &packet_number, sizeof(packet_number));

    socket.async_send_to(
        asio::buffer(buffer, sizeof(buffer)),
        asio::ip::udp::endpoint(host, UDP_DST_PORT),
        std::bind(send_next, std::ref(socket), asio::placeholders::error, asio::placeholders::bytes_transferred));

    // run the async thing
    io_context.run();

    return 0;
}
