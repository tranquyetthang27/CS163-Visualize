#pragma once
#include "raylib.h"

struct Button {
    Rectangle rect;
    const char* label;
    Color baseColor;
    Color hoverColor;
    Color textColor;
    float fontSize;
    float anim;   // 0..1 hover lerp

    Button() = default;
    Button(Rectangle r, const char* lbl,
           Color base, Color hover,
           Color text = WHITE, float fs = 17.0f);

    bool Update();   // returns true on click
    void Draw() const;
};
