#pragma once
#include "raylib.h"
#include <string>

struct InputField {
    Rectangle rect;
    std::string text;
    const char* placeholder;
    bool focused;
    float cursorBlink;
    int maxLen;

    InputField() = default;
    InputField(Rectangle r, const char* ph, int ml = 12);

    void Update();
    void Draw() const;
    void Clear() { text.clear(); }
    bool IsEmpty() const { return text.empty(); }
};
