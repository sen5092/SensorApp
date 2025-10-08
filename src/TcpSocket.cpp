/**
 * @file TcpSocket.cpp
 * @brief Implementation of the TCP socket wrapper.
 *
 * Defines the TcpSocket class methods responsible for creating,
 * connecting, sending, and receiving data using TCP sockets.
 * Includes robust error handling and RAII-based cleanup of
 * the underlying socket descriptor.
 *
 * @see TcpSocket
 */

#include "TcpSocket.hpp"

#include <stdexcept>      // std::runtime_error
#include <string>
#include <cstring>        // std::strerror, std::memset
#include <cerrno>         // errno
#include <unistd.h>       // ::close, ::shutdown, ::write
#include <sys/socket.h>   // ::socket, ::connect, ::send, SOL_SOCKET, etc.
#include <netdb.h>        // ::getaddrinfo, ::freeaddrinfo, addrinfo
#include <cstdint>        // int32_t
#include <utility>       // std::move
#include <cstddef>       // std::size_t
#include <iterator>

// ---------- small helpers: turn errno into a readable message ----------
namespace {

    // turn errno into an exception with context
    std::runtime_error systemErr(const std::string& where) {
        return std::runtime_error(where + ": " + std::strerror(errno));
    }
}

// ---------- ctor / dtor ----------

/*
 * Constructor
 * - Just stores the host and port you want to connect to.
 * - Does NOT create the socket or touch the network yet.
 */
TcpSocket::TcpSocket(std::string host, uint16_t port)
    : host_(std::move(host)), port_(port) {}

/*
 * Destructor
 * - Make sure the socket is closed if the user forgot to call close().
 * - Never throw from a destructor; we use noexcept close().
 */
TcpSocket::~TcpSocket() {
    close();
}

// ---------- connection management ----------

/*
 * connect()
 * - Resolve host + port into one or more address candidates (IPv4/IPv6).
 * - Try each candidate: create a socket, attempt ::connect().
 * - On first success, keep that socket and return.
 * - On failure for all candidates, throw.
 */
void TcpSocket::connect() {

    if (isConnected()) {
        return; // already connected; no-op
    }

    if (host_.empty()) {
        throw std::invalid_argument("TcpSocket: host cannot be empty");
    }

    // Prepare hints for getaddrinfo: we want a TCP stream socket, any family.
    struct addrinfo hints = {};
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;   // allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags    = 0;
    hints.ai_protocol = 0;           // any


    const std::string portStr = std::to_string(port_);
    struct addrinfo* results = nullptr;
    const int rtnCode = ::getaddrinfo(host_.c_str(), portStr.c_str(), &hints, &results);
    if (rtnCode != 0) {
        // getaddrinfo has its own error strings separate from errno.
        std::string msg = "getaddrinfo('" + host_ + "', " + portStr + "): ";
        msg += ::gai_strerror(rtnCode);
        throw std::runtime_error(msg);
    }

    // Walk the result list, trying to connect to each address until one works.
    int lastErrno = 0;
    for (struct addrinfo* ai = results; ai != nullptr; ai = ai->ai_next) {
        // Create a socket for this candidate address.
        const int sock = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sock < 0) {
            lastErrno = errno;
            continue; // try next candidate
        }

        // Try to connect.
        if (::connect(sock, ai->ai_addr, ai->ai_addrlen) == 0) {
            // Success! keep the socket and stop trying others.
            fd_ = sock;
            ::freeaddrinfo(results);
            return;
        }

        // Failed to connect this candidate; remember errno, close, and continue.
        lastErrno = errno;
        ::close(sock);
    }

    ::freeaddrinfo(results);

    // If we got here, all candidates failed.
    errno = (lastErrno != 0) ? lastErrno : ECONNREFUSED;
    throw systemErr("connect");
}

/*
 * isConnected()
 * - True if we have an open socket file descriptor (fd_ >= 0).
 */
bool TcpSocket::isConnected() const noexcept {
    return fd_ >= 0;
}

/*
 * close()
 * - Safe to call multiple times (idempotent).
 * - If connected, try a graceful shutdown then close the fd.
 * - Never throws (noexcept).
 */
void TcpSocket::close() noexcept {
    if (fd_ < 0) {
        return;
    }

    // Try to be polite: shutdown both directions.
    // If it fails (e.g., already closed by peer), we still proceed to ::close.
    ::shutdown(fd_, SHUT_RDWR);
    ::close(fd_);
    fd_ = -1;
}

// ---------- sending data ----------

/*
 * send(const void* data, std::size_t len)
 * - Blocking, "send-all" semantics:
 *   We loop until all 'len' bytes have been written or an error occurs.
 * - Handles partial writes (common in TCP) and EINTR (interrupted syscalls).
 * - On any real error, throws std::runtime_error.
 * - Returns total bytes written (== len on success).
 */
std::size_t TcpSocket::send(const void* data, std::size_t len) const {
    if (!isConnected()) {
        throw std::runtime_error("send: not connected");
    }

    const char* dataPtr = static_cast<const char*>(data);
    std::size_t bytesLeft = len;

    while (bytesLeft > 0) {
        // ::send may write fewer bytes than requested; we must loop.
        const ssize_t bytesSent = ::send(fd_, dataPtr, bytesLeft, 0);   // NOLINT(misc-include-cleaner)

        if (bytesSent > 0) {
            dataPtr = std::next(dataPtr, bytesSent);
            bytesLeft -= static_cast<std::size_t>(bytesSent);
            continue;
        }

        if (bytesSent == 0) {
            // 0 from send() is unusual; treat as connection issue.
            throw std::runtime_error("send: connection closed by peer");
        }

        // n < 0 -> check errno for what happened.
        if (errno == EINTR) {
            // Interrupted by a signal; retry the send.
            continue;
        }

        // EAGAIN/EWOULDBLOCK would matter on non-blocking sockets.
        // We’re blocking, so treat other errors as fatal.
        throw systemErr("send");
    }

    // If we sent everything, return 'len'.
    return len;
}

/*
 * sendString(const std::string& s)
 * - Convenience wrapper for text payloads (e.g., your JSON).
 * - Calls the raw send() with the string’s bytes.
 */
std::size_t TcpSocket::sendString(const std::string& payload) const {
    return send(payload.data(), payload.size());
}
