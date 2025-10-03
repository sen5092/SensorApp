// TcpSocket.cpp
#include "TcpSocket.hpp"
#include "ITransport.hpp"

#include <stdexcept>      // std::runtime_error
#include <string>
#include <cstring>        // std::strerror, std::memset
#include <cerrno>         // errno
#include <unistd.h>       // ::close, ::shutdown, ::write
#include <sys/types.h>
#include <sys/socket.h>   // ::socket, ::connect, ::send, SOL_SOCKET, etc.
#include <netdb.h>        // ::getaddrinfo, ::freeaddrinfo, addrinfo

// ---------- small helpers: turn errno into a readable message ----------
static std::runtime_error sys_err(const std::string& where) {
    return std::runtime_error(where + ": " + std::strerror(errno));
}

// ---------- ctor / dtor ----------

/*
 * Constructor
 * - Just stores the host and port you want to connect to.
 * - Does NOT create the socket or touch the network yet.
 */
TcpSocket::TcpSocket(std::string host, uint16_t port)
    : host_(std::move(host)), port_(port), fd_(-1) {}

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

    // Prepare hints for getaddrinfo: we want a TCP stream socket, any family.
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;   // allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags    = 0;
    hints.ai_protocol = 0;           // any

    // Port must be a C-string; we’ll format it from the int.
    char portStr[16];
    std::snprintf(portStr, sizeof(portStr), "%d", port_);

    struct addrinfo* results = nullptr;
    int gai_rc = ::getaddrinfo(host_.c_str(), portStr, &hints, &results);
    if (gai_rc != 0) {
        // getaddrinfo has its own error strings separate from errno.
        std::string msg = "getaddrinfo('" + host_ + "', " + portStr + "): ";
        msg += ::gai_strerror(gai_rc);
        throw std::runtime_error(msg);
    }

    // Walk the result list, trying to connect to each address until one works.
    int lastErrno = 0;
    for (struct addrinfo* ai = results; ai != nullptr; ai = ai->ai_next) {
        // Create a socket for this candidate address.
        int s = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (s < 0) {
            lastErrno = errno;
            continue; // try next candidate
        }

        // Try to connect.
        if (::connect(s, ai->ai_addr, ai->ai_addrlen) == 0) {
            // Success! keep the socket and stop trying others.
            fd_ = s;
            ::freeaddrinfo(results);
            return;
        }

        // Failed to connect this candidate; remember errno, close, and continue.
        lastErrno = errno;
        ::close(s);
    }

    ::freeaddrinfo(results);

    // If we got here, all candidates failed.
    errno = lastErrno ? lastErrno : ECONNREFUSED;
    throw sys_err("connect");
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
    if (fd_ < 0) return;

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
std::size_t TcpSocket::send(const void* data, std::size_t len) {
    if (!isConnected()) {
        throw std::runtime_error("send: not connected");
    }

    const char* p = static_cast<const char*>(data);
    std::size_t bytesLeft = len;

    while (bytesLeft > 0) {
        // ::send may write fewer bytes than requested; we must loop.
        ssize_t n = ::send(fd_, p, bytesLeft, 0);

        if (n > 0) {
            p += n;
            bytesLeft -= static_cast<std::size_t>(n);
            continue;
        }

        if (n == 0) {
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
        throw sys_err("send");
    }

    // If we sent everything, return 'len'.
    return len;
}

/*
 * sendString(const std::string& s)
 * - Convenience wrapper for text payloads (e.g., your JSON).
 * - Calls the raw send() with the string’s bytes.
 */
std::size_t TcpSocket::sendString(const std::string& s) {
    return send(s.data(), s.size());
}
