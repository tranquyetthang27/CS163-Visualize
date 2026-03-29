#include "settings_screen.h"
#include "font.h"
#include "colors.h"
#include "app_settings.h"
#include "audio_manager.h"
#include <cstdio>

// ---- Slider ----

Slider::Slider(Rectangle r, float v, const char* lbl)
    : track(r), value(v), dragging(false), label(lbl) {}

bool Slider::Update() {
    Vector2 mp = GetMousePosition();
    bool wasChanged = false;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mp, {track.x - 10, track.y - 12,
                                    track.width + 20, track.height + 24}))
        dragging = true;

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) dragging = false;

    if (dragging) {
        float newVal = (mp.x - track.x) / track.width;
        newVal = newVal < 0 ? 0 : (newVal > 1 ? 1 : newVal);
        if (newVal != value) { value = newVal; wasChanged = true; }
    }
    return wasChanged;
}

void Slider::Draw() const {
    // Label
    DrawTextEx(fontBold, label,
               {track.x, track.y - 26}, 15.0f, 1.0f, Pal::TxtDark);

    // Percentage
    char buf[8]; snprintf(buf, sizeof(buf), "%d%%", (int)(value * 100));
    Vector2 ps = MeasureTextEx(fontRegular, buf, 14.0f, 1.0f);
    DrawTextEx(fontRegular, buf,
               {track.x + track.width + 12, track.y - 1}, 14.0f, 1.0f, Pal::TxtMid);

    // Track background
    DrawRectangleRounded(track, 1.0f, 6, Pal::PanelDark);

    // Filled portion
    Rectangle filled = {track.x, track.y, track.width * value, track.height};
    if (filled.width > 0)
        DrawRectangleRounded(filled, 1.0f, 6, Pal::Indigo);

    // Thumb
    float thumbX = track.x + track.width * value;
    float thumbY = track.y + track.height / 2.0f;
    DrawCircleV({thumbX, thumbY}, 10.0f, Pal::Surface);
    DrawCircleV({thumbX, thumbY},  8.0f, Pal::Indigo);
}

// ---- SettingsScreen ----

SettingsScreen::SettingsScreen()
    : sliderMusic({240, 220, 480, 10}, gSettings.musicVolume, "Music Volume"),
      sliderSFX  ({240, 310, 480, 10}, gSettings.sfxVolume,   "Sound Effects Volume"),
      btnBack    ({40,  640, 130, 44}, "< Back",    Pal::BtnNeutral, Pal::BtnNeutHov),
      btnTestSFX ({750, 640, 160, 44}, "Test Sound", Pal::BtnOrange,  Pal::BtnOrangeHov),
      prevBgIdx(gSettings.bgColorIdx)
{}

Screen SettingsScreen::Update() {
    if (btnBack.Update() || IsKeyPressed(KEY_ESCAPE)) {
        AudioPlayBack();
        return Screen::MainMenu;
    }
    if (btnTestSFX.Update()) AudioPlaySuccess();

    if (sliderMusic.Update()) AudioSetMusicVolume(sliderMusic.value);
    if (sliderSFX.Update())   AudioSetSFXVolume(sliderSFX.value);

    // Background color swatches
    float swatchX = 240.0f, swatchY = 430.0f;
    float sw = 70.0f, sh = 44.0f, sgap = 14.0f;
    for (int i = 0; i < AppSettings::bgColorCount; i++) {
        Rectangle r = {swatchX + i * (sw + sgap), swatchY, sw, sh};
        if (CheckCollisionPointRec(GetMousePosition(), r) &&
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (gSettings.bgColorIdx != i) {
                gSettings.bgColorIdx = i;
                AudioPlayClick();
            }
        }
    }

    return Screen::Settings;
}

void SettingsScreen::Draw() const {
    ClearBackground({245, 247, 252, 255});

    // Header
    DrawRectangleRec({0, 0, 1280, 80}, Pal::Surface);
    DrawLineEx({0, 80}, {1280, 80}, 1.0f, Pal::Border);
    DrawTextEx(fontBold, "Settings", {50, 22}, 32.0f, 1.0f, Pal::TxtDark);
    btnBack.Draw();

    // Panel
    DrawRectangleRounded({180, 110, 920, 540}, 0.05f, 8, Pal::Surface);

    // Section: Audio
    DrawTextEx(fontBold, "Audio", {240, 155}, 18.0f, 1.0f, Pal::Indigo);
    DrawLineEx({240, 178}, {840, 178}, 1.0f, Pal::Border);

    sliderMusic.Draw();
    sliderSFX.Draw();

    btnTestSFX.Draw();

    // Section: Background Color
    DrawTextEx(fontBold, "Background Color", {240, 386}, 18.0f, 1.0f, Pal::Indigo);
    DrawLineEx({240, 409}, {840, 409}, 1.0f, Pal::Border);

    float swatchX = 240.0f, swatchY = 430.0f;
    float sw = 70.0f, sh = 44.0f, sgap = 14.0f;

    for (int i = 0; i < AppSettings::bgColorCount; i++) {
        Rectangle r = {swatchX + i * (sw + sgap), swatchY, sw, sh};
        bool selected = (gSettings.bgColorIdx == i);

        // Border when selected
        if (selected) {
            Rectangle sel = {r.x - 3, r.y - 3, r.width + 6, r.height + 6};
            DrawRectangleRounded(sel, 0.2f, 6, Pal::Indigo);
        }
        DrawRectangleRounded(r, 0.2f, 6, AppSettings::bgColors[i]);

        // Name
        Vector2 ts = MeasureTextEx(fontRegular, AppSettings::bgColorNames[i], 11.0f, 1.0f);
        DrawTextEx(fontRegular, AppSettings::bgColorNames[i],
                   {r.x + r.width/2 - ts.x/2, r.y + r.height + 6},
                   11.0f, 1.0f, Pal::TxtMid);
    }

    // Preview
    DrawTextEx(fontRegular, "Preview:", {240, 502}, 14.0f, 1.0f, Pal::TxtMid);
    Rectangle preview = {340, 497, 200, 36};
    DrawRectangleRounded(preview, 0.2f, 6, gSettings.GetBG());
    DrawRectangleRoundedLines(preview, 0.2f, 6, Pal::Border);
}
