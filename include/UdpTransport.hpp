// UdpTransport.hpp
#pragma once

#include <string>
#include <cstdint>
#include "ITransport.hpp"
#include "UdpSocket.hpp"

class UdpTransport : public ITransport {
public:

    UdpTransport(std::string host, uint16_t port) : socket_{std::move(host), port} {}

    void connect() override                         { socket_.connect(); }
    std::size_t sendString(const std::string& s) override { return socket_.sendString(s); }
    void close() override                           { socket_.close(); }
    bool isConnected() const override               { return socket_.isConnected(); }

private:
    UdpSocket socket_;
};
