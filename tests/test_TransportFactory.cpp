#include "TransportFactory.hpp"
#include "ConfigTypes.hpp"
#include "ITransport.hpp"
#include "TcpTransport.hpp"
#include "UdpTransport.hpp"

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <memory>
#include <stdexcept>
#include <string>

using namespace Catch::Matchers;

TEST_CASE("TransportFactory throws on empty kind", "[TransportFactory]") {
    TransportConfig cfg;
    cfg.kind = "";
    cfg.host = "127.0.0.1";
    cfg.port = 1234;

    REQUIRE_THROWS_WITH(TransportFactory::make(cfg), ContainsSubstring("empty 'kind'"));
}

TEST_CASE("TransportFactory throws on empty host", "[TransportFactory]") {
    TransportConfig cfg;
    cfg.kind = "tcp";
    cfg.host = "";
    cfg.port = 1234;

    REQUIRE_THROWS_AS(TransportFactory::make(cfg), std::runtime_error);

    REQUIRE_THROWS_WITH(TransportFactory::make(cfg), ContainsSubstring("empty host"));
}

TEST_CASE("TransportFactory creates TcpTransport", "[TransportFactory]") {
    TransportConfig cfg;
    cfg.kind = "tcp";
    cfg.host = "localhost";
    cfg.port = 9000;

    auto transport = TransportFactory::make(cfg);
    REQUIRE(transport != nullptr);
    REQUIRE(dynamic_cast<TcpTransport*>(transport.get()) != nullptr);
}

TEST_CASE("TransportFactory creates UdpTransport", "[TransportFactory]") {
    TransportConfig cfg;
    cfg.kind = "udp";
    cfg.host = "localhost";
    cfg.port = 9001;

    auto transport = TransportFactory::make(cfg);
    REQUIRE(transport != nullptr);
    REQUIRE(dynamic_cast<UdpTransport*>(transport.get()) != nullptr);
}

TEST_CASE("TransportFactory throws on unsupported kind", "[TransportFactory]") {
    TransportConfig cfg;
    cfg.kind = "bluetooth";
    cfg.host = "localhost";
    cfg.port = 1234;

    REQUIRE_THROWS_AS(TransportFactory::make(cfg), std::runtime_error);
    REQUIRE_THROWS_WITH(TransportFactory::make(cfg), ContainsSubstring("unsupported kind 'bluetooth'"));
}

TEST_CASE("TransportFactory matches kind case-insensitively", "[TransportFactory]") {
    TransportConfig cfg;
    cfg.host = "localhost";
    cfg.port = 1111;

    SECTION("Uppercase TCP") {
        cfg.kind = "TCP";
        auto transport = TransportFactory::make(cfg);
        REQUIRE(dynamic_cast<TcpTransport*>(transport.get()) != nullptr);
    }

    SECTION("Mixedcase Udp") {
        cfg.kind = "uDp";
        auto transport = TransportFactory::make(cfg);
        REQUIRE(dynamic_cast<UdpTransport*>(transport.get()) != nullptr);
    }
}
