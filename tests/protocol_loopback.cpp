#include "protocol_loopback.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

#include "../common/Communication/common_protocol.h"
#include "../common/Communication/socket.h"

// Rango de puertos.
std::atomic<uint16_t> ProtocolLoopback::nextPort{49100};

ProtocolLoopback::ProtocolLoopback() {
    // Reintentamos por si el puerto esta ocupado.
    for (int attempt = 0; attempt < 100; ++attempt) {
        uint16_t port = nextPort.fetch_add(1);
        std::string portStr = std::to_string(port);
        try {
            Socket listener(portStr.c_str());

            // accept() es bloqueante: va en otro thread mientras el principal hace el connect.
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

ProtocolLoopback::~ProtocolLoopback() {
    try {
        if (a)
            a->shutdown();
    } catch (...) {}
    try {
        if (b)
            b->shutdown();
    } catch (...) {}
}
