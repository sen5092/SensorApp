// ITransport.hpp
#pragma once
#include <string>
#include <cstddef>

class ITransport {
public:
    virtual ~ITransport() = default;

    // establish the link to the collector (whatever “link” means: TCP, TLS, cellular…)
    virtual void connect() = 0;

    // blocking, send-all semantics; throws on failure
    virtual std::size_t sendString(const std::string& payload) = 0;

    // tear down the link; safe to call multiple times
    virtual void close() = 0;

    // optional helper for status
    virtual bool isConnected() const = 0;
};
