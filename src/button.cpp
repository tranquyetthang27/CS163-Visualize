#include "button.h"
#include "font.h"

Button::Button(Rectangle r, const char* lbl,
               Color base, Color hover,
               Color text, float fs)
    : rect(r), label(lbl),
      baseColor(base), hoverColor(hover),
      textColor(text), fontSize(fs), anim(0.0f) {}

bool Button::Update() {
    bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
    float speed  = 10.0f * GetFrameTime();
    anim += ((hovered ? 1.0f : 0.0f) - anim) * speed;
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void Button::Draw() const {
    auto lerpByte = [](unsigned char a, unsigned char b, float t) {
        return (unsigned char)(a + (b - a) * t);
    };
    Color c = {
        lerpByte(baseColor.r, hoverColor.r, anim),
        lerpByte(baseColor.g, hoverColor.g, anim),
        lerpByte(baseColor.b, hoverColor.b, anim),
        255
    };

    // subtle lift shadow when hovered
    if (anim > 0.05f) {
        Rectangle shadow = {rect.x + 2, rect.y + 3, rect.width, rect.height};
        DrawRectangleRounded(shadow, 0.3f, 8, {0, 0, 0, (unsigned char)(30 * anim)});
    }

    DrawRectangleRounded(rect, 0.3f, 8, c);
    DrawTextInRect(fontBold, label, rect, fontSize, textColor);
}
