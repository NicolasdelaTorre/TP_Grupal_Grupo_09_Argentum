#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_TESTS_PROTOCOL_LOOPBACK_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_TESTS_PROTOCOL_LOOPBACK_H

#include <atomic>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

#include "../common/Communication/common_protocol.h"
#include "../common/Communication/socket.h"

// Conexion TCP loopback con dos CommonProtocols, para probar el wire de verdad sin mockear el socket.
class ProtocolLoopback {
public:
    std::unique_ptr<CommonProtocol> a;
    std::unique_ptr<CommonProtocol> b;

    ProtocolLoopback() {
        for (int attempt = 0; attempt < 100; ++attempt) {
            uint16_t port = nextPort.fetch_add(1);
            std::string portStr = std::to_string(port);
            try {
                Socket listener(portStr.c_str());
                std::optional<Socket> accepted;
                std::thread acceptor([&] { accepted.emplace(listener.accept()); });
                Socket connecting("127.0.0.1", portStr.c_str());
                acceptor.join();
                a = std::make_unique<CommonProtocol>(std::move(connecting));
                b = std::make_unique<CommonProtocol>(std::move(*accepted));
                return;
            } catch (...) {
                continue;
            }
        }
        throw std::runtime_error("ProtocolLoopback: no pude conseguir un puerto libre");
    }

    ~ProtocolLoopback() {
        try {
            if (a)
                a->shutdown();
        } catch (...) {}
        try {
            if (b)
                b->shutdown();
        } catch (...) {}
    }

    ProtocolLoopback(const ProtocolLoopback&) = delete;
    ProtocolLoopback& operator=(const ProtocolLoopback&) = delete;

private:
    static std::atomic<uint16_t> nextPort;
};

inline std::atomic<uint16_t> ProtocolLoopback::nextPort{49100};

#endif
