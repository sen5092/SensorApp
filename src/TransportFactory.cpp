// TransportFactory.cpp
#include "TransportFactory.hpp"

#include <stdexcept>
#include <string>
#include <algorithm>
#include <cctype>

#include "TcpTransport.hpp"
#include "UdpTransport.hpp"   // comment this include if you haven't added Udp yet

static inline bool valid_port(int32_t p) { return p > 0 && p <= 65535; }

std::unique_ptr<ITransport> TransportFactory::make(const TransportConfig& cfg) {
    if (cfg.kind.empty()) {
        throw std::runtime_error("TransportFactory: empty 'kind'");
    }
    if (cfg.host.empty()) {
        throw std::runtime_error("TransportFactory: empty host");
    }
    if (!valid_port(cfg.port)) {
        throw std::runtime_error("TransportFactory: invalid port (1..65535)");
    }

    std::string kind = cfg.kind;

    if (strcasecmp(cfg.kind.c_str(), "tcp") == 0) {
        return std::make_unique<TcpTransport>(cfg.host, cfg.port);
    }
    if (strcasecmp(cfg.kind.c_str(), "udp") == 0) {
        return std::make_unique<UdpTransport>(cfg.host, cfg.port);
    }

    throw std::runtime_error("TransportFactory: unsupported kind '" + cfg.kind + "'");
}
