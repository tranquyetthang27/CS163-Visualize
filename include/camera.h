#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"

// Khoi tao camera mac dinh
Camera2D InitSceneCamera();

// Xu ly thu phong tai vi tri con tro chuot
void UpdateCameraZoom(Camera2D &camera);

// Xu ly di chuyen man hinh bang chuot phai
void UpdateCameraPan(Camera2D &camera);

#endif
