#pragma once
#include <iostream>
#include <cmath>
#include <vector>
#include <functional>
#include "Math.hpp"
#include "Physics.hpp"
#include "ECS.hpp"

struct Grid {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t cellSize;
    uint16_t cellHeight;
    uint32_t heightInCells;
    uint32_t maxIndex;
    vector3 origin;

    Grid(uint32_t size, uint32_t height, uint16_t cellSize, uint16_t cellHeight, const vector3& origin) 
        : size(size), height(height), cellSize(cellSize), cellHeight(cellHeight), origin(origin) {
        
        width = size * 2 + 1;
        heightInCells = (cellHeight > 0) ? (height / cellHeight) : 1;
        maxIndex = width * heightInCells * width;
    }

    uint32_t GetCellIndex(const vector3& cellPos) const {
        uint32_t x = static_cast<uint32_t>(cellPos.x + size);
        uint32_t y = static_cast<uint32_t>(cellPos.y);
        uint32_t z = static_cast<uint32_t>(cellPos.z + size);

        return (x * heightInCells * width) + (y * width) + z;
    }

    vector3 GetCellPos(uint32_t idx) const {
        uint32_t z = idx % width;
        uint32_t y = (idx / width) % heightInCells;
        uint32_t x = idx / (width * heightInCells);

        return vector3(
            static_cast<float>(x) - static_cast<float>(size),
            static_cast<float>(y),
            static_cast<float>(z) - static_cast<float>(size)
        );
    }

    void ForNeighbors(uint32_t idx, const std::vector<int>& offsets, const std::function<void(uint32_t, size_t)>& func) const {
        for (size_t i = 0; i < offsets.size(); ++i) {
            func(static_cast<uint32_t>(static_cast<int>(idx) + offsets[i]), i);
        }
    }

    vector3 WorldToCellScale(const vector3& pos) const {
        vector3 localPos = pos - origin;
        return vector3(
            std::round(localPos.x / cellSize),
            std::floor(localPos.y / cellHeight),
            std::round(localPos.z / cellSize)
        );
    } 

    vector3 CellToWorldCenter(const vector3& cellPos) const {
        return origin + vector3(
            cellPos.x * cellSize,
            cellPos.y * cellHeight,
            cellPos.z * cellSize
        );
    }

    bool IsCellInBounds(const vector3& cellPos) const {
        return std::abs(cellPos.x) <= size && 
               std::abs(cellPos.z) <= size && 
               cellPos.y >= 0 && 
               cellPos.y < heightInCells;
    }

    bool IsWorldPosInBounds(const vector3& worldPos) const {
        return IsCellInBounds(WorldToCellScale(worldPos));
    }
};

struct CellComponent {
    int32_t cellX = 0;
    int32_t cellY = 0;
    int32_t cellZ = 0;
};

void updateSpatialIndex(const Grid& grid, Registry& registry) {
    registry.view<CellComponent, TransformComponent>([&grid](Entity id, CellComponent& cellComponent, const TransformComponent& tfc) {
        vector3 currentCellPos = grid.WorldToCellScale(tfc.position);
        
        int32_t newX = static_cast<int32_t>(currentCellPos.x);
        int32_t newY = static_cast<int32_t>(currentCellPos.y);
        int32_t newZ = static_cast<int32_t>(currentCellPos.z);

        if (newX != cellComponent.cellX || newY != cellComponent.cellY || newZ != cellComponent.cellZ) {
            cellComponent.cellX = newX;
            cellComponent.cellY = newY;
            cellComponent.cellZ = newZ;
        }
    });
}