#include "about_screen.h"
#include "font.h"
#include "colors.h"
#include "audio_manager.h"
#include <cmath>

struct Member {
    const char* id;
    const char* name;
    const char* initials;
    Color       avatarColor;
};

static const Member members[] = {
    {"25125053", "Tran Quoc Huy",    "TH", {63,  81, 181, 255}},
    {"25125034", "Tran Quyet Thang", "TT", {0,  172, 193, 255}},
    {"25125059", "Tran Thanh Loi",   "TL", {251,192,  45, 255}},
    {"25125058", "Nguyen Son Lam",   "NL", {229, 57,  53, 255}},
};

AboutScreen::AboutScreen()
    : btnBack({40, 640, 130, 44}, "< Back", Pal::BtnNeutral, Pal::BtnNeutHov) {}

Screen AboutScreen::Update() {
    if (btnBack.Update() || IsKeyPressed(KEY_ESCAPE)) {
        AudioPlayBack();
        return Screen::MainMenu;
    }
    return Screen::About;
}

void AboutScreen::Draw() const {
    ClearBackground({245, 247, 252, 255});

    // Header
    DrawRectangleRec({0, 0, 1280, 80}, Pal::Surface);
    DrawLineEx({0, 80}, {1280, 80}, 1.0f, Pal::Border);
    DrawTextEx(fontBold, "About Us", {50, 22}, 32.0f, 1.0f, Pal::TxtDark);
    btnBack.Draw();

    // Project info
    DrawTextCentered(fontBold,    "Data Structure Visualization", 110.0f, 22.0f, Pal::Indigo);
    DrawTextCentered(fontRegular, "CS163 - Data Structures & Algorithms | Group Project",
                     142.0f, 14.5f, Pal::TxtMid);

    // Thin divider
    DrawLineEx({200, 172}, {1080, 172}, 1.0f, Pal::Border);

    // Member cards: 2 columns, 2 rows
    float cardW  = 420.0f, cardH  = 180.0f;
    float startX = 640.0f - cardW - 25.0f;
    float startY = 200.0f;
    float gapX   = 50.0f, gapY = 30.0f;

    for (int i = 0; i < 4; i++) {
        const Member& m = members[i];
        int col = i % 2, row = i / 2;
        float cx = startX + col * (cardW + gapX);
        float cy = startY + row * (cardH + gapY);
        Rectangle card = {cx, cy, cardW, cardH};

        // Card shadow
        DrawRectangleRounded({cx+3, cy+4, cardW, cardH}, 0.08f, 8, {0,0,0,15});
        DrawRectangleRounded(card, 0.08f, 8, Pal::Surface);
        DrawRectangleRoundedLines(card, 0.08f, 8, Pal::Border);

        // Avatar circle
        float ax = cx + 55, ay = cy + cardH / 2.0f;
        DrawCircleV({ax, ay}, 42.0f, m.avatarColor);
        // Initials
        Vector2 its = MeasureTextEx(fontBold, m.initials, 22.0f, 1.0f);
        DrawTextEx(fontBold, m.initials,
                   {ax - its.x/2, ay - its.y/2}, 22.0f, 1.0f, WHITE);

        // Accent bar on left
        DrawRectangleRounded({cx, cy, 5, cardH}, 0.3f, 4, m.avatarColor);

        // Info
        DrawTextEx(fontBold, m.name,
                   {cx + 115, cy + 52}, 19.0f, 1.0f, Pal::TxtDark);
        DrawTextEx(fontRegular, m.id,
                   {cx + 115, cy + 82}, 15.0f, 1.0f, Pal::TxtMid);
        DrawTextEx(fontRegular, "VNU-HCM, University of Science",
                   {cx + 115, cy + 108}, 12.0f, 1.0f, Pal::TxtLight);
    }

    // Footer
    DrawTextCentered(fontRegular, "Built with Raylib  -  C++17  -  2026",
                     666.0f, 13.0f, Pal::TxtLight);
}
