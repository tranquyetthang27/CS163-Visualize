#pragma once
#include "raylib.h"

extern Font fontBold;
extern Font fontRegular;

void LoadFonts();
void UnloadFonts();

void DrawTextCentered(Font f, const char* text, float y, float fontSize, Color color);
void DrawTextInRect(Font f, const char* text, Rectangle rect, float fontSize, Color color);
