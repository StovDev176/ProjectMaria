#pragma once
#include "Math.hpp"
#include "ECS.hpp"
#include "raylib.h"
#include "Physics.hpp"
#include "Components.hpp"
#include <iostream>
#include <vector>
#include "ui/imgui_wrapper.hpp"

bool DrawRaylibColorPicker(const char* label, Color& color) {
    float col[4] = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f
    };

    if (ImGui::ColorEdit4(label, col)) {
        color.r = (unsigned char)(col[0] * 255.0f);
        color.g = (unsigned char)(col[1] * 255.0f);
        color.b = (unsigned char)(col[2] * 255.0f);
        color.a = (unsigned char)(col[3] * 255.0f);
        return true; 
    }

    return false;
}

void drawECSInspector(Registry& registry, Entity& selectedEntity, 
                      ComponentPool<TransformComponent>* tfcPool, 
                      ComponentPool<MeshComponent>* meshPool, 
                      ComponentPool<RigidBody>* rigidBodyPool) 
{
    ImGui::Begin("Entity hierarchy");
    for (size_t i = 0; i < tfcPool->denseIds.size(); i++) {
        Entity id = tfcPool->denseIds[i];
        
        std::string label = "Entity " + std::to_string(id);
        
        if (ImGui::Selectable(label.c_str(), selectedEntity == id)) {
            selectedEntity = id;
        }
    }
    ImGui::End();

    ImGui::Begin("Inspector");
    
    ImGui::Text("Entity selected : %d", selectedEntity);
    ImGui::Separator();

    TransformComponent* tfc = tfcPool->getComponent(selectedEntity);
    if (tfc != nullptr) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", &tfc->position.x, 0.1f);
            ImGui::DragFloat3("Rotation", &tfc->rotation.x, 1.0f);
            ImGui::DragFloat3("Scale",  &tfc->scale.x,    0.1f);
        }
    }

    MeshComponent* mesh = meshPool->getComponent(selectedEntity);
    if (mesh != nullptr) {
        if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawRaylibColorPicker("Mesh color", mesh->color);
        }
    }

    RigidBody* rb = rigidBodyPool->getComponent(selectedEntity);
    if (rb != nullptr) {
        if (ImGui::CollapsingHeader("Rigid Body", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat("Mass", &rb->mass, 0.1f, 0.0f, 100.0f);
            ImGui::SliderFloat("Rebound (Bounciness)", &rb->bounciness, 0.0f, 1.0f);
        }
    }

    ImGui::End();
}