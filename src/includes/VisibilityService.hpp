#pragma once

#include <iostream>
#include "CameraService.hpp"
#include "ECS.hpp"
#include "Physics.hpp"
#include "Math.hpp"
#include "raylib.h"

void evaluateVisiblity(const Camera3D& camera, const ComponentPool<TransformComponent>* tfcPool, float radius, Registry& registry) {
    if (!tfcPool) return;
    float xMin = camera.position.x - radius;
    float xMax = camera.position.x + radius;
    float zMin = camera.position.z - radius;
    float zMax = camera.position.z + radius;

    ComponentPool<VisibleComponent>* visibleComponentPool = registry.getComponentPool<VisibleComponent>();

    for (int i=0;i<tfcPool->dense.size();i++) {
        Entity entityId = tfcPool->denseIds[i];
        const TransformComponent& tfc = tfcPool->dense[i];
        if (IsPositionInBounds(xMin, xMax, zMin, zMax, tfc.position)) {
            if (!visibleComponentPool->hasEntity(entityId)) {
                visibleComponentPool->addData(entityId);
            }
        } else {
            std::cout << tfc.position << "isn't in bounds" << std::endl;
            if (visibleComponentPool->hasEntity(entityId)) {
                visibleComponentPool->removeData(entityId);
            }
        }
    }
}