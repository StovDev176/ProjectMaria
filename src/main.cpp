#include "includes/ECS.hpp"
#include "includes/Math.hpp"
#include "includes/DataModel.hpp"
#include "includes/Physics.hpp"
#include "includes/TaskScheduler.hpp"
#include "includes/Renderer.hpp"
#include "includes/Components.hpp"
#include "includes/AssetManager.hpp"
#include "includes/ui/imgui_wrapper.hpp"
#include "includes/CameraService.hpp"
#include "includes/Logger.hpp"
#include "includes/HierarchyService.hpp"
#include <iostream>
#include <cmath>
#include <memory>
#include <vector>
#include <optional>
#include <chrono>



int main() {
    Registry registry;
    Entity player = registry.create();
    Entity enemy = registry.create();
    Entity mesh1 = registry.create();
    Entity baseplate = registry.create();
    ComponentPool<RigidBody>* rigidBodyPool = registry.getComponentPool<RigidBody>();
    ComponentPool<TransformComponent>* tfcPool = registry.getComponentPool<TransformComponent>();
    ComponentPool<MeshComponent>* meshPool = registry.getComponentPool<MeshComponent>();
    ComponentPool<AABB>* AABB_Pool = registry.getComponentPool<AABB>();
    ComponentPool<MaterialComponent>* materialPool = registry.getComponentPool<MaterialComponent>();
    ComponentPool<HierarchyComponent>* hierarchyComponentPool = registry.getComponentPool<HierarchyComponent>();
    AssetManager assetManager;
    rigidBodyPool->addData(
      RigidBody{
        vector3(1.0f, 2.0f, 0.0f),
        vector3(0.0f, 0.0f, 0.0f),
        5.0f,
        0.5f,
        vector3(0.0f, 0.0f, 0.0f)}, mesh1);
    rigidBodyPool->addData(
      RigidBody{
        vector3(0.0f, 0.0f, 0.0f), 
        vector3(0.0f, 0.0f, 0.0f),
        0.0f,
        0.5f,
        vector3(0.0f, 0.0f, 0.0f)}, baseplate);

    tfcPool->addData(TransformComponent{
      vector3(0.0f, 100.0f, 0.0f),
      vector3(90.0f, 20.0f, 0.0f),
      vector3(1.0f, 1.0f, 1.0f)}, mesh1);

    tfcPool->addData(TransformComponent{
      vector3(0.0f, -0.5f, 0.0f),
      vector3(0.0f, 0.0f, 0.0f),
      vector3(100.0f, 1.0f, 100.0f)
    }, baseplate);   
    hierarchyComponentPool->addData(HierarchyComponent{}, baseplate);
    hierarchyComponentPool->addData(HierarchyComponent{}, mesh1);

    meshPool->addData(MeshComponent(PrimitiveType::SPHERE, GREEN), mesh1);
    meshPool->addData(MeshComponent(PrimitiveType::CUBE, BLUE), baseplate);
    TaskScheduler taskScheduler;
    Camera3D camera = CreateCamera(); 
    Renderer renderer;

    taskScheduler.insertEvent(Event([&registry](float dt){
      registry.view<RigidBody, TransformComponent>([dt](Entity id, RigidBody& rb, TransformComponent& tfc){
        applyGravity(rb);
        updatePhysics(rb, tfc, dt); 
      });
    }, 1, false));

    taskScheduler.insertEvent(Event([&](float dt){
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        Ray raylibRay = GetMouseRay(mousePos, camera);

        Raycast raycast(
          vector3(raylibRay.position.x, raylibRay.position.y, raylibRay.position.z), 
          vector3(raylibRay.direction.x, raylibRay.direction.y, raylibRay.direction.z) 
        );

        registry.view<MeshComponent, TransformComponent>([&raycast](Entity id, MeshComponent& mesh, TransformComponent& tfC){
          Sphere tempSphere(tfC.position, tfC.scale.x);
            auto hit = tempSphere.intersect(raycast);

            if (hit != std::nullopt) {
              mesh.color = RED; 
            }
        });
      }  
    }, 4, false)); 

    taskScheduler.insertEvent(Event([&](float dt) {
        UpdateHierarchy(registry, tfcPool, hierarchyComponentPool);
    }, 1, false));


    taskScheduler.insertEvent(Event([&registry](float dt) {
      Update(registry);
    }, 2, false));
    Entity selectedEntity = mesh1;
    ImGuiIO& io = ImGui::GetIO();
    while (!WindowShouldClose()) {
      taskScheduler.update(GetFrameTime());
      UpdateCamera(camera, 20, GetFrameTime(), 0.003, io);
      renderer.Draw(registry, selectedEntity, *tfcPool, *meshPool, *rigidBodyPool, assetManager, camera, *materialPool, *hierarchyComponentPool);
    }
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}