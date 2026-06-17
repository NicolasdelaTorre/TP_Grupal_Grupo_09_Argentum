#ifndef CLAN_REGISTRY_H
#define CLAN_REGISTRY_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "clan.h"

// Registro central de clanes
class ClanRegistry {
private:
    std::vector<Clan> clans;
    uint8_t maxMembers = 16;
    uint8_t foundLevel = 6;

    Clan* findClan(const std::string& name);
    const Clan* findClan(const std::string& name) const;
    Clan* findClanOfNick(const std::string& nick);
    const Clan* findClanOfNick(const std::string& nick) const;
    // Saca al nick de pendientes en cualquier clan. Devuelve true si limpio algo.
    bool clearPendingForNick(const std::string& nick);
    void persist() const;
    void load();

public:
    ClanRegistry();

    void configure(uint8_t maxMembers, uint8_t foundLevel);

    // Operaciones (todas devuelven ClanOutcome con mensaje listo para chat).
    ClanOutcome tryFound(const std::string& clanName, const std::string& founderNick,
                        uint8_t founderLevel);
    ClanOutcome tryRequestJoin(const std::string& clanName, const std::string& playerNick);
    ClanOutcome tryAccept(const std::string& founderNick, const std::string& applicantNick);
    ClanOutcome tryReject(const std::string& founderNick, const std::string& applicantNick);
    ClanOutcome tryBan(const std::string& founderNick, const std::string& targetNick);
    ClanOutcome tryKick(const std::string& founderNick, const std::string& targetNick);
    ClanOutcome tryLeave(const std::string& playerNick);

    // Consultas.
    std::optional<std::string> getClanOf(const std::string& nick) const;
    std::vector<std::string> getMembersOf(const std::string& clanName) const;
    std::vector<std::string> getPendingOf(const std::string& clanName) const;
    bool isFounder(const std::string& nick) const;
    bool areInSameClan(const std::string& nickA, const std::string& nickB) const;
};

#endif
