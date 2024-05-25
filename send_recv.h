#ifndef SEND_RECV_H__INCLUDED
#define SEND_RECV_H__INCLUDED

constexpr unsigned short UDP_DST_PORT = 0x4321;
constexpr unsigned short UDP_SRC_PORT = 0x1234;

// Setup buffers
constexpr size_t MAX_OUTSTANDING_REQUESTS = 512;
constexpr size_t MAX_PACKET_LENGTH = 1024;

#include <chrono>
using wall_clock = std::chrono::steady_clock;

#endif
