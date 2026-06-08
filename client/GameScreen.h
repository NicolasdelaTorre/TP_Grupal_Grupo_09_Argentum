#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2pp/SDL2pp.hh>
#include <SDL2pp/SDLTTF.hh>

#include "../common/position.h"
#include "../common/queue.h"

#include "client_protocol.h"  // ReceivedMap
#include "map_renderer.h"

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
    SDL2pp::SDLTTF chatTtf;
    SDL2pp::Font chatFont;
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
    // Items del inventario del jugador local (solo slots ocupadas, en orden).
    // Se actualiza al recibir INVENTORY. Se dibujan sobre el grid del HUD.
    std::vector<uint8_t> inventoryItems;
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

    // ── Chat ──────────────────────────────────────────────────
    // Barra de comandos arriba de la pantalla. Se abre con Enter; mientras está
    // abierta el movimiento queda congelado y las teclas escriben en chatInput.
    // Al presionar Enter de nuevo se parsea chatInput a un evento de la queue.
    bool chatActive = false;
    std::string chatInput;
    // Historial de líneas mostradas en la caja de chat (comandos tipeados + feedback
    // local). Se conserva solo lo último; ver MAX_CHAT_LINES.
    std::vector<std::string> chatHistory;
    static constexpr size_t MAX_CHAT_LINES = 5;
    // Geometría de la caja de chat. CHAT_BOX_H también lo usa la cámara para
    // centrar al jugador en el área que queda por debajo del chat.
    static constexpr int CHAT_LINE_H = 18;
    static constexpr int CHAT_PAD = 4;
    static constexpr int CHAT_BOX_H = CHAT_PAD * 2 + CHAT_LINE_H * (int)(MAX_CHAT_LINES + 1);

    // Agrega una línea al historial, recortando las más viejas si hace falta.
    void addChatLine(const std::string& line);

    // Convierte el texto tipeado (ej "/oro", "/tirar 2") al evento que entiende
    // el client_sender y lo pushea a events_queue. Comandos desconocidos se ignoran.
    void submitChat();

    // Dibuja la caja de chat (siempre visible) arriba de todo: historial + línea
    // de input. Fondo negro semi-transparente.
    void renderChat();

    // ── Input ─────────────────────────────────────────────────
    bool handleEvents(float dt);

    // ── Update ────────────────────────────────────────────────
    void update(float dt);

    // ── Render ────────────────────────────────────────────────
    void render();

    // Resolución de diseño base (ventana 900x600, ver client.cpp). La UI fija
    // en píxeles (HUD, inventario, chat) se escala por screenH/BASE_SCREEN_H para
    // que mantenga su proporción tanto en ventana fija como en fullscreen.
    static constexpr int BASE_SCREEN_H = 600;
    float uiScale() const;       // factor de escala de la UI según el alto actual
    int hudPanelW() const;       // ancho del panel del HUD escalado
    int chatBoxH() const;        // alto de la caja de chat escalado

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
