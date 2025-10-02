#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "UdpSocket.hpp"

#include <thread>
#include <chrono>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// Simple UDP server helper for tests
class DummyUdpServer {
public:
    explicit DummyUdpServer(int port) : port_(port) {
        fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
        REQUIRE(fd_ >= 0);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port_);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        int opt = 1;
        ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        REQUIRE(::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0);
    }

    ~DummyUdpServer() {
        if (fd_ >= 0) ::close(fd_);
    }

    std::string receiveOnce() {
        char buffer[1024];
        sockaddr_in src{};
        socklen_t srclen = sizeof(src);

        ssize_t n = ::recvfrom(fd_, buffer, sizeof(buffer), 0,
                               reinterpret_cast<sockaddr*>(&src), &srclen);
        REQUIRE(n >= 0);

        return std::string(buffer, buffer + n);
    }

private:
    int fd_{-1};
    int port_;
};

// ----------------------------------------------------------
// Tests
// ----------------------------------------------------------

TEST_CASE("UdpSocket connect to invalid host fails", "[udp]") {
    UdpSocket sock("nonexistent.localdomain", 50000);
    REQUIRE_THROWS(sock.connect()); // should throw runtime_error
}

TEST_CASE("UdpSocket refuses send before connect", "[udp]") {
    UdpSocket sock("127.0.0.1", 50001);
    REQUIRE_FALSE(sock.isConnected());
    REQUIRE_THROWS(sock.sendString("hello"));
}

TEST_CASE("UdpSocket can send datagram to server", "[udp]") {
    constexpr int testPort = 50002;
    DummyUdpServer server(testPort);

    UdpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());
    REQUIRE(client.isConnected());

    std::string payload = "hello_udp";
    REQUIRE(client.sendString(payload) == payload.size());

    std::string received = server.receiveOnce();
    REQUIRE(received == payload);
}

TEST_CASE("UdpSocket close() is safe to call multiple times", "[udp]") {
    UdpSocket sock("127.0.0.1", 50003);
    REQUIRE_NOTHROW(sock.close()); // before connect
    REQUIRE_NOTHROW(sock.close()); // again after already closed
}
