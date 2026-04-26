#include "input_field.h"
#include "font.h"
#include "colors.h"

InputField::InputField(Rectangle r, const char* ph, int ml)
    : rect(r), placeholder(ph), focused(false), cursorBlink(0.0f), maxLen(ml) {}

static char KeyCodeToChar(int key) {
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    if (key >= KEY_A && key <= KEY_Z)
        return shift ? ('A' + key - KEY_A) : ('a' + key - KEY_A);
    if (key >= KEY_ZERO && key <= KEY_NINE) {
        static const char shifted[] = ")!@#$%^&*(";
        return shift ? shifted[key - KEY_ZERO] : ('0' + key - KEY_ZERO);
    }
    switch (key) {
        case KEY_SPACE:         return ' ';
        case KEY_MINUS:         return shift ? '_' : '-';
        case KEY_EQUAL:         return shift ? '+' : '=';
        case KEY_SLASH:         return shift ? '?' : '/';
        case KEY_PERIOD:        return shift ? '>' : '.';
        case KEY_COMMA:         return shift ? '<' : ',';
        case KEY_SEMICOLON:     return shift ? ':' : ';';
        case KEY_APOSTROPHE:    return shift ? '"' : '\'';
        case KEY_LEFT_BRACKET:  return shift ? '{' : '[';
        case KEY_RIGHT_BRACKET: return shift ? '}' : ']';
        case KEY_BACKSLASH:     return shift ? '|' : '\\';
        case KEY_GRAVE:         return shift ? '~' : '`';
        default:                return 0;
    }
}

static void ProcessKeys(std::string& text, int maxLen) {
    int key = GetKeyPressed();
    while (key > 0) {
        char ch = KeyCodeToChar(key);
        if (ch >= 32 && (int)text.size() < maxLen)
            text += ch;
        key = GetKeyPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !text.empty())
        text.pop_back();
}

void InputField::Update() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        if (CheckCollisionPointRec(GetMousePosition(), rect))
            focused = true;

    if (!focused) return;

    cursorBlink += GetFrameTime();
    if (cursorBlink > 1.0f) cursorBlink = 0.0f;

    ProcessKeys(text, maxLen);
}

void InputField::UpdateFocused() {
    focused = true;
    cursorBlink += GetFrameTime();
    if (cursorBlink > 1.0f) cursorBlink = 0.0f;

    ProcessKeys(text, maxLen);
}

void InputField::Draw() const {
    bool isFocused = focused;

    if (isFocused) {
        Rectangle glow = {rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4};
        DrawRectangleRounded(glow, 0.25f, 8, {63, 81, 181, 60});
    }

    DrawRectangleRounded(rect, 0.25f, 8, Pal::Surface);

    Color borderCol = isFocused ? Pal::InputFocus : Pal::InputBorder;
    Rectangle outer = {rect.x - 1.5f, rect.y - 1.5f, rect.width + 3, rect.height + 3};
    DrawRectangleRounded(outer, 0.27f, 8, borderCol);
    DrawRectangleRounded(rect,  0.25f, 8, Pal::Surface);

    const char* display = text.empty() ? placeholder : text.c_str();
    Color tc            = text.empty() ? Pal::TxtLight : Pal::TxtDark;

    Vector2 ts = MeasureTextEx(fontRegular, display, 17.0f, 1.0f);
    float ty   = rect.y + rect.height / 2 - ts.y / 2;
    DrawTextEx(fontRegular, display, {rect.x + 10, ty}, 17.0f, 1.0f, tc);

    if (isFocused && cursorBlink < 0.5f) {
        Vector2 tw = MeasureTextEx(fontRegular, text.c_str(), 17.0f, 1.0f);
        float cx   = rect.x + 10 + tw.x + 2;
        DrawLineEx({cx, rect.y + 6}, {cx, rect.y + rect.height - 6}, 2.0f, Pal::InputFocus);
    }
}
