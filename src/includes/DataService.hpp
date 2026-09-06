#pragma once
#include <fstream>
#include <filesystem>
#include "GridService.hpp"
#include <variant>
#include "ECS.hpp"

#pragma pack(push, 1)
struct WorldFileHeader {
    uint32_t magicNumber;
    uint32_t version;

    uint32_t size;
    uint32_t height;
    uint16_t cellSize;
    uint16_t cellHeight;
    vector3 origin;

};
#pragma pack(pop)

bool SaveWorld(const std::filesystem::path& filepath, const Grid& grid, ComponentPool<CellComponent>* cellComponent) {
    if (!cellComponent) return false;

    std::ofstream saveFile(filepath, std::ios::binary);
    if (!saveFile.is_open()) return false;

    WorldFileHeader wfh;
    wfh.magicNumber = 0x4D475244;
    wfh.version = 1;
    wfh.size = grid.size;
    wfh.height = grid.height;
    wfh.cellSize = grid.cellSize;
    wfh.cellHeight = grid.cellHeight;
    wfh.origin = grid.origin;

    saveFile.write(reinterpret_cast<const char*>(&wfh), sizeof(wfh));

    uint32_t elementCount = static_cast<uint32_t>(cellComponent->dense.size()); 
    saveFile.write(reinterpret_cast<const char*>(&elementCount), sizeof(elementCount));

    for (size_t i = 0; i < elementCount; ++i) {
    Entity entity = cellComponent->denseIds[i]; 
    CellComponent& cell = cellComponent->dense[i];
    
    saveFile.write(reinterpret_cast<const char*>(&entity), sizeof(Entity));
    saveFile.write(reinterpret_cast<const char*>(&cell), sizeof(CellComponent));
    }
    return true;
}

bool LoadWorld(const std::filesystem::path& filepath, Grid& outGrid, ComponentPool<CellComponent>* outCellComponent) {
    if (!outCellComponent) return false;

    std::ifstream saveFile(filepath, std::ios::binary);
    if (!saveFile.is_open()) return false;

    WorldFileHeader wfh;
    saveFile.read(reinterpret_cast<char*>(&wfh), sizeof(wfh));

    if (wfh.magicNumber != 0x4D475244 || wfh.version != 1) {
        saveFile.close();
        return false; 
    }

    outGrid.size = wfh.size;
    outGrid.height = wfh.height;
    outGrid.cellSize = wfh.cellSize;
    outGrid.cellHeight = wfh.cellHeight;
    outGrid.origin = wfh.origin;

    uint32_t elementCount = 0;
    saveFile.read(reinterpret_cast<char*>(&elementCount), sizeof(elementCount));

    outCellComponent->clear(); 

    for (size_t i = 0; i < elementCount; ++i) {
    Entity entity;
    CellComponent cell;
    
    saveFile.read(reinterpret_cast<char*>(&entity), sizeof(Entity));
    saveFile.read(reinterpret_cast<char*>(&cell), sizeof(CellComponent));
    
    outCellComponent->addData(cell, entity); 
    }
    return true; 
}