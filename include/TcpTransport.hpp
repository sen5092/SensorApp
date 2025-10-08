/**
 * @file TcpTransport.hpp
 * @brief Transport layer implementation for sending data over TCP.
 *
 * Handles the creation and management of TCP connections for transmitting
 * serialized sensor data. Encapsulates connection lifecycle, error handling,
 * and socket cleanup to provide a robust communication interface.
 *
 * @throws std::runtime_error on socket or connection failure.
 */

#pragma once

#include <string>
#include <cstdint>
#include "ITransport.hpp"
#include "TcpSocket.hpp"

class TcpTransport : public ITransport {
public:

    TcpTransport(std::string host, u_int16_t port) : socket_{std::move(host), port} {}

    void connect() override            { socket_.connect(); }
    std::size_t sendString(const std::string& payload) override { return socket_.sendString(payload); }
    void close() override              { socket_.close(); }
    [[nodiscard]] bool isConnected() const override  { return socket_.isConnected(); }

private:
    TcpSocket socket_;
};
