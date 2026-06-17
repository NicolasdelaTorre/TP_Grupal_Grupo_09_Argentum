#ifndef CLAN_H
#define CLAN_H

#include <string>
#include <vector>

struct Clan {
    std::string name;
    std::string founderNick;
    std::vector<std::string> members;       // incluye al fundador
    std::vector<std::string> pendingNicks;  // pedidos pendientes para este clan
    std::vector<std::string> bannedNicks;
};

// Resultado de cualquier accion sobre clanes (fundar, unirse, aceptar, etc).
// El mensaje queda listo para mandar al chat del jugador.
struct ClanOutcome {
    bool ok = false;
    std::string message;
};

#endif
