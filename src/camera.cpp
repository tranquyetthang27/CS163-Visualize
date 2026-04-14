#include "camera.h"
#include "raymath.h"

void UpdateCameraZoom(Camera2D &camera) {
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        
        camera.offset = GetMousePosition();
        camera.target = mouseWorldPos;

        const float zoomIncrement = 0.125f;
        camera.zoom += (wheel * zoomIncrement);
        
        if (camera.zoom < 0.1f) camera.zoom = 0.1f;
    }
}

void UpdateCameraPan(Camera2D &camera) {
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        camera.target.x -= delta.x / camera.zoom;
        camera.target.y -= delta.y / camera.zoom;
    }
}
