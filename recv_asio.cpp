
#include <iostream>
#include <vector>
#include <span>

#include <asio.hpp>

#include "send_recv.h"

// Statistics

struct Statistics {
    size_t bytes_transferred = 0;
    size_t packets_sent = 0;
    wall_clock::time_point last_time = wall_clock::now();
};
Statistics statistics;

struct PrivateData {
    size_t i;
    std::span<char> buffer;
};

void show_statistics(const asio::error_code& /*ec*/, asio::steady_timer& t)
{
    auto now = wall_clock::now();

    using namespace std::literals::chrono_literals;
    auto diff_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - statistics.last_time);
    auto bit_rate = (8 * 1000.0 * statistics.bytes_transferred / diff_time_ms.count());
    auto packet_rate = (1000.0 * statistics.packets_sent / diff_time_ms.count());
    std::cout << "Received " << statistics.bytes_transferred << " bytes (" << statistics.packets_sent << " packets) in "
              << diff_time_ms.count();
    std::cout << "  => " << packet_rate << " pkt/s or " << bit_rate << " bit/s" << std::endl;

    // next cycle
    statistics.last_time = now;
    statistics.bytes_transferred = 0;
    statistics.packets_sent = 0;

    // re-schedule timer
    t.expires_at(t.expiry() + asio::chrono::seconds(1));
    t.async_wait(std::bind(show_statistics, asio::placeholders::error, std::ref(t)));
}


void handle_receive(PrivateData* context, asio::ip::udp::socket& socket, asio::error_code, size_t bytes_transferred)
{
    statistics.bytes_transferred += bytes_transferred;
    statistics.packets_sent++;

    // re-enqueue
    socket.async_receive(
        asio::buffer(context->buffer, sizeof(context->buffer)),
        std::bind(
            handle_receive,
            context,
            std::ref(socket),
            asio::placeholders::error,
            asio::placeholders::bytes_transferred));
}

int main()
{
    asio::io_context io_context;
    asio::ip::udp::socket socket(io_context);
    asio::ip::udp::endpoint endpoint(asio::ip::udp::v4(), UDP_DST_PORT);

    socket.open(asio::ip::udp::v4());
    socket.bind(endpoint);

    std::vector<PrivateData*> private_data;
    auto buffer = std::make_unique<char[]>(MAX_OUTSTANDING_REQUESTS * MAX_PACKET_LENGTH);

    // Create and enqueue some buffer
    for (size_t i = 0; i < MAX_OUTSTANDING_REQUESTS; i++) {
        auto context = new PrivateData {
            .i = i,
            .buffer = std::span {&buffer[i * MAX_PACKET_LENGTH], MAX_PACKET_LENGTH},
        };
        private_data.push_back(context);

        socket.async_receive(
            asio::buffer(context->buffer.data(), context->buffer.size_bytes()),
            std::bind(
                handle_receive,
                context,
                std::ref(socket),
                asio::placeholders::error,
                asio::placeholders::bytes_transferred));
    }

    // statistics
    asio::steady_timer t(io_context, asio::chrono::seconds(1));
    statistics.last_time = wall_clock::now();
    t.async_wait(std::bind(show_statistics, asio::placeholders::error, std::ref(t)));

    // run the async thing
    io_context.run();

    return 0;
}
