#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_TESTS_PROTOCOL_LOOPBACK_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_TESTS_PROTOCOL_LOOPBACK_H

#include <atomic>
#include <cstdint>
#include <memory>

class CommonProtocol;

// Dos CommonProtocols conectados por loopback TCP. Lo que escribe `a` lo lee `b`.
class ProtocolLoopback {
public:
    std::unique_ptr<CommonProtocol> a;
    std::unique_ptr<CommonProtocol> b;

    ProtocolLoopback();
    ~ProtocolLoopback();

    ProtocolLoopback(const ProtocolLoopback&) = delete;
    ProtocolLoopback& operator=(const ProtocolLoopback&) = delete;

private:
    // Atomic porque gtest puede correr tests en paralelo.
    static std::atomic<uint16_t> nextPort;
};

#endif
