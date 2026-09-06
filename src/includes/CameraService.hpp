#pragma once
#include "raylib.h"
#include "Math.hpp"
#include "Physics.hpp"
#include "GridService.hpp"
#include <vector>
#include "raymath.h"
#include <algorithm> 
#include <cmath>
#include "ui/imgui_wrapper.hpp"

void UpdateCameraTarget(float sensibility, Camera3D& camera) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        DisableCursor(); 
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        EnableCursor();  
    }

    if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) { return; }
    static float yaw = -1.57f;
    static float pitch = 0.0f;
    Vector2 delta = GetMouseDelta();
    yaw += delta.x * sensibility;
    pitch -= delta.y * sensibility;

    pitch = std::clamp(pitch, -1.55f, 1.55f);

    Vector3 direction(0.0f, 0.0f, 0.0f);
    direction.x = cosf(pitch) * cosf(yaw);
    direction.y = sinf(pitch);
    direction.z = cosf(pitch) * sinf(yaw);

    camera.target.x = camera.position.x + direction.x;
    camera.target.y = camera.position.y + direction.y;
    camera.target.z = camera.position.z + direction.z;
}

void UpdateFreeflyCamera(Camera3D& camera, float speed, float dt) {
    Vector3 moveDirection = { 0.0f, 0.0f, 0.0f };

    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    moveDirection = Vector3Add(moveDirection, forward);
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  moveDirection = Vector3Subtract(moveDirection, forward);
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveDirection = Vector3Add(moveDirection, right);
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  moveDirection = Vector3Subtract(moveDirection, right);

    if (Vector3Length(moveDirection) > 0.0f) {
        moveDirection = Vector3Normalize(moveDirection);
        
        Vector3 displacement = Vector3Scale(moveDirection, speed * dt);
        camera.position = Vector3Add(camera.position, displacement);
        camera.target   = Vector3Add(camera.target, displacement); 
    }
}

void UpdateCamera(Camera3D camera, float speed, float dt, float sensibility, ImGuiIO& io) {
    if (!io.WantCaptureMouse) {
        UpdateCameraTarget(0.003, camera);
        UpdateFreeflyCamera(camera, 20, GetFrameTime());
    }
}

Camera3D CreateCamera() {
    Camera3D camera = {0};
    camera.position = (Vector3){ 20.0f, 20.0f, 20.0f }; 
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          
    camera.fovy = 45.0f; 
    camera.projection = CAMERA_PERSPECTIVE; 
    return camera;
}

inline bool IsPositionInBounds(float xMin, float xMax, float zMin, float zMax, const vector3& pos) {
    return (pos.x > xMin && pos.x < xMax && pos.z > zMin && pos.z < zMax);
}
