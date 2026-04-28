#include "home_screen.h"
#include "font.h"
#include "colors.h"
#include "app_settings.h"
#include "audio_manager.h"

HomeScreen::HomeScreen()
    : btnBack({30, 20, 110, 38}, "< Back", Pal::BtnNeutral, Pal::BtnNeutHov, WHITE, 15.0f)
{
    const int W = 1280, H = 720;
    float padX  = 70.0f;
    float topY  = 148.0f;
    float gap   = 22.0f;
    float cardW = (W - padX * 2 - gap) / 2.0f;
    float cardH = (H - topY - 30.0f - gap) / 2.0f;

    cards[0] = Card({padX,              topY,              cardW, cardH},
                    Screen::LinkedList, "Linked List",
                    "Node and pointer operations", Pal::Indigo);
    cards[1] = Card({padX + cardW + gap, topY,             cardW, cardH},
                    Screen::Trie, "Trie",
                    "Prefix tree visualization", Pal::Teal);
    cards[2] = Card({padX,              topY + cardH + gap, cardW, cardH},
                    Screen::Heap, "Heap",
                    "Binary heap relationships", Pal::Amber);
    cards[3] = Card({padX + cardW + gap, topY + cardH + gap, cardW, cardH},
                    Screen::MST, "Graph",
                    "Kruskal's and Prim's algorithm step-by-step", Pal::Coral);
}

Screen HomeScreen::Update() {
    if (btnBack.Update() || IsKeyPressed(KEY_ESCAPE)) {
        AudioPlayBack();
        return Screen::MainMenu;
    }
    for (auto& c : cards)
        if (c.Update()) { AudioPlayClick(); return c.target; }
    return Screen::Home;
}

void HomeScreen::Draw() const {
    ClearBackground(gSettings.GetBG());

    DrawTextCentered(fontBold,    "Data Structure",
                     42.0f, 38.0f, Pal::TxtDark);
    DrawTextCentered(fontRegular, "Visualising data structures and algorithms through animation",
                     90.0f, 17.0f, Pal::TxtMid);

    btnBack.Draw();
    for (const auto& c : cards) c.Draw();
}
