#pragma once
#include "raylib.h"
#include "screen.h"

struct Card {
    Rectangle rect;
    Screen    target;
    const char* title;
    const char* desc;
    Color       accent;
    float       hoverAnim;  // 0..1
    float       animTime;   // continuous timer for illustration animation

    Card() = default;
    Card(Rectangle r, Screen t,
         const char* title, const char* desc, Color accent);

    bool Update();
    void Draw() const;

private:
    void DrawIllustration(Rectangle area) const;
};
