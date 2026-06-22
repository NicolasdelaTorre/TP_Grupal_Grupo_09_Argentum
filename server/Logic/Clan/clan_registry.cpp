#include "clan_registry.h"

#include <algorithm>
#include <cstring>
#include <fstream>

static constexpr const char* CLANS_FILE = "server/Logic/clans.bin";

ClanRegistry::ClanRegistry() { load(); }

void ClanRegistry::configure(uint8_t maxMembers_, uint8_t foundLevel_) {
    maxMembers = maxMembers_;
    foundLevel = foundLevel_;
}

Clan* ClanRegistry::findClan(const std::string& name) {
    for (auto& c: clans) {
        if (c.name == name)
            return &c;
    }
    return nullptr;
}

const Clan* ClanRegistry::findClan(const std::string& name) const {
    for (const auto& c: clans) {
        if (c.name == name)
            return &c;
    }
    return nullptr;
}

Clan* ClanRegistry::findClanOfNick(const std::string& nick) {
    for (auto& c: clans) {
        if (std::find(c.members.begin(), c.members.end(), nick) != c.members.end())
            return &c;
    }
    return nullptr;
}

const Clan* ClanRegistry::findClanOfNick(const std::string& nick) const {
    for (const auto& c: clans) {
        if (std::find(c.members.begin(), c.members.end(), nick) != c.members.end())
            return &c;
    }
    return nullptr;
}

bool ClanRegistry::clearPendingForNick(const std::string& nick) {
    bool changed = false;
    for (auto& c: clans) {
        auto it = std::find(c.pendingNicks.begin(), c.pendingNicks.end(), nick);
        if (it != c.pendingNicks.end()) {
            c.pendingNicks.erase(it);
            changed = true;
        }
    }
    return changed;
}

// ── Operaciones ──────────────────────────────────────────────────────────

ClanOutcome ClanRegistry::tryFound(const std::string& clanName, const std::string& founderNick,
                                   uint8_t founderLevel) {
    if (clanName.empty())
        return {false, "El nombre del clan no puede estar vacío"};
    if (founderLevel < foundLevel)
        return {false, "Tenés que ser nivel " + std::to_string(foundLevel) + "+ para fundar un clan"};
    if (findClanOfNick(founderNick))
        return {false, "Ya pertenecés a un clan"};
    if (findClan(clanName))
        return {false, "Ya existe un clan con ese nombre"};

    clearPendingForNick(founderNick);
    Clan c;
    c.name = clanName;
    c.founderNick = founderNick;
    c.members.push_back(founderNick);
    clans.push_back(std::move(c));
    persist();
    return {true, "Fundaste el clan " + clanName};
}

ClanOutcome ClanRegistry::tryRequestJoin(const std::string& clanName,
                                        const std::string& playerNick) {
    if (findClanOfNick(playerNick))
        return {false, "Ya pertenecés a un clan, dejalo primero con /dejar-clan"};
    Clan* c = findClan(clanName);
    if (!c)
        return {false, "No existe el clan " + clanName};
    if (std::find(c->bannedNicks.begin(), c->bannedNicks.end(), playerNick) !=
        c->bannedNicks.end())
        return {false, "Estás baneado del clan " + clanName};
    if (c->members.size() >= maxMembers)
        return {false, "El clan " + clanName + " está lleno"};

    // Solo una solicitud pendiente por jugador: cancelo cualquier otra.
    clearPendingForNick(playerNick);
    if (std::find(c->pendingNicks.begin(), c->pendingNicks.end(), playerNick) ==
        c->pendingNicks.end()) {
        c->pendingNicks.push_back(playerNick);
    }
    persist();
    return {true, "Pediste unirte al clan " + clanName};
}

ClanOutcome ClanRegistry::tryAccept(const std::string& founderNick,
                                    const std::string& applicantNick) {
    Clan* c = findClanOfNick(founderNick);
    if (!c || c->founderNick != founderNick)
        return {false, "Solo el fundador puede aceptar pedidos"};
    auto it = std::find(c->pendingNicks.begin(), c->pendingNicks.end(), applicantNick);
    if (it == c->pendingNicks.end())
        return {false, "No hay pedido de " + applicantNick};
    if (c->members.size() >= maxMembers)
        return {false, "El clan está lleno"};
    c->pendingNicks.erase(it);
    c->members.push_back(applicantNick);
    persist();
    return {true, "Aceptaste a " + applicantNick + " en el clan"};
}

ClanOutcome ClanRegistry::tryReject(const std::string& founderNick,
                                    const std::string& applicantNick) {
    Clan* c = findClanOfNick(founderNick);
    if (!c || c->founderNick != founderNick)
        return {false, "Solo el fundador puede rechazar pedidos"};
    auto it = std::find(c->pendingNicks.begin(), c->pendingNicks.end(), applicantNick);
    if (it == c->pendingNicks.end())
        return {false, "No hay pedido de " + applicantNick};
    c->pendingNicks.erase(it);
    persist();
    return {true, "Rechazaste el pedido de " + applicantNick};
}

ClanOutcome ClanRegistry::tryBan(const std::string& founderNick, const std::string& targetNick) {
    Clan* c = findClanOfNick(founderNick);
    if (!c || c->founderNick != founderNick)
        return {false, "Solo el fundador puede banear"};
    if (targetNick == founderNick)
        return {false, "No te podés banear a vos mismo"};
    // Saco de pendientes si estaba.
    auto itP = std::find(c->pendingNicks.begin(), c->pendingNicks.end(), targetNick);
    if (itP != c->pendingNicks.end())
        c->pendingNicks.erase(itP);
    // Saco de miembros si estaba.
    auto itM = std::find(c->members.begin(), c->members.end(), targetNick);
    if (itM != c->members.end())
        c->members.erase(itM);
    if (std::find(c->bannedNicks.begin(), c->bannedNicks.end(), targetNick) ==
        c->bannedNicks.end())
        c->bannedNicks.push_back(targetNick);
    persist();
    return {true, "Baneaste a " + targetNick + " del clan"};
}

ClanOutcome ClanRegistry::tryKick(const std::string& founderNick, const std::string& targetNick) {
    Clan* c = findClanOfNick(founderNick);
    if (!c || c->founderNick != founderNick)
        return {false, "Solo el fundador puede echar miembros"};
    if (targetNick == founderNick)
        return {false, "No te podés echar a vos mismo"};
    auto it = std::find(c->members.begin(), c->members.end(), targetNick);
    if (it == c->members.end())
        return {false, targetNick + " no es miembro del clan"};
    c->members.erase(it);
    persist();
    return {true, "Echaste a " + targetNick + " del clan"};
}

ClanOutcome ClanRegistry::tryLeave(const std::string& playerNick) {
    Clan* c = findClanOfNick(playerNick);
    if (!c)
        return {false, "No pertenecés a ningún clan"};
    if (c->founderNick == playerNick)
        return {false, "El fundador no puede dejar el clan"};
    auto it = std::find(c->members.begin(), c->members.end(), playerNick);
    if (it != c->members.end())
        c->members.erase(it);
    persist();
    return {true, "Dejaste el clan " + c->name};
}

// ── Consultas ────────────────────────────────────────────────────────────

std::optional<std::string> ClanRegistry::getClanOf(const std::string& nick) const {
    const Clan* c = findClanOfNick(nick);
    if (!c)
        return std::nullopt;
    return c->name;
}

std::vector<std::string> ClanRegistry::getMembersOf(const std::string& clanName) const {
    const Clan* c = findClan(clanName);
    if (!c)
        return {};
    return c->members;
}

std::vector<std::string> ClanRegistry::getPendingOf(const std::string& clanName) const {
    const Clan* c = findClan(clanName);
    if (!c)
        return {};
    return c->pendingNicks;
}

bool ClanRegistry::isFounder(const std::string& nick) const {
    const Clan* c = findClanOfNick(nick);
    return c && c->founderNick == nick;
}

bool ClanRegistry::areInSameClan(const std::string& nickA, const std::string& nickB) const {
    const Clan* a = findClanOfNick(nickA);
    if (!a)
        return false;
    return std::find(a->members.begin(), a->members.end(), nickB) != a->members.end();
}

// ── Persistencia ─────────────────────────────────────────────────────────

static void writeString(std::ofstream& f, const std::string& s) {
    uint16_t len = static_cast<uint16_t>(s.size());
    f.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len)
        f.write(s.data(), len);
}

static bool readString(std::ifstream& f, std::string& out) {
    uint16_t len = 0;
    if (!f.read(reinterpret_cast<char*>(&len), sizeof(len)))
        return false;
    out.assign(len, '\0');
    if (len && !f.read(&out[0], len))
        return false;
    return true;
}

void ClanRegistry::persist() const {
    std::ofstream f(CLANS_FILE, std::ios::binary | std::ios::trunc);
    if (!f.is_open())
        return;
    uint32_t count = static_cast<uint32_t>(clans.size());
    f.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& c: clans) {
        writeString(f, c.name);
        writeString(f, c.founderNick);
        uint16_t mc = static_cast<uint16_t>(c.members.size());
        f.write(reinterpret_cast<const char*>(&mc), sizeof(mc));
        for (const auto& m: c.members)
            writeString(f, m);
        uint16_t pc = static_cast<uint16_t>(c.pendingNicks.size());
        f.write(reinterpret_cast<const char*>(&pc), sizeof(pc));
        for (const auto& p: c.pendingNicks)
            writeString(f, p);
        uint16_t bc = static_cast<uint16_t>(c.bannedNicks.size());
        f.write(reinterpret_cast<const char*>(&bc), sizeof(bc));
        for (const auto& b: c.bannedNicks)
            writeString(f, b);
    }
}

void ClanRegistry::load() {
    std::ifstream f(CLANS_FILE, std::ios::binary);
    if (!f.is_open())
        return;
    uint32_t count = 0;
    if (!f.read(reinterpret_cast<char*>(&count), sizeof(count)))
        return;
    clans.clear();
    clans.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        Clan c;
        if (!readString(f, c.name))
            return;
        if (!readString(f, c.founderNick))
            return;
        uint16_t mc = 0;
        if (!f.read(reinterpret_cast<char*>(&mc), sizeof(mc)))
            return;
        c.members.resize(mc);
        for (uint16_t j = 0; j < mc; ++j) {
            if (!readString(f, c.members[j]))
                return;
        }
        uint16_t pc = 0;
        if (!f.read(reinterpret_cast<char*>(&pc), sizeof(pc)))
            return;
        c.pendingNicks.resize(pc);
        for (uint16_t j = 0; j < pc; ++j) {
            if (!readString(f, c.pendingNicks[j]))
                return;
        }
        uint16_t bc = 0;
        if (!f.read(reinterpret_cast<char*>(&bc), sizeof(bc)))
            return;
        c.bannedNicks.resize(bc);
        for (uint16_t j = 0; j < bc; ++j) {
            if (!readString(f, c.bannedNicks[j]))
                return;
        }
        clans.push_back(std::move(c));
    }
}
