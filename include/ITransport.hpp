// ITransport.hpp
#pragma once
#include <string>
#include <cstddef>

class ITransport {
public:
    ITransport() = default;
    virtual ~ITransport() = default;

    ITransport(const ITransport&) = delete;
    ITransport& operator=(const ITransport&) = delete;
    ITransport(ITransport&&) = delete;
    ITransport& operator=(ITransport&&) = delete;

    // establish the link to the collector (whatever “link” means: TCP, TLS, cellular…)
    virtual void connect() = 0;

    // blocking, send-all semantics; throws on failure
    virtual std::size_t sendString(const std::string& payload) = 0;

    // tear down the link; safe to call multiple times
    virtual void close() = 0;

    // optional helper for status
    [[nodiscard]] virtual bool isConnected() const = 0;
};
