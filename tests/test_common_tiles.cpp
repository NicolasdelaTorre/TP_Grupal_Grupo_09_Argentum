#include <string>

#include <gtest/gtest.h>

#include "../common/common_tiles.h"

TEST(CommonTilesTest, floorTilesTieneEntradas) { EXPECT_GT(floor_tiles().size(), 0u); }

TEST(CommonTilesTest, lookupPorGridValueEncuentraConocidos) {
    auto* t = floor_tile_from_grid_value(1);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(std::string(t->id), "explanada");
}

TEST(CommonTilesTest, lookupPorIdEncuentraConocidos) {
    auto* t = floor_tile_from_id("mazmorra");
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->grid_value, 13);
}

TEST(CommonTilesTest, lookupPorTextureEncuentraConocidos) {
    auto* t = floor_tile_from_texture("tiles/desierto.png");
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(std::string(t->id), "desierto");
}

TEST(CommonTilesTest, lookupsDevuelvenNullSiNoExiste) {
    EXPECT_EQ(floor_tile_from_grid_value(200), nullptr);
    EXPECT_EQ(floor_tile_from_id("inventado"), nullptr);
    EXPECT_EQ(floor_tile_from_texture("nope.png"), nullptr);
}

TEST(CommonTilesTest, lookupPorTextureVaciaEsNull) {
    EXPECT_EQ(floor_tile_from_texture(""), nullptr);
}

TEST(CommonTilesTest, exteriorTieneValorReservado) {
    auto* t = floor_tile_from_grid_value(EXTERIOR_TILE_VALUE);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(std::string(t->id), "exterior");
}

TEST(CommonTilesTest, gridValueToCharCubreDigitos) {
    EXPECT_EQ(grid_value_to_char(0), '0');
    EXPECT_EQ(grid_value_to_char(9), '9');
}

TEST(CommonTilesTest, gridValueToCharCubreLetras) {
    EXPECT_EQ(grid_value_to_char(10), 'a');
    EXPECT_EQ(grid_value_to_char(35), 'z');
}

TEST(CommonTilesTest, gridValueToCharFueraDeRangoDevuelveCero) {
    EXPECT_EQ(grid_value_to_char(36), '0');
    EXPECT_EQ(grid_value_to_char(255), '0');
}

TEST(CommonTilesTest, gridCharToValueDigitosYLetras) {
    EXPECT_EQ(grid_char_to_value('0'), 0);
    EXPECT_EQ(grid_char_to_value('9'), 9);
    EXPECT_EQ(grid_char_to_value('a'), 10);
    EXPECT_EQ(grid_char_to_value('z'), 35);
    EXPECT_EQ(grid_char_to_value('A'), 10);
    EXPECT_EQ(grid_char_to_value('Z'), 35);
}

TEST(CommonTilesTest, gridCharToValueInvalidoDevuelveCero) {
    EXPECT_EQ(grid_char_to_value(' '), 0);
    EXPECT_EQ(grid_char_to_value('!'), 0);
}

TEST(CommonTilesTest, gridRoundTrip0a35) {
    for (uint8_t v = 0; v <= 35; ++v) {
        EXPECT_EQ(grid_char_to_value(grid_value_to_char(v)), v) << "v=" << static_cast<int>(v);
    }
}
