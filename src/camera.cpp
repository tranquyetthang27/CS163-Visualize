#include "camera.h"
#include "raymath.h"

static float targetZoom = 1.0f;
static const float MIN_ZOOM = 0.05f;
static const float MAX_ZOOM = 10.0f;

void UpdateCameraZoom(Camera2D &camera) {

    float wheel = GetMouseWheelMove();
    if (IsKeyDown(KEY_EQUAL)) wheel += 0.05f; 
    if (IsKeyDown(KEY_MINUS)) wheel -= 0.05f; 

    if (wheel != 0) {

        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
 
        camera.offset = GetMousePosition();
        camera.target = mouseWorldPos;

        targetZoom += (wheel * 0.15f * targetZoom); 
        if (targetZoom < MIN_ZOOM) targetZoom = MIN_ZOOM;
        if (targetZoom > MAX_ZOOM) targetZoom = MAX_ZOOM;
    }

    camera.zoom = Lerp(camera.zoom, targetZoom, 0.15f);

    if (IsKeyPressed(KEY_ZERO) || IsKeyPressed(KEY_R)) {
        targetZoom = 1.0f;
        camera.zoom = 1.0f;
        camera.target = (Vector2){ 640, 140 };
        camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/3.0f };
    }
}

void UpdateCameraPan(Camera2D &camera) {
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta();
        camera.target.x -= delta.x / camera.zoom;
        camera.target.y -= delta.y / camera.zoom;
    }

    float speed = 5.0f / camera.zoom;
    if (IsKeyDown(KEY_RIGHT)) camera.target.x += speed;
    if (IsKeyDown(KEY_LEFT))  camera.target.x -= speed;
    if (IsKeyDown(KEY_UP))    camera.target.y -= speed;
    if (IsKeyDown(KEY_DOWN))  camera.target.y += speed;
}