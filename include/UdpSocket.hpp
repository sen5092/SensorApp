// UdpSocket.hpp
#pragma once

#include <string>
#include <cstddef>
#include <cstdint>

class UdpSocket {
public:
    UdpSocket(std::string host, int32_t port);
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    void connect();                       // resolve + create datagram socket + ::connect (sets default peer)
    bool isConnected() const noexcept;    // fd_ >= 0
    std::size_t send(const void* data, std::size_t len); // send one datagram
    std::size_t sendString(const std::string& s);        // convenience
    void close() noexcept;                // shutdown (best-effort) + close

private:
    std::string host_;
    int32_t     port_;
    int         fd_{-1};                  // POSIX socket fd; -1 = not connected
};
