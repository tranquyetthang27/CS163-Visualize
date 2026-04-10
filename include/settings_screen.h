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
    Button btnNoMusic, btnMusic1, btnMusic2;
    Button btnBack, btnTestSFX, btnOk;
    int    origMusicIdx;
    int    origBgIdx;
    int    pendingMusicIdx;
    int    pendingBgIdx;

public:
    SettingsScreen();
    Screen Update();
    void   Draw() const;
};
