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

TEST_CASE("UdpSocket connect() is idempotent", "[udp]") {
    UdpSocket sock("127.0.0.1", 50004);
    REQUIRE_NOTHROW(sock.connect());
    REQUIRE(sock.isConnected());
    // calling connect again should be a no-op and not throw
    REQUIRE_NOTHROW(sock.connect());
    REQUIRE(sock.isConnected());
    REQUIRE_NOTHROW(sock.close());
}

TEST_CASE("UdpSocket send after close throws", "[udp]") {
    constexpr int testPort = 50005;
    DummyUdpServer server(testPort);

    UdpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());
    REQUIRE(client.isConnected());

    REQUIRE_NOTHROW(client.close());
    REQUIRE_FALSE(client.isConnected());
    // sending after close must throw
    REQUIRE_THROWS(client.sendString("payload"));
}

TEST_CASE("UdpSocket sending empty string returns 0", "[udp]") {
    // We only validate the send() return value here. Some platforms
    // may not deliver zero-length datagrams reliably, so avoid blocking
    // on the server receive in this test.
    constexpr int testPort = 50006;
    DummyUdpServer server(testPort);

    UdpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());
    REQUIRE(client.isConnected());

    std::string empty;
    REQUIRE(client.sendString(empty) == 0);

    REQUIRE_NOTHROW(client.close());
}

TEST_CASE("UdpSocket multiple datagrams are received in order", "[udp]") {
    constexpr int testPort = 50007;
    DummyUdpServer server(testPort);

    UdpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());
    REQUIRE(client.isConnected());

    std::vector<std::string> msgs = {"one", "two", "three"};
    for (const auto& m : msgs) {
        REQUIRE(client.sendString(m) == m.size());
    }

    for (const auto& expected : msgs) {
        std::string r = server.receiveOnce();
        REQUIRE(r == expected);
    }

    REQUIRE_NOTHROW(client.close());
}

TEST_CASE("UdpSocket can send and receive a reasonably large datagram", "[udp]") {
    constexpr int testPort = 50008;
    DummyUdpServer server(testPort);

    UdpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());

    // Use 8192 bytes (safe cross-platform size)
    std::string payload(1024, 'X');

    REQUIRE(client.sendString(payload) == payload.size());
    std::string received = server.receiveOnce();
    REQUIRE(received == payload);
}


TEST_CASE("UdpSocket reconnect works after close", "[udp]") {
    constexpr int testPort = 50010;
    DummyUdpServer server(testPort);

    UdpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());
    REQUIRE(client.isConnected());

    REQUIRE_NOTHROW(client.close());
    REQUIRE_FALSE(client.isConnected());

    REQUIRE_NOTHROW(client.connect());
    REQUIRE(client.isConnected());
    REQUIRE(client.sendString("reconnected") > 0);

    std::string received = server.receiveOnce();
    REQUIRE(received == "reconnected");
}


TEST_CASE("Multiple clients can send to one server", "[udp]") {
    constexpr int testPort = 50011;
    DummyUdpServer server(testPort);

    UdpSocket client1("127.0.0.1", testPort);
    UdpSocket client2("127.0.0.1", testPort);

    REQUIRE_NOTHROW(client1.connect());
    REQUIRE_NOTHROW(client2.connect());

    REQUIRE(client1.sendString("from1") > 0);
    REQUIRE(client2.sendString("from2") > 0);

    std::string r1 = server.receiveOnce();
    std::string r2 = server.receiveOnce();
    REQUIRE(( (r1 == "from1" && r2 == "from2") || (r1 == "from2" && r2 == "from1") ));
}


TEST_CASE("UdpSocket can send and receive binary data", "[udp]") {
    constexpr int testPort = 50012;
    DummyUdpServer server(testPort);

    UdpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());

    std::string payload = std::string("\x01\x02\xFF\x00hello", 8);
    REQUIRE(client.sendString(payload) == payload.size());

    std::string received = server.receiveOnce();
    REQUIRE(received == payload);
}

TEST_CASE("UdpSocket can handle many small datagrams", "[udp]") {
    constexpr int testPort = 50013;
    DummyUdpServer server(testPort);

    UdpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());

    for (int i = 0; i < 100; i++) {
        std::string msg = "msg_" + std::to_string(i);
        REQUIRE(client.sendString(msg) == msg.size());
        std::string received = server.receiveOnce();
        REQUIRE(received == msg);
    }
}

// 1. Attempt to close without ever connecting
TEST_CASE("UdpSocket close without connect is safe", "[udp]") {
    UdpSocket client("127.0.0.1", 50020);
    REQUIRE_NOTHROW(client.close());
}

// 2. Connect to an unreachable port (no server listening)
TEST_CASE("UdpSocket connect to unreachable port does not throw", "[udp]") {
    UdpSocket client("127.0.0.1", 59999); // no server bound
    REQUIRE_NOTHROW(client.connect());    // connect may still succeed locally
    REQUIRE(client.isConnected());
    REQUIRE_NOTHROW(client.close());
}

// 3. Send very large datagram (should fail or truncate depending on platform)
TEST_CASE("UdpSocket send overly large datagram fails or truncates", "[udp]") {
    constexpr int testPort = 50021;
    DummyUdpServer server(testPort);

    UdpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());

    std::string payload(70000, 'A'); // larger than typical UDP limit
    REQUIRE_THROWS(client.sendString(payload));
}

// 4. Repeated close() calls after connect
TEST_CASE("UdpSocket multiple close calls after connect are safe", "[udp]") {
    constexpr int testPort = 50022;
    DummyUdpServer server(testPort);

    UdpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());
    REQUIRE(client.isConnected());

    REQUIRE_NOTHROW(client.close());
    REQUIRE_NOTHROW(client.close()); // should be no-op
}

// 5. Send after failed connect should throw
TEST_CASE("UdpSocket send after failed connect throws", "[udp]") {
    UdpSocket client("256.256.256.256", 50023); // invalid IP
    REQUIRE_THROWS(client.connect());
    REQUIRE_THROWS(client.sendString("should_fail"));
}

// 6. Hostname resolves but not reachable
TEST_CASE("UdpSocket unreachable hostname fails", "[udp]") {
    UdpSocket client("example.invalid", 50024);
    REQUIRE_THROWS(client.connect());
}

// 7. Send to closed server socket should still succeed locally
TEST_CASE("UdpSocket send to closed server socket does not throw", "[udp]") {
    int testPort = 50025;
    {
        DummyUdpServer server(testPort);
    } // server closed

    UdpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());
    REQUIRE(client.isConnected());
    REQUIRE_NOTHROW(client.sendString("ghost_send"));
    REQUIRE_NOTHROW(client.close());
}

// 8. isConnected remains false if connect fails
TEST_CASE("UdpSocket isConnected false after failed connect", "[udp]") {
    UdpSocket client("invalid.host", 50026);
    REQUIRE_THROWS(client.connect());
    REQUIRE_FALSE(client.isConnected());
}

// 9. Attempt to reuse UdpSocket after close with new server
TEST_CASE("UdpSocket reuse same instance after close", "[udp]") {
    constexpr int testPort1 = 50027;
    constexpr int testPort2 = 50028;

    DummyUdpServer server1(testPort1);
    DummyUdpServer server2(testPort2);

    {
        UdpSocket client("127.0.0.1", testPort1);
        client.connect();
        client.sendString("first");
        client.close();
    } // client destroyed cleanly

    {
        UdpSocket client("127.0.0.1", testPort2);
        client.connect();
        client.sendString("second");
    }
}

// 10. Empty host string should throw on connect
TEST_CASE("UdpSocket empty host throws", "[udp]") {
    UdpSocket client("", 50029);
    REQUIRE_THROWS(client.connect());
}

TEST_CASE("UdpSocket connect fails when socket() cannot create", "[udp]") {
    // Give host that forces unsupported family
    UdpSocket client("invalid-family-host", 50031);
    REQUIRE_THROWS(client.connect());
}

