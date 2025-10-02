// TransportFactory.hpp
#pragma once

#include <memory>
#include "ConfigTypes.hpp"   // TransportConfig
#include "ITransport.hpp"    // interface

struct TransportFactory {
    // Build a concrete transport based on cfg.kind ("tcp" | "udp").
    // NOTE: This does NOT call connect(); the caller decides when to connect.
    static std::unique_ptr<ITransport> make(const TransportConfig& cfg);
};
