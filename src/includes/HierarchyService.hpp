#pragma once
#include <iostream>
#include "ECS.hpp"
#include "Physics.hpp"
#include "Math.hpp"

struct HierarchyComponent {
    Entity parentID = UINT32_MAX;
    std::vector<Entity> children;
};

void UpdateHierarchy(Registry& registry, ComponentPool<TransformComponent>* tfcPool, ComponentPool<HierarchyComponent>* hierarchyComponentPool) {
    registry.view<TransformComponent>([&](Entity id, TransformComponent& tfc) {
        tfc.localMatrix = Matrix4::CreateTRS(tfc.position, tfc.rotation, tfc.scale);
    });

    registry.view<TransformComponent, HierarchyComponent>([&](Entity id, TransformComponent& tfc, HierarchyComponent& hierarchy) {
        if (hierarchy.parentID == UINT32_MAX) {
            tfc.worldMatrix = tfc.localMatrix;

            auto propagateToChildren = [&](auto& self, Entity parentId, const Matrix4& parentWorldMatrix) -> void {
                const HierarchyComponent* parentHierarchy = hierarchyComponentPool->getComponent(parentId);
                if (!parentHierarchy) return;

                for (Entity childId : parentHierarchy->children) {
                    TransformComponent* childTfc = tfcPool->getComponent(childId);
                    if (childTfc) {
                        childTfc->worldMatrix = parentWorldMatrix * childTfc->localMatrix;
                        self(self, childId, childTfc->worldMatrix);
                    }
                }
            };
            propagateToChildren(propagateToChildren, id, tfc.worldMatrix);
        }
    });
}