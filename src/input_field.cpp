#include "input_field.h"
#include "font.h"
#include "colors.h"

InputField::InputField(Rectangle r, const char* ph, int ml)
    : rect(r), placeholder(ph), focused(false), cursorBlink(0.0f), maxLen(ml) {}

void InputField::Update() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        focused = CheckCollisionPointRec(GetMousePosition(), rect);

    if (!focused) return;

    cursorBlink += GetFrameTime();
    if (cursorBlink > 1.0f) cursorBlink = 0.0f;

    // Character input
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && (int)text.size() < maxLen)
            text += (char)key;
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !text.empty())
        text.pop_back();
}

void InputField::Draw() const {
    bool isFocused = focused;

    // Border glow when focused
    if (isFocused) {
        Rectangle glow = {rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4};
        DrawRectangleRounded(glow, 0.25f, 8, {63, 81, 181, 60});
    }

    DrawRectangleRounded(rect, 0.25f, 8, Pal::Surface);

    Color borderCol = isFocused ? Pal::InputFocus : Pal::InputBorder;
    // Draw border via two rects
    Rectangle outer = {rect.x - 1.5f, rect.y - 1.5f, rect.width + 3, rect.height + 3};
    DrawRectangleRounded(outer, 0.27f, 8, borderCol);
    DrawRectangleRounded(rect,  0.25f, 8, Pal::Surface);

    const char* display = text.empty() ? placeholder : text.c_str();
    Color tc            = text.empty() ? Pal::TxtLight : Pal::TxtDark;

    Vector2 ts = MeasureTextEx(fontRegular, display, 17.0f, 1.0f);
    float ty   = rect.y + rect.height / 2 - ts.y / 2;
    DrawTextEx(fontRegular, display, {rect.x + 10, ty}, 17.0f, 1.0f, tc);

    // Cursor blink
    if (isFocused && cursorBlink < 0.5f) {
        Vector2 tw = MeasureTextEx(fontRegular, text.c_str(), 17.0f, 1.0f);
        float cx   = rect.x + 10 + tw.x + 2;
        DrawLineEx({cx, rect.y + 6}, {cx, rect.y + rect.height - 6}, 2.0f, Pal::InputFocus);
    }
}
