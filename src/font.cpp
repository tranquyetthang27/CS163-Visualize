#include "font.h"
#include "raylib.h"

Font fontBold;
Font fontRegular;

static Font TryLoadFont(const char* paths[], int n, int size) {
    for (int i = 0; i < n; i++) {
        if (FileExists(paths[i])) {
            Font f = LoadFontEx(paths[i], size, nullptr, 512);
            SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
            GenTextureMipmaps(&f.texture);
            return f;
        }
    }
    return GetFontDefault();
}

void LoadFonts() {
    const char* boldPaths[] = {
        "/Users/tranquochuy/Library/Fonts/JetBrainsMono-Bold.ttf",
        "/Users/tranquochuy/Downloads/JetBrainsMono-2/fonts/ttf/JetBrainsMono-Bold.ttf",
        "fonts/Roboto-Bold.ttf"
    };
    const char* regPaths[] = {
        "/Users/tranquochuy/Library/Fonts/JetBrainsMono-Regular.ttf",
        "/Users/tranquochuy/Downloads/JetBrainsMono-2/fonts/ttf/JetBrainsMono-Regular.ttf",
        "fonts/regular.ttf"
    };
    // Load at 2× the max display size → downsample = crisp text
    fontBold    = TryLoadFont(boldPaths, 3, 128);
    fontRegular = TryLoadFont(regPaths,  3, 128);
}

void UnloadFonts() {
    UnloadFont(fontBold);
    UnloadFont(fontRegular);
}

void DrawTextCentered(Font f, const char* text, float y, float fontSize, Color color) {
    Vector2 sz = MeasureTextEx(f, text, fontSize, 1.0f);
    DrawTextEx(f, text, {(GetScreenWidth() - sz.x) / 2.0f, y}, fontSize, 1.0f, color);
}

void DrawTextInRect(Font f, const char* text, Rectangle rect, float fontSize, Color color) {
    Vector2 sz = MeasureTextEx(f, text, fontSize, 1.0f);
    DrawTextEx(f, text,
        {rect.x + rect.width/2 - sz.x/2, rect.y + rect.height/2 - sz.y/2},
        fontSize, 1.0f, color);
}
