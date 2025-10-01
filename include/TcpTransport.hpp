// TcpTransport.hpp
#pragma once

#include <string>
#include <cstdint>
#include "ITransport.hpp"
#include "TcpSocket.hpp"

class TcpTransport : public ITransport {
public:
    TcpTransport(std::string host, int32_t port) : client_{std::move(host), port} {}

    void connect() override            { client_.connect(); }
    std::size_t sendString(const std::string& payload) override { return client_.sendString(payload); }
    void close() override              { client_.close(); }
    bool isConnected() const override  { return client_.isConnected(); }

private:
    TcpSocket client_;
};
