#include "formula_catalog.h"

#include "../toml.hpp"

FormulaCatalog::FormulaCatalog() {
    const toml::value cfg = toml::parse("server/Logic/Stats/config.toml");

    formulas.goldSafeBase = toml::find<float>(cfg, "formula", "gold", "safeBase");
    formulas.goldSafeExp = toml::find<float>(cfg, "formula", "gold", "safeExp");
    formulas.goldMaxFactor = toml::find<float>(cfg, "formula", "gold", "maxFactor");
    formulas.expNextBase = toml::find<float>(cfg, "formula", "experience", "nextBase");
    formulas.expNextExp = toml::find<float>(cfg, "formula", "experience", "nextExp");
    formulas.expKillBonusMaxPct =
            toml::find<uint8_t>(cfg, "formula", "experience", "killBonusMaxPct");
    formulas.expLevelDiffBase =
            toml::find<uint8_t>(cfg, "formula", "experience", "levelDiffBase");
    formulas.evadeThreshold = toml::find<float>(cfg, "formula", "combat", "evadeThreshold");
    formulas.criticalChancePct =
            toml::find<uint8_t>(cfg, "formula", "combat", "criticalChancePct");
    formulas.fairPlayNewbieLevel =
            toml::find<uint8_t>(cfg, "formula", "fairplay", "newbieLevel");
    formulas.fairPlayMaxLevelDiff =
            toml::find<uint8_t>(cfg, "formula", "fairplay", "maxLevelDiff");

    clan.maxMembers = toml::find<uint8_t>(cfg, "clan", "maxMembers");
    clan.foundLevel = toml::find<uint8_t>(cfg, "clan", "foundLevel");
    clan.bonusRadius = toml::find<uint8_t>(cfg, "clan", "bonusRadius");
    clan.bonusPctPerMember = toml::find<float>(cfg, "clan", "bonusPctPerMember");
}

const FormulaCatalog& FormulaCatalog::instance() {
    static const FormulaCatalog catalog;
    return catalog;
}
