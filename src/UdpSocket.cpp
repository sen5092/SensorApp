// UdpSocket.cpp
#include "UdpSocket.hpp"

#include <unistd.h>  // for ssize_t
#include <stdexcept>
#include <string>
#include <utility>      // std::move
#include <cstring>      // std::strerror, std::memset
#include <cerrno>
#include <cstdint>      // int32_t
#include <sys/socket.h> // ::socket, ::connect, ::send, SOCK_DGRAM
#include <netdb.h>      // ::getaddrinfo, ::freeaddrinfo, addrinfo

namespace {

    // turn errno into an exception with context
    std::runtime_error systemErr(const std::string& where) {
        return std::runtime_error(where + ": " + std::strerror(errno));
    }
}

UdpSocket::UdpSocket(std::string host, uint16_t port)
    : host_(std::move(host)), port_(port) {}

UdpSocket::~UdpSocket() {
    close();
}

void UdpSocket::connect() {

    if (isConnected()) {
        return;
    }

    // Resolve destination (IPv4 or IPv6) for UDP
    struct addrinfo hints = {};
    hints.ai_family   = AF_UNSPEC;   // allow v4 or v6
    hints.ai_socktype = SOCK_DGRAM;  // UDP
    hints.ai_protocol = 0;


    const std::string portStr = std::to_string(port_);
    struct addrinfo* results = nullptr;
    const int rtnCode = ::getaddrinfo(host_.c_str(), portStr.c_str(), &hints, &results);
    if (rtnCode != 0) {
        std::string msg = "getaddrinfo('" + host_ + "', " + portStr + "): ";
        msg += ::gai_strerror(rtnCode);
        throw std::runtime_error(msg);
    }

    int lastErr = 0;
    for (struct addrinfo* ai = results; ai != nullptr; ai = ai->ai_next) {

        const int sock = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sock < 0) {
            lastErr = errno;
            continue;
        }

        // For UDP, calling ::connect sets the default peer so we can use ::send()
        if (::connect(sock, ai->ai_addr, ai->ai_addrlen) == 0) {
            fd_ = sock;
            ::freeaddrinfo(results);
            return;
        }

        lastErr = errno;
        ::close(sock);
    }

    ::freeaddrinfo(results);
    errno = (lastErr != 0) ? lastErr : ECONNREFUSED;
    throw systemErr("udp connect");
}

bool UdpSocket::isConnected() const noexcept {
    return fd_ >= 0;
}

std::size_t UdpSocket::send(const void* data, std::size_t len) const {

    if (!isConnected()) {
        throw std::runtime_error("udp send: not connected");
    }

    const char* dataPtr = static_cast<const char*>(data);

    for (;;) {
        const ssize_t bytesSent = ::send(fd_, dataPtr, len, 0);
        if (bytesSent >= 0) {
            // For UDP, partial sends are not expected; verify all-or-error.
            if (static_cast<std::size_t>(bytesSent) != len) {
                throw std::runtime_error("udp send: short datagram send");
            }
            return static_cast<std::size_t>(bytesSent);
        }
        if (errno == EINTR) {
            continue; // interrupted by signal → retry
        }
        throw systemErr("udp send");
    }
}

std::size_t UdpSocket::sendString(const std::string& payload) const {
    return send(payload.data(), payload.size());
}

void UdpSocket::close() noexcept {
    if (fd_ < 0) {
        return;
    }
    // shutdown is optional for UDP; do best-effort then close
    ::shutdown(fd_, SHUT_RDWR);
    ::close(fd_);
    fd_ = -1;
}
