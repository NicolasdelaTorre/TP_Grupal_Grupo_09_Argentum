#include "common_biome.h"

#include <array>

namespace {

struct BiomeName {
    BiomeType type;
    const char* template_id;
};

constexpr std::array<BiomeName, 8> BIOME_NAMES = {{
        {BiomeType::EXPLANADA, "explanada"},
        {BiomeType::BOSQUE_OSCURO, "bosque_oscuro"},
        {BiomeType::BOSQUE_ARANAS, "bosque_aranas"},
        {BiomeType::DESIERTO, "desierto"},
        {BiomeType::CEMENTERIO, "cementerio"},
        {BiomeType::NIEVE, "nieve"},
        {BiomeType::ROCOSAS, "rocosas"},
        {BiomeType::PANTANO_EMBRUJADO, "pantano_embrujado"},
}};

}  // namespace

BiomeType biome_from_template_id(const std::string& template_id) {
    for (const auto& entry: BIOME_NAMES) {
        if (template_id == entry.template_id) {
            return entry.type;
        }
    }
    return BiomeType::NONE;
}
