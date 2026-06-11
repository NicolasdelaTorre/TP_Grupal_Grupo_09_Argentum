#include "biome_grid.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <queue>
#include <utility>

std::vector<int> computeBiomeOwners(int width, int height,
                                    const std::vector<BiomeSource>& sources) {
    const int W = width;
    const int H = height;
    std::vector<int> owner(static_cast<size_t>(W) * H, -1);
    if (W <= 0 || H <= 0 || sources.empty()) {
        return owner;
    }

    std::vector<double> cost(static_cast<size_t>(W) * H, std::numeric_limits<double>::infinity());

    // los más grandes van mas rapido
    std::vector<double> speed(sources.size(), 1.0);
    for (size_t i = 0; i < sources.size(); ++i) {
        speed[i] =
                std::max(1.0, std::sqrt(static_cast<double>(sources[i].width * sources[i].height)));
    }

    struct FrontierNode {
        double cost = 0.0;
        int x = 0;
        int y = 0;
        int owner = -1;

        bool operator<(const FrontierNode& other) const { return cost > other.cost; }
    };
    std::priority_queue<FrontierNode> expansion_frontier;

    for (size_t biome_idx = 0; biome_idx < sources.size(); ++biome_idx) {
        const auto& biome_zone = sources[biome_idx];
        for (int rel_y = 0; rel_y < biome_zone.height; ++rel_y) {
            for (int rel_x = 0; rel_x < biome_zone.width; ++rel_x) {
                const int absolute_x = biome_zone.x + rel_x;
                const int absolute_y = biome_zone.y + rel_y;
                if (absolute_x < 0 || absolute_y < 0 || absolute_x >= W || absolute_y >= H) {
                    continue;
                }
                const size_t flat_index = static_cast<size_t>(absolute_y) * W + absolute_x;
                if (cost[flat_index] <= 0.0) {
                    continue;
                }
                owner[flat_index] = static_cast<int>(biome_idx);
                cost[flat_index] = 0.0;
                expansion_frontier.push({0.0, absolute_x, absolute_y, static_cast<int>(biome_idx)});
            }
        }
    }

    const int delta_x4[] = {1, -1, 0, 0};
    const int delta_y4[] = {0, 0, 1, -1};
    while (!expansion_frontier.empty()) {
        const FrontierNode frontier_node = expansion_frontier.top();
        expansion_frontier.pop();
        const size_t frontier_flat_idx = static_cast<size_t>(frontier_node.y) * W + frontier_node.x;
        if (frontier_node.cost != cost[frontier_flat_idx] ||
            frontier_node.owner != owner[frontier_flat_idx]) {
            continue;
        }

        const double biome_step_cost = 1.0 / speed[frontier_node.owner];
        for (int dir = 0; dir < 4; ++dir) {
            const int neighbor_x = frontier_node.x + delta_x4[dir];
            const int neighbor_y = frontier_node.y + delta_y4[dir];
            if (neighbor_x < 0 || neighbor_y < 0 || neighbor_x >= W || neighbor_y >= H) {
                continue;
            }
            const size_t neighbor_flat_idx = static_cast<size_t>(neighbor_y) * W + neighbor_x;
            const double propagated_cost = frontier_node.cost + biome_step_cost;
            if (propagated_cost >= cost[neighbor_flat_idx]) {
                continue;
            }
            owner[neighbor_flat_idx] = frontier_node.owner;
            cost[neighbor_flat_idx] = propagated_cost;
            expansion_frontier.push({propagated_cost, neighbor_x, neighbor_y, frontier_node.owner});
        }
    }

    return owner;
}

// Calcula las celdas exteriores del mapa, para los environments.
std::vector<bool> computeExteriorCells(int width, int height,
                                       const std::vector<bool>& is_wall) {
    std::vector<bool> is_exterior(static_cast<size_t>(width) * height, false);
    if (width <= 0 || height <= 0) {
        return is_exterior;
    }

    std::queue<std::pair<int, int>> frontier;
    auto enqueue_if_open = [&](int x, int y) {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            return;
        }
        const size_t idx = static_cast<size_t>(y) * width + x;
        if (is_wall[idx] || is_exterior[idx]) {
            return;
        }
        is_exterior[idx] = true;
        frontier.emplace(x, y);
    };

    for (int x = 0; x < width; ++x) {
        enqueue_if_open(x, 0);
        enqueue_if_open(x, height - 1);
    }
    for (int y = 0; y < height; ++y) {
        enqueue_if_open(0, y);
        enqueue_if_open(width - 1, y);
    }

    const int dx4[] = {1, -1, 0, 0};
    const int dy4[] = {0, 0, 1, -1};
    while (!frontier.empty()) {
        const auto [cx, cy] = frontier.front();
        frontier.pop();
        for (int k = 0; k < 4; ++k) {
            enqueue_if_open(cx + dx4[k], cy + dy4[k]);
        }
    }

    return is_exterior;
}
