#include <iostream>
#include <vector>

#include <asio.hpp>

constexpr unsigned short UDP_DST_PORT = 1234;
constexpr unsigned short BUFFER_COUNT = 128;

struct PrivateData {
    size_t i;
    char buffer[2048];
};

void handle_receive(PrivateData* context, asio::ip::udp::socket& socket, asio::error_code, size_t bytes_transferred)
{
    std::cout << "handle_receive: context is: " << context << std::endl;
    std::cout << "  -> i:                 " << context->i << std::endl;
    std::cout << "  -> bytes_transferred: " << bytes_transferred << std::endl;

    size_t buffer_i = 0;
    std::memcpy(&buffer_i, &context->buffer[0], sizeof(buffer_i));
    std::cout << "  -> buffer_i:          " << buffer_i << std::endl;

    // Send answer
    socket.async_send_to(
        asio::const_buffer(&context->i, sizeof(context->i)),
        asio::ip::udp::endpoint(asio::ip::udp::v4(), UDP_DST_PORT),
        [](asio::error_code, size_t bytes_transferred) {
            std::cout << "handle_receive: async_send_to: bytes_transferred: " << bytes_transferred << std::endl;
        });

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

    socket.open(asio::ip::udp::v4());
    socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), UDP_DST_PORT));

    std::vector<PrivateData*> private_data;

    // Create and enqueue some buffer
    for (size_t i = 0; i < BUFFER_COUNT; i++) {
        auto context = new PrivateData;
        context->i = i;
        private_data.push_back(context);

        std::cout << "Context " << i << " is " << context << std::endl;

        socket.async_receive(
            asio::buffer(context->buffer, sizeof(context->buffer)),
            std::bind(
                handle_receive,
                context,
                std::ref(socket),
                asio::placeholders::error,
                asio::placeholders::bytes_transferred));
    }

    // Initial send
    size_t initial = 0;
    socket.async_send_to(
        asio::const_buffer(&initial, sizeof(initial)),
        asio::ip::udp::endpoint(asio::ip::udp::v4(), UDP_DST_PORT),
        [](asio::error_code, size_t bytes_transferred) {
            std::cout << "main: async_send_to: bytes_transferred: " << bytes_transferred << std::endl;
        });

    // Run for some time
    auto count1 = io_context.run_for(std::chrono::milliseconds(100));
    auto count2 = io_context.run_for(std::chrono::milliseconds(100));
    auto count3 = io_context.run_for(std::chrono::milliseconds(100));
    std::cout << "counts:\n";
    std::cout << "  1: " << count1 << "\n";
    std::cout << "  2: " << count2 << "\n";
    std::cout << "  3: " << count3 << "\n";

    return 0;
}
