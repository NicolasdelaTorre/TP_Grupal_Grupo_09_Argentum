#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2pp/SDL2pp.hh>
#include <SDL2pp/SDLTTF.hh>

#include "../common/Communication/events/server_events.h"
#include "../common/position.h"
#include "../common/queue.h"

#include "Communication/client_receiver.h"  // IncomingQueue alias
#include "Communication/client_sender.h"    // OutgoingQueue alias
#include "map_renderer.h"
#include "render_constants.h"
#include "visual_types.h"

static constexpr float FEET_OFFSET = 0.8f;
static constexpr float HEAD_OFFSET = 0.5f;

static constexpr float BLOOD_DURATION = 0.5f;  // seconds a blood splatter stays visible
static constexpr float ARROW_SPEED = 10.0f;    // tiles/sec

struct BloodEffect {
    float x, y;   // world tile position where the hit occurred
    float timer;  // remaining display time in seconds
};

// Jugador remoto del que recibimos eventos por broadcast del servidor.
// target* es el tile destino que mandó el server; visual.x/y avanzan hacia ahí
// a PLAYER_MOVE_SPEED para que el movimiento se vea fluido en vez de teletransporte.
struct OtherPlayer {
    Player_ visual;
    float targetX = 0.0f;
    float targetY = 0.0f;
    std::string name;
    bool ghost = false;  // PlayerDiedEvent/PlayerRevivedEvent alternan este flag
};

// NPC remoto. Mismo patrón que OtherPlayer: visual es el sprite, target* el tile
// destino al que está caminando, alive controla si se renderiza o no.
struct RemoteNpc {
    NpcEntity visual;
    float targetX = 0.0f;
    float targetY = 0.0f;
    bool alive = true;
};

class GameScreen {
public:
    GameScreen(SDL2pp::Renderer& renderer, const std::string& assetsPath,
               OutgoingQueue& clientEvents, IncomingQueue& serverEvents, const MapEvent& mapData,
               Position spawn, Player_ player);

    // Retorna false cuando el jugador quiere salir
    bool run();


private:
    SDL2pp::Renderer& renderer;
    SDL2pp::SDLTTF chatTtf;
    SDL2pp::Font chatFont;
    TextureCache cache;
    MapRenderer mapRenderer;
    GameMap map;

    uint16_t a = 0;

    // Queue al sender: empujamos ClientEvents ya construidos (MovementEvent, etc.).
    OutgoingQueue& clientEvents;
    int lastTileX;
    int lastTileY;
    SpriteRow lastSentDir;  // última dirección que mandamos al server (para detectar giros)

    // Eventos del servidor (NEW_PLAYER / PLAYER_MOVED / PLAYER_DISCONNECTED) que el receiver
    // pushea tipados.
    IncomingQueue& serverEvents;
    Player_ player;
    std::unordered_map<int, OtherPlayer> otherPlayers;
    std::unordered_map<int, RemoteNpc> npcs;
    std::vector<DroppedItem> droppedItems;
    // Inventario del jugador local: itemIds en orden, solo slots ocupadas.
    // Se actualiza con InventoryUpdateEvent.
    std::vector<uint8_t> inventoryItems;
    std::vector<BloodEffect> bloodEffects;
    std::vector<ArrowProjectile> arrows;
    bool chatActive = false;
    std::string chatBuffer;
    // Historial visible en la caja de chat: comandos tipeados + broadcasts.
    std::vector<std::string> chatHistory;

    // True si el jugador local está muerto (fantasma). Lo activa
    // PlayerDiedEvent dirigido a nuestro id (no está en otherPlayers).
    bool localGhost = false;

    // Stats del jugador local (vienen por STATS_JUGADOR).
    uint16_t health = 0;
    uint16_t maxHealth = 0;
    uint16_t mana = 0;
    uint16_t maxMana = 0;
    uint32_t gold = 0;
    uint32_t experience = 0;
    uint32_t nextLevelExp = 0;
    uint8_t level = 1;

    // ── UI / chat ─────────────────────────────────────────────
    // Constantes de la caja de chat y del panel derecho del HUD. La geometria
    // se escala dinamicamente con uiScale() (ver implementacion).
    static constexpr size_t MAX_CHAT_LINES = 5;
    static constexpr int CHAT_LINE_H = 18;
    static constexpr int CHAT_PAD = 4;
    static constexpr int CHAT_BOX_H = CHAT_PAD * 2 + CHAT_LINE_H * (int)(MAX_CHAT_LINES + 1);
    static constexpr int BASE_SCREEN_H = 600;
    static constexpr int HUD_PANEL_W = 230;  // ancho base del panel derecho

    float uiScale() const;       // factor de escala segun el alto actual
    int hudPanelW() const;       // ancho del panel del HUD escalado
    int chatBoxH() const;        // alto de la caja de chat escalado

    // Agrega una linea al historial del chat, recortando las viejas si pasa el limite.
    void addChatLine(const std::string& line);

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

    // Dibuja la barra de vida/mana/exp en la esquina superior izquierda.
    void renderStatsBar();

    // Dibuja el panel derecho del HUD (fondo + inventario).
    void renderInventoryPanel();

    // Dibuja la caja de chat arriba con historial e input actual.
    void renderChat();

    // Dibuja todos los efectos de sangre activos.
    void renderBloodEffects(float camX, float camY);
};
