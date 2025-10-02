// UdpSocket.cpp
#include "UdpSocket.hpp"

#include <stdexcept>
#include <string>
#include <cstring>      // std::strerror, std::memset
#include <cerrno>
#include <unistd.h>     // ::close, ::shutdown
#include <sys/types.h>
#include <sys/socket.h> // ::socket, ::connect, ::send, SOCK_DGRAM
#include <netdb.h>      // ::getaddrinfo, ::freeaddrinfo, addrinfo

// turn errno into an exception with context
static std::runtime_error sys_err(const std::string& where) {
    return std::runtime_error(where + ": " + std::strerror(errno));
}

UdpSocket::UdpSocket(std::string host, int32_t port)
    : host_(std::move(host)), port_(port) {}

UdpSocket::~UdpSocket() { close(); }

void UdpSocket::connect() {
    if (isConnected()) return;

    // Resolve destination (IPv4 or IPv6) for UDP
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;   // allow v4 or v6
    hints.ai_socktype = SOCK_DGRAM;  // UDP
    hints.ai_protocol = 0;

    char portStr[16];
    std::snprintf(portStr, sizeof(portStr), "%d", port_);

    struct addrinfo* results = nullptr;
    int rc = ::getaddrinfo(host_.c_str(), portStr, &hints, &results);
    if (rc != 0) {
        std::string msg = "getaddrinfo('" + host_ + "', " + portStr + "): ";
        msg += ::gai_strerror(rc);
        throw std::runtime_error(msg);
    }

    int lastErr = 0;
    for (struct addrinfo* ai = results; ai; ai = ai->ai_next) {
        int s = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (s < 0) { lastErr = errno; continue; }

        // For UDP, calling ::connect sets the default peer so we can use ::send()
        if (::connect(s, ai->ai_addr, ai->ai_addrlen) == 0) {
            fd_ = s;
            ::freeaddrinfo(results);
            return;
        }

        lastErr = errno;
        ::close(s);
    }

    ::freeaddrinfo(results);
    errno = lastErr ? lastErr : ECONNREFUSED;
    throw sys_err("udp connect");
}

bool UdpSocket::isConnected() const noexcept { return fd_ >= 0; }

std::size_t UdpSocket::send(const void* data, std::size_t len) {
    if (!isConnected()) throw std::runtime_error("udp send: not connected");

    const char* p = static_cast<const char*>(data);

    for (;;) {
        ssize_t n = ::send(fd_, p, len, 0);
        if (n >= 0) {
            // For UDP, partial sends are not expected; verify all-or-error.
            if (static_cast<std::size_t>(n) != len) {
                throw std::runtime_error("udp send: short datagram send");
            }
            return static_cast<std::size_t>(n);
        }
        if (errno == EINTR) continue; // interrupted by signal → retry
        throw sys_err("udp send");
    }
}

std::size_t UdpSocket::sendString(const std::string& s) {
    return send(s.data(), s.size());
}

void UdpSocket::close() noexcept {
    if (fd_ < 0) return;
    // shutdown is optional for UDP; do best-effort then close
    ::shutdown(fd_, SHUT_RDWR);
    ::close(fd_);
    fd_ = -1;
}
