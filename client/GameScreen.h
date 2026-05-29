#pragma once

#include <string>
#include <unordered_map>

#include <SDL2pp/SDL2pp.hh>

#include "../common/position.h"
#include "../common/queue.h"

#include "client_protocol.h"  // ReceivedMap
#include "map_renderer.h"

static constexpr float FEET_OFFSET = 0.5f;
static constexpr float HEAD_OFFSET = 0.5f;

// Jugador remoto del que recibimos eventos por broadcast del servidor.
struct OtherPlayer {
    Player visual;
    std::string name;
};

class GameScreen {
public:
    GameScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath,
               Queue<std::string>& events_queue, Queue<std::string>& server_queue,
               const ReceivedMap& mapData, Position spawn);

    // Retorna false cuando el jugador quiere salir
    bool run();

private:
    SDL2pp::Renderer& renderer;
    TextureCache cache;
    MapRenderer mapRenderer;
    GameMap map;
    Player player;

    // Queue al sender: pusheamos "TOP"/"BOTTOM"/"LEFT"/"RIGHT" cuando el jugador cruza un tile.
    Queue<std::string>& events_queue;
    int lastTileX;
    int lastTileY;
    Direction lastSentDir;  // última dirección que mandamos al server (para detectar giros)

    // Eventos del servidor (NEW_PLAYER / PLAYER_MOVED / PLAYER_DISCONNECTED) que el receiver
    // pushea.
    Queue<std::string>& server_queue;
    std::unordered_map<int, OtherPlayer> otherPlayers;

    // ── Input ─────────────────────────────────────────────────
    bool handleEvents(float dt);

    // ── Update ────────────────────────────────────────────────
    void update(float dt);

    // ── Render ────────────────────────────────────────────────
    void render();

    // Detecta cuando el jugador cruza a un tile distinto y notifica al server
    void notifyTileChange();

    // Si la dirección local cambió respecto a lo último que mandamos, manda TURN_*.
    void notifyDirectionChange();

    // Drena los eventos pendientes del servidor y actualiza otherPlayers.
    void consumeServerEvents();

    // True si algún otherPlayer está en (tileX, tileY). Para que la predicción local no choque.
    bool isOccupiedByOther(int tileX, int tileY) const;
};
