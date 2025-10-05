/**
 * @file UdpSocket.hpp
 * @brief Lightweight RAII wrapper for UDP socket communication.
 *
 * Encapsulates creation, binding, and transmission of UDP datagrams using
 * BSD sockets. Supports both sending and receiving modes. Provides move
 * semantics for ownership transfer of the underlying socket descriptor.
 *
 * @throws std::runtime_error on socket creation or binding failure.
 * @note The socket is automatically closed on destruction.
 */

#pragma once

#include <string>
#include <cstddef>
#include <cstdint>
#include <utility>

class UdpSocket {
public:
    UdpSocket(std::string host, uint16_t port);
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    // Movable: transfer ownership of the underlying socket fd_
    UdpSocket(UdpSocket&& other) noexcept
        : host_(std::move(other.host_)), port_(other.port_), fd_(other.fd_) {
        other.fd_ = -1;
    }

    UdpSocket& operator=(UdpSocket&& other) noexcept {
        if (this != &other) {
            // best-effort cleanup of our current resource
            close();
            host_ = std::move(other.host_);
            port_ = other.port_;
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    void connect();                       // resolve + create datagram socket + ::connect (sets default peer)
    [[nodiscard]] bool isConnected() const noexcept;    // fd_ >= 0
    std::size_t send(const void* data, std::size_t len) const; // send one datagram
    [[nodiscard]] std::size_t sendString(const std::string& payload) const;        // convenience
    void close() noexcept;                // shutdown (best-effort) + close

private:
    std::string host_;
    uint16_t     port_;
    int         fd_{-1};                  // POSIX socket fd; -1 = not connected
};
