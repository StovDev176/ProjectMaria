#pragma once
#include <iostream>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <cstdint>
#include <vector>
#include <utility>
#include <functional>
#include "Logger.hpp"

using Entity = uint32_t;

struct VisibleComponent {};

class IComponentPool {
public:
    virtual ~IComponentPool() = default;
    virtual void removeData(Entity entityId) = 0;
    virtual size_t size() = 0;
    virtual const std::vector<uint32_t>& getDenseIds() = 0;
};

template<typename T>
class ComponentPool : public IComponentPool {
public:
    std::vector<T> dense;
    std::vector<uint32_t> denseIds;
    std::vector<uint32_t> sparseSet;

    const std::vector<uint32_t>& getDenseIds() override {
        return denseIds;
    }

    void addData(T data, uint32_t entityId) {
        if (entityId >= sparseSet.size()) {
            sparseSet.resize(entityId+1, UINT32_MAX);
        }
        dense.push_back(data);
        denseIds.push_back(entityId);
        sparseSet[entityId] = dense.size() - 1;

    }

    void addData(uint32_t entityId) {
        if (entityId >= sparseSet.size()) {
            sparseSet.resize(entityId + 1, UINT32_MAX);
        }
        dense.push_back(T{}); 
        denseIds.push_back(entityId);
        sparseSet[entityId] = static_cast<uint32_t>(denseIds.size() - 1);
    }
    size_t size() override {
        return dense.size();
    }

    void removeData(uint32_t entityId) override {
        if (entityId >= sparseSet.size() || sparseSet[entityId] == UINT32_MAX) {
            return;
        }
        uint32_t denseIdx = sparseSet[entityId]; 
        uint32_t lastEntityId = denseIds.back(); 
        
        dense[denseIdx] = dense.back(); 
        denseIds[denseIdx] = lastEntityId;
        
        sparseSet[lastEntityId] = denseIdx; 
        
        dense.pop_back();
        denseIds.pop_back();
        sparseSet[entityId] = UINT32_MAX;
    }

    T* getComponent(uint32_t entityId) {
        if (entityId >= sparseSet.size() || sparseSet[entityId] == UINT32_MAX) {
            return nullptr;
        }
        uint32_t denseIdx = sparseSet[entityId];
        return &dense[denseIdx];
    }
    bool hasEntity(Entity entity) const {
        if (entity >= sparseSet.size()) {
            return false;
        }   
        return sparseSet[entity] != UINT32_MAX; 
    }
};

class Registry {
private:
    Entity nextEntity = 0;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> pools;

public:
    Entity create() {
        return nextEntity++;
    }

    template<typename T, typename... Args>
    void addComponent(Entity entity, Args&&... args) {
        std::type_index typeId(typeid(T));
        if (pools.find(typeId) == pools.end()) {
            pools[typeId] = std::make_unique<ComponentPool<T>>();
        }
        ComponentPool<T>* pool = static_cast<ComponentPool<T>*>(pools[typeId].get());
    pool->addData(T(std::forward<Args>(args)...), entity);
    }

    template<typename T>
    ComponentPool<T>* getComponentPool() {
        std::type_index typeId(typeid(T));
        if (pools.find(typeId) == pools.end()) {
            pools[typeId] = std::make_unique<ComponentPool<T>>();
        }
        return static_cast<ComponentPool<T>*>(pools[typeId].get());
    }

    void removeEntityFromAllPools(Entity entityId) {
        for (auto& [id, pool] : pools) {
            pool->removeData(entityId);
        }
    }
    
    template<typename... Components, typename Func>

void view(Func&& func) {
    std::array<IComponentPool*, sizeof...(Components)> pools = {
        getComponentPool<Components>()...
    };

    for (auto* pool : pools) {
        if (!pool) {
            return;
        }
    }

    size_t smallestIndex = 0;
    size_t minSize = pools[0]->size();

    for (size_t i = 1; i < pools.size(); ++i) {
        if (pools[i]->size() < minSize) {
            minSize = pools[i]->size();
            smallestIndex = i;
        }
    }


    auto typedPools = std::make_tuple(getComponentPool<Components>()...);

    auto& smallestTypedPool = std::get<0>(typedPools); 

    IComponentPool* smallestPoolPtr = pools[smallestIndex];

    auto processEntities = [&](auto* targetPool) {
        for (Entity entity : targetPool->getDenseIds()) {
            bool valid = (... && std::get<ComponentPool<Components>*>(typedPools)->hasEntity(entity)); 
            if (valid) {
                func(entity, (*std::get<ComponentPool<Components>*>(typedPools)->getComponent(entity))...);
            }
        }
    };
    processEntities(smallestPoolPtr);
}
};