#/**
 * @file UdpTransport.hpp
 * @brief Lightweight UDP transport adapter implementing ITransport.
 *
 * This header declares `UdpTransport`, a tiny adapter that wraps an
 * `UdpSocket` and exposes the `ITransport` interface used by the
 * Sensor application. It provides a blocking, send-all `sendString`
 * convenience and lifecycle methods (connect/close).
 *
 * Usage example:
 * @code{.cpp}
 * auto transport = UdpTransport("127.0.0.1", 5000);
 * transport.connect();
 * transport.sendString("{\"hello\":1}\n");
 * transport.close();
 * @endcode
 *
 * The implementation is intentionally minimal: `connect()` performs
 * address resolution and opens a socket; `sendString()` forwards to
 * the underlying `UdpSocket` which throws on errors.
 *
 * @note This header is part of the SensorApp project.
 */
#pragma once

#include <string>
#include <cstdint>
#include "ITransport.hpp"
#include "UdpSocket.hpp"

class UdpTransport : public ITransport {
public:

    UdpTransport(std::string host, uint16_t port) : socket_{std::move(host), port} {}

    void connect() override                         { socket_.connect(); }
    std::size_t sendString(const std::string& payload) override { return socket_.sendString(payload); }
    void close() override                           { socket_.close(); }
    [[nodiscard]] bool isConnected() const override               { return socket_.isConnected(); }

private:
    UdpSocket socket_;
};
