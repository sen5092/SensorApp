/**
 * @file TcpSocket.hpp
 * @brief RAII wrapper for TCP socket operations.
 *
 * Provides low-level send and receive functionality for TCP connections,
 * including connection establishment, error translation, and automatic
 * cleanup. Designed for use by higher-level transport classes such as
 * TcpTransport.
 *
 * @note The socket is automatically closed when the object is destroyed.
 */

#pragma once

#include "ITransport.hpp"

#include <string>
#include <cstddef>   // std::size_t
#include <cstdint>   // int32_t

// Super-simple, blocking TCP client.
// Usage:
//   TcpSocket cli{"127.0.0.1", 8080};
//   cli.connect();
//   cli.sendString(payload);
//   cli.close();

class TcpSocket {
public:
    // construct with destination
    TcpSocket(std::string host, uint16_t port);
    ~TcpSocket();

    // no copies (socket ownership)
    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;

        // Movable: transfer ownership of the underlying socket fd_
    TcpSocket(TcpSocket&& other) noexcept
        : host_(std::move(other.host_)), port_(other.port_), fd_(other.fd_) {
        other.fd_ = -1;
    }

    TcpSocket& operator=(TcpSocket&& other) noexcept {
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

    // connect to host:port (blocking). Throws on failure.
    void connect();

    // true if a socket is currently open
    [[nodiscard]] bool isConnected() const noexcept;

    // blocking send; attempts to write all bytes. Throws on failure.
    std::size_t send(const void* data, std::size_t len) const;

    // convenience for text payloads (e.g., JSON)
    [[nodiscard]] std::size_t sendString(const std::string& payload) const;

    // close the socket (safe to call multiple times)
    void close() noexcept;

private:
    std::string host_;
    uint16_t     port_;
    int         fd_{-1};   // POSIX socket fd; -1 means "not connected"
};
