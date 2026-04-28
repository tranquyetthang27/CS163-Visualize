#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"


Camera2D InitSceneCamera();


void UpdateCameraZoom(Camera2D &camera);

void UpdateCameraPan(Camera2D &camera);

#endif
