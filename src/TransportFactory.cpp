// TransportFactory.cpp
#include "TransportFactory.hpp"
#include "TcpTransport.hpp"
#include "UdpTransport.hpp"
#include "ITransport.hpp"
#include "ConfigTypes.hpp"
#include "StringUtils.hpp"



#include <stdexcept>
#include <string>
#include <cctype>
#include <memory> // for std::make_unique


std::unique_ptr<ITransport> TransportFactory::make(const TransportConfig& cfg) {

    if (cfg.kind.empty()) {
        throw std::runtime_error("TransportFactory: empty 'kind'");
    }
    if (cfg.host.empty()) {
        throw std::runtime_error("TransportFactory: empty host");
    }
    if (StringUtils::iequals(cfg.kind, "tcp")) {
        return std::make_unique<TcpTransport>(cfg.host, cfg.port);
    }
    if (StringUtils::iequals(cfg.kind, "udp")) {
        return std::make_unique<UdpTransport>(cfg.host, cfg.port);
    }

    throw std::runtime_error("TransportFactory: unsupported kind '" + cfg.kind + "'");
}
