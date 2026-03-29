#pragma once
#include "screen.h"
#include "button.h"

struct Slider {
    Rectangle track;
    float     value;   // 0..1
    bool      dragging;
    const char* label;

    Slider() = default;
    Slider(Rectangle r, float v, const char* lbl);
    bool Update();   // returns true if changed
    void Draw() const;
};

class SettingsScreen {
    Slider sliderMusic, sliderSFX;
    Button btnBack, btnTestSFX;
    int    prevBgIdx;

public:
    SettingsScreen();
    Screen Update();
    void   Draw() const;
};
