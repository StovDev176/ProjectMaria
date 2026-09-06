#pragma once
#include <iostream>
#include "Math.hpp"
#include "raylib.h"
#include "raymath.h"
#include "Physics.hpp"
#include "ECS.hpp"
#include "ui/imgui_wrapper.hpp"
#include "AssetManager.hpp"
#include "Logger.hpp"
#include "VisibilityService.hpp"
#include <cstring>
#include "ui/ImGuizmo.h"
#include "HierarchyService.hpp"

void ApplyModernTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.WindowRounding    = 6.0f;
    style.ChildRounding     = 4.0f;
    style.FrameRounding     = 4.0f;
    style.PopupRounding     = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding        = 4.0f;

    style.WindowPadding     = ImVec2(10, 10);
    style.FramePadding      = ImVec2(6, 4);
    style.ItemSpacing       = ImVec2(8, 6);
    style.WindowBorderSize  = 1.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]           = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_Header]             = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.28f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.35f, 0.38f, 0.48f, 1.00f);
    colors[ImGuiCol_Button]             = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.28f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.35f, 0.38f, 0.48f, 1.00f);
    colors[ImGuiCol_FrameBg]            = ImVec4(0.16f, 0.17f, 0.21f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.22f, 0.24f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.28f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_TitleBg]            = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.14f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    colors[ImGuiCol_CheckMark]          = ImVec4(0.40f, 0.65f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]         = ImVec4(0.40f, 0.65f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.50f, 0.75f, 1.00f, 1.00f);
}


class Renderer {
public:
    Renderer() {
        InitWindow(1200, 650, "Project Maria");
        SetTargetFPS(30);
        rlImGuiSetup(true);
        ApplyModernTheme();
    }   
    void beginFrame(const Camera3D& cam) {
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(cam);
    } 
    void drawUI() {
        DrawFPS(10, 10);
    
        DrawText("Project Maria Engine", 10, 40, 20, RAYWHITE);
    }

    void Draw(Registry& registry, Entity& selectedEntity, ComponentPool<TransformComponent>& tfcPool, ComponentPool<MeshComponent>& meshPool, ComponentPool<RigidBody>& rigidBodyPool, AssetManager& assetManager, Camera3D& camera, ComponentPool<MaterialComponent>& matPool, ComponentPool<HierarchyComponent>& hierarchyComponentPool) {
      beginFrame(camera);
      evaluateVisiblity(camera, &tfcPool, 120, registry);
      registry.view<MeshComponent, TransformComponent, VisibleComponent>([&](Entity id, MeshComponent& mesh, TransformComponent& tfc, VisibleComponent& visibleComponent) {
        if (mesh.type == MeshType::PRIMITIVE) {

          Vector3 pos = tfc.worldMatrix.GetTranslation();
          Vector3 scale = tfc.worldMatrix.GetScale();
          Vector3 rotation = tfc.worldMatrix.GetRotation();
          switch (mesh.primitive) {
            case PrimitiveType::CUBE: {
                DrawCubeV(pos, scale, mesh.color);
                break;
            }    
            case PrimitiveType::SPHERE: {
                DrawSphere(pos, scale.x, mesh.color);
                break;
            }    
          }
        } else if (mesh.type == MeshType::MODEL) {
          Model& model = assetManager.GetAsset<Model>(mesh.assetId);
          MaterialComponent* mat = matPool.getComponent(id);
          DrawModelWithMaterial(model, *mat, tfc, assetManager, mesh);
        }
      });

      EndMode3D();
      drawUI();
      rlImGuiBegin();
      drawECSInspector(registry, selectedEntity, &tfcPool, &meshPool, &rigidBodyPool);

      if (selectedEntity != UINT32_MAX && tfcPool.hasEntity(selectedEntity) && hierarchyComponentPool.hasEntity(selectedEntity)) {
        TransformComponent* tfc = tfcPool.getComponent(selectedEntity);
        HierarchyComponent* hierarchyComponent = hierarchyComponentPool.getComponent(selectedEntity);
        drawGizmo(registry ,camera, *tfc, *hierarchyComponent);
      }
      rlImGuiEnd();
      EndDrawing();
      
    }
 void drawGizmo(Registry& registry,  Camera3D& camera, TransformComponent& tfc,  HierarchyComponent& hierarchyComponent) {
    ImGuizmo::BeginFrame();
    ImGuizmo::Enable(true);
    
    ImGui::PushID(0);

    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
    ImGuizmo::SetRect(0, 0, (float)GetScreenWidth(), (float)GetScreenHeight());
    Matrix4 viewMatrix, projMatrix;
    Matrix rlViewMatrix = GetCameraMatrix(camera);
    Matrix rlProjMatrix = MatrixPerspective(
        camera.fovy * (PI / 180.0f), 
        (float)GetScreenWidth() / (float)GetScreenHeight(), 
        0.01f, 1000.0f
    );

    std::memcpy(viewMatrix.m, &rlViewMatrix, sizeof(float) * 16);
    std::memcpy(projMatrix.m, &rlProjMatrix, sizeof(float) * 16);

    static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
    if (IsKeyPressed(KEY_W)) currentOp = ImGuizmo::TRANSLATE;
    if (IsKeyPressed(KEY_E)) currentOp = ImGuizmo::ROTATE;
    if (IsKeyPressed(KEY_R)) currentOp = ImGuizmo::SCALE;

    Matrix4 modifiedWorld = tfc.worldMatrix;
    ImGuizmo::Manipulate(
        &viewMatrix.m[0], 
        &projMatrix.m[0], 
        currentOp, 
        ImGuizmo::LOCAL, 
        &modifiedWorld.m[0]
    );

    TransformComponent* parentWorld = registry.getComponentPool<TransformComponent>()->getComponent(hierarchyComponent.parentID);


    if (parentWorld) {
      Matrix4 localMatrix = parentWorld->worldMatrix.Invert() * modifiedWorld;
      tfc.position = localMatrix.GetTranslation();
      tfc.rotation = localMatrix.GetRotation();
      tfc.scale    = localMatrix.GetScale();
    } else {
      tfc.position = modifiedWorld.GetTranslation();
      tfc.rotation = modifiedWorld.GetRotation();
      tfc.scale    = modifiedWorld.GetScale();
    }

    ImGui::PopID();
}

  void DrawModelWithMaterial(Model& model, MaterialComponent& materialComponent, TransformComponent& tfc, AssetManager& assetManager,MeshComponent& meshComponent) {
    if (meshComponent.type != MeshType::MODEL) {
      Logger::log(LogLevel::INFO, "Please submit a model and not a primitive object!");
      return;
    }
    if (!materialComponent.shaderId.empty()) {
        const ShaderAsset& shaderAsset = assetManager.GetAsset<ShaderAsset>(materialComponent.shaderId);
        
        model.materials[0].shader = shaderAsset.shader;

        auto roughnessIt = shaderAsset.locations.find("u_roughness");
        if (roughnessIt != shaderAsset.locations.end() && roughnessIt->second != -1) {
            SetShaderValue(shaderAsset.shader, roughnessIt->second, &materialComponent.roughness, SHADER_UNIFORM_FLOAT);
        }

        auto metallicIt = shaderAsset.locations.find("u_metallic");
        if (metallicIt != shaderAsset.locations.end() && metallicIt->second != -1) {
            SetShaderValue(shaderAsset.shader, metallicIt->second, &materialComponent.metallic, SHADER_UNIFORM_FLOAT);
        }     
    }
    if (!materialComponent.textureId.empty()) {
      Texture2D& texture = assetManager.GetAsset<Texture2D>(materialComponent.textureId);
      model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    }

    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = materialComponent.tint;
    Matrix raylibMatrix;
    std::memcpy(&raylibMatrix, &tfc.worldMatrix, sizeof(float) * 16);
    model.transform = raylibMatrix;
    DrawModel(model, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, meshComponent.color);

  }
};