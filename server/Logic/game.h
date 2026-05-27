#ifndef GAME_H
#define GAME_H

#include <string>
#include <unordered_map>

#include "../../common/position.h"

#include "map.h"
#include "player.h"

// Orquestador del juego. Es el único componente con la "verdad" sobre qué
// jugadores existen y dónde están parados. El Map es solo datos estáticos
// y los Player son entidades dueñas de su propio estado — el Game las junta.
class Game {
private:
    Map& map;
    std::unordered_map<int, Player> players;

    // Busca una posición libre para spawnear un jugador nuevo.
    // Intenta primero el centro del mapa, sino cae a la primera celda
    // caminable y desocupada. Tira excepción si no encuentra ninguna.
    Position findSpawnPosition() const;

    // True si ningún jugador está parado en pos.
    bool isPositionFree(Position pos) const;

    bool processMovement(int playerId, const std::string& direction);

public:
    explicit Game(Map& map);

    bool processCommand(int playerId, const std::string& command);
};

#endif
