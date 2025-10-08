
/**
 * @file test_TcpSocket.cpp
 * @brief Unit tests for TcpSocket class using Catch2 v3.
 *
 * These tests validate the behavior of the TcpSocket wrapper under various
 * normal and error conditions. Because TcpSocket requires a real TCP server
 * to connect to, we simulate connections using localhost and dummy ports.
 */

#include "TcpSocket.hpp"
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>

// Helper to spin up a minimal server that accepts one connection
static void runDummyServer(uint16_t port) {
    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    REQUIRE(server_fd >= 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    REQUIRE(::bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == 0);
    REQUIRE(::listen(server_fd, 1) == 0);

    // Accept one connection and close
    int client_fd = ::accept(server_fd, nullptr, nullptr);
    if (client_fd >= 0) ::close(client_fd);
    ::close(server_fd);
}

TEST_CASE("TcpSocket connects and sends", "[TcpSocket]") {
    const uint16_t testPort = 45678;

    std::thread serverThread(runDummyServer, testPort);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    TcpSocket client("127.0.0.1", testPort);
    REQUIRE_NOTHROW(client.connect());
    REQUIRE(client.isConnected());

    REQUIRE_NOTHROW(client.sendString("Hello, server!"));

    client.close();
    REQUIRE_FALSE(client.isConnected());

    serverThread.join();
}

TEST_CASE("TcpSocket fails to connect with invalid host", "[TcpSocket]") {
    TcpSocket client("invalid.localhost", 12345);
    REQUIRE_THROWS_AS(client.connect(), std::runtime_error);
}

TEST_CASE("TcpSocket fails to connect with no listener", "[TcpSocket]") {
    TcpSocket client("127.0.0.1", 54321); // assume this port is closed
    REQUIRE_THROWS_AS(client.connect(), std::runtime_error);
}

TEST_CASE("TcpSocket handles double close gracefully", "[TcpSocket]") {
    TcpSocket client("127.0.0.1", 12345);
    client.close();
    REQUIRE_NOTHROW(client.close());
}

TEST_CASE("TcpSocket throws when sending without connect", "[TcpSocket]") {
    TcpSocket client("127.0.0.1", 12345);
    REQUIRE_THROWS_WITH(client.sendString("fail"), "send: not connected");
}
