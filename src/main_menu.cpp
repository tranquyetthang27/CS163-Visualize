#include "main_menu.h"
#include "font.h"
#include "colors.h"
#include "audio_manager.h"
#include <cmath>
#include <cstdlib>

static constexpr Color BG_DARK  = {12,  18,  38,  255};
static constexpr Color BG_NODE  = {63,  81, 181,  60};
static constexpr Color BG_EDGE  = {63,  81, 181,  25};
static constexpr Color ACCENT   = {100, 130, 255, 255};

// Button layout: centered, 300×52, spaced 20px
static constexpr float BTN_W  = 300.0f;
static constexpr float BTN_H  = 52.0f;
static constexpr float BTN_SP = 18.0f;
static constexpr float BTN_X  = (1280.0f - BTN_W) / 2.0f;
static constexpr float BTN_Y0 = 320.0f;

MainMenu::MainMenu()
    : btnStart   ({BTN_X, BTN_Y0,               BTN_W, BTN_H}, "Start",
                  Pal::Indigo, Pal::IndigoDark),
      btnSettings({BTN_X, BTN_Y0 + (BTN_H+BTN_SP),   BTN_W, BTN_H}, "Settings",
                  {50, 65, 100, 255}, {38, 50, 80, 255}),
      btnAbout   ({BTN_X, BTN_Y0 + 2*(BTN_H+BTN_SP), BTN_W, BTN_H}, "About Us",
                  {50, 65, 100, 255}, {38, 50, 80, 255}),
      btnExit    ({BTN_X, BTN_Y0 + 3*(BTN_H+BTN_SP), BTN_W, BTN_H}, "Exit",
                  {120, 40, 40, 255}, {160, 40, 40, 255}),
      time(0.0f)
{
    // Scatter background nodes randomly but deterministically
    srand(42);
    for (auto& n : bgNodes) {
        n.x  = (float)(rand() % 1280);
        n.y  = (float)(rand() % 720);
        n.vx = ((rand() % 100) - 50) * 0.003f;
        n.vy = ((rand() % 100) - 50) * 0.003f;
        n.r  = 3.0f + (rand() % 4);
        n.pulse = (float)(rand() % 100) / 100.0f;
    }
}

// Track previous hover state for sound trigger
static bool prevHover[4] = {};

Screen MainMenu::Update() {
    float dt = GetFrameTime();
    time += dt;

    // Animate background nodes (wrap around edges)
    for (auto& n : const_cast<MainMenu*>(this)->bgNodes) {
        n.x += n.vx;
        n.y += n.vy;
        n.pulse += dt * 0.8f;
        if (n.x < 0)    n.x += 1280;
        if (n.x > 1280) n.x -= 1280;
        if (n.y < 0)    n.y += 720;
        if (n.y > 720)  n.y -= 720;
    }

    // Hover sounds
    bool hov[4] = {
        CheckCollisionPointRec(GetMousePosition(), btnStart.rect),
        CheckCollisionPointRec(GetMousePosition(), btnSettings.rect),
        CheckCollisionPointRec(GetMousePosition(), btnAbout.rect),
        CheckCollisionPointRec(GetMousePosition(), btnExit.rect),
    };
    for (int i = 0; i < 4; i++)
        if (hov[i] && !prevHover[i]) AudioPlayHover();
    for (int i = 0; i < 4; i++) prevHover[i] = hov[i];

    if (btnStart.Update())    { AudioPlayClick(); return Screen::Home;     }
    if (btnSettings.Update()) { AudioPlayClick(); return Screen::Settings; }
    if (btnAbout.Update())    { AudioPlayClick(); return Screen::About;    }
    if (btnExit.Update())     { AudioPlayBack();  CloseWindow(); return Screen::MainMenu; }

    // ESC or window close → exit handled in main
    return Screen::MainMenu;
}

bool MainMenuWantsExit() { return false; } // signal via main

void MainMenu::Draw() const {
    // Dark background
    ClearBackground(BG_DARK);

    // Background: draw edges between nearby nodes
    for (int i = 0; i < BG_NODE_COUNT; i++) {
        for (int j = i + 1; j < BG_NODE_COUNT; j++) {
            float dx = bgNodes[i].x - bgNodes[j].x;
            float dy = bgNodes[i].y - bgNodes[j].y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist < 200.0f) {
                unsigned char alpha = (unsigned char)(25 * (1.0f - dist / 200.0f));
                DrawLineEx({bgNodes[i].x, bgNodes[i].y},
                           {bgNodes[j].x, bgNodes[j].y},
                           1.0f, {100, 130, 255, alpha});
            }
        }
    }

    // Background: draw nodes
    for (const auto& n : bgNodes) {
        float pulse = 0.7f + 0.3f * sinf(n.pulse * 2.0f);
        unsigned char alpha = (unsigned char)(50 * pulse);
        DrawCircleV({n.x, n.y}, n.r, {100, 130, 255, alpha});
    }

    // Centered panel glow
    float glow = 0.85f + 0.15f * sinf(time * 1.5f);
    DrawRectangle(0, 0, 1280, 720, {0, 0, 0, (unsigned char)(30 * glow)});

    // ---- Title ----
    const char* title = "Data Structure Visualization";
    Vector2 tsz = MeasureTextEx(fontBold, title, 44.0f, 1.0f);
    float tx = 640.0f - tsz.x / 2.0f;
    float ty = 140.0f;

    // Subtle glow behind title
    DrawRectangle((int)(tx - 30), (int)(ty - 10), (int)(tsz.x + 60), (int)(tsz.y + 20),
                  {63, 81, 181, 18});

    DrawTextEx(fontBold, title, {tx + 2, ty + 2}, 44.0f, 1.0f, {0, 0, 0, 60});
    DrawTextEx(fontBold, title, {tx, ty},          44.0f, 1.0f, WHITE);

    // Subtitle
    const char* sub = "Choose an option to begin";
    DrawTextCentered(fontRegular, sub, 200.0f, 17.0f, {180, 195, 220, 255});

    // Thin accent line under title
    float lw = tsz.x * 0.6f;
    DrawRectangle((int)(640 - lw/2), (int)(ty + tsz.y + 8), (int)lw, 2,
                  {100, 130, 255, 180});

    // ---- Buttons ----
    const_cast<Button&>(btnStart).Draw();
    const_cast<Button&>(btnSettings).Draw();
    const_cast<Button&>(btnAbout).Draw();
    const_cast<Button&>(btnExit).Draw();

    // Version / hint
    DrawTextEx(fontRegular, "CS163 - Group Project",
               {20, 700}, 12.0f, 1.0f, {100, 120, 160, 180});
    DrawTextEx(fontRegular, "ESC to exit",
               {1180, 700}, 12.0f, 1.0f, {100, 120, 160, 180});
}
