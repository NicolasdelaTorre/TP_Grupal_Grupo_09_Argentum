#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "../common/position.h"
#include "../common/queue.h"

#include "client_protocol.h"  // ReceivedMap
#include "map_renderer.h"

static constexpr float FEET_OFFSET = 0.8f;
static constexpr float HEAD_OFFSET = 0.5f;

static constexpr float BLOOD_DURATION = 0.5f;   // seconds a blood splatter stays visible
static constexpr float ARROW_SPEED    = 10.0f;  // tiles/sec

struct BloodEffect {
    float x, y;   // world tile position where the hit occurred
    float timer;  // remaining display time in seconds
};

// Jugador remoto del que recibimos eventos por broadcast del servidor.
// target* es el tile destino que mandó el server; visual.x/y avanzan hacia ahí
// a PLAYER_MOVE_SPEED para que el movimiento se vea fluido en vez de teletransporte.
struct OtherPlayer {
    Player visual;
    float targetX = 0.0f;
    float targetY = 0.0f;
    std::string name;
};

class GameScreen {
public:
    GameScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath,
               Queue<std::string>& events_queue, Queue<std::string>& server_queue,
               const ReceivedMap& mapData, Position spawn, Player player);

    // Retorna false cuando el jugador quiere salir
    bool run();
    

private:
    SDL2pp::Renderer& renderer;
    TextureCache cache;
    MapRenderer mapRenderer;
    GameMap map;

    uint16_t a = 0;

    // Queue al sender: pusheamos "TOP"/"BOTTOM"/"LEFT"/"RIGHT" cuando el jugador cruza un tile.
    Queue<std::string>& events_queue;
    int lastTileX;
    int lastTileY;
    Direction lastSentDir;  // última dirección que mandamos al server (para detectar giros)

    // Eventos del servidor (NEW_PLAYER / PLAYER_MOVED / PLAYER_DISCONNECTED) que el receiver
    // pushea.
    Queue<std::string>& server_queue;
    Player player;
    std::unordered_map<int, OtherPlayer> otherPlayers;
    std::vector<DroppedItem> droppedItems;
    std::vector<BloodEffect> bloodEffects;
    std::vector<ArrowProjectile> arrows;

    // Stats del jugador local (vienen por STATS_JUGADOR).
    uint16_t health = 0;
    uint16_t maxHealth = 0;
    uint16_t mana = 0;
    uint16_t maxMana = 0;
    uint32_t gold = 0;
    uint32_t experience = 0;
    uint32_t nextLevelExp = 0;
    uint8_t level = 1;

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

    // Dibuja la barra de vida en la esquina superior izquierda.
    void renderHUD();

    // Dibuja todos los efectos de sangre activos.
    void renderBloodEffects(float camX, float camY);
};
