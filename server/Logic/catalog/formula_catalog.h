#ifndef FORMULA_CATALOG_H
#define FORMULA_CATALOG_H

#include <cstdint>

// Constantes de las formulas del enunciado.
struct FormulaConfig {
    float goldSafeBase;
    float goldSafeExp;
    float goldMaxFactor;
    float expNextBase;
    float expNextExp;
    uint8_t expKillBonusMaxPct;
    uint8_t expLevelDiffBase;
    float evadeThreshold;
    uint8_t criticalChancePct;
    uint8_t fairPlayNewbieLevel;
    uint8_t fairPlayMaxLevelDiff;
};

// Parametros de clanes.
struct ClanConfig {
    uint8_t maxMembers;
    uint8_t foundLevel;
    uint8_t bonusRadius;
    float bonusPctPerMember;
};

class FormulaCatalog {
private:
    FormulaConfig formulas;
    ClanConfig clan;

    FormulaCatalog();

public:
    static const FormulaCatalog& instance();

    const FormulaConfig& getFormulas() const { return formulas; }
    const ClanConfig& getClan() const { return clan; }
};

#endif
