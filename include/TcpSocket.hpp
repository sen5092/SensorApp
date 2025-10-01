// TcpSocket.hpp
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

class TcpSocket : public ITransport {
public:
    // construct with destination
    TcpSocket(std::string host, int32_t port);
    ~TcpSocket();

    // no copies (socket ownership)
    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;

    // connect to host:port (blocking). Throws on failure.
    void connect();

    // true if a socket is currently open
    bool isConnected() const noexcept;

    // blocking send; attempts to write all bytes. Throws on failure.
    std::size_t send(const void* data, std::size_t len);

    // convenience for text payloads (e.g., JSON)
    std::size_t sendString(const std::string& s);

    // close the socket (safe to call multiple times)
    void close() noexcept;

private:
    std::string host_;
    int32_t     port_;
    int         fd_{-1};   // POSIX socket fd; -1 means "not connected"
};
