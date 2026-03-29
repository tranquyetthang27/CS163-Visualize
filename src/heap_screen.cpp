#include "heap_screen.h"
#include "font.h"
#include "colors.h"
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <stdexcept>

static constexpr float TREE_CX    = 640.0f;
static constexpr float TREE_TOP_Y = 130.0f;
static constexpr float LEVEL_H    = 90.0f;
static constexpr float HNODE_R    = 24.0f;

HeapScreen::HeapScreen()
    : input({290, 636, 160, 40}, "Value...", 4),
      btnInsert({20,  636, 120, 40}, "Insert",     Pal::BtnPrimary, Pal::BtnPrimHov),
      btnDelMax ({460, 636, 140, 40}, "Delete Max", Pal::BtnDanger,  Pal::BtnDangHov),
      btnBack   ({20,  20,  100, 36}, "< Back",     Pal::BtnNeutral, Pal::BtnNeutHov),
      msgTimer(0), msgColor(Pal::BtnSuccess),
      animA(-1), animB(-1), animTimer(0), animDuration(0.4f), doingSwap(false)
{}

void HeapScreen::GetNodePos(int i, float& x, float& y) const {
    int level = 0, tmp = i + 1;
    while (tmp > 1) { tmp >>= 1; level++; }
    int nodesAtLevel = 1 << level;
    int posInLevel   = i - (nodesAtLevel - 1);
    float spread     = 1280.0f / (nodesAtLevel + 1);
    x = spread * (posInLevel + 1);
    y = TREE_TOP_Y + level * LEVEL_H;
}

void HeapScreen::ComputePositions() {
    vis.resize(heap.size());
    for (int i = 0; i < (int)heap.size(); i++) {
        float tx, ty;
        GetNodePos(i, tx, ty);
        auto& v = vis[i];
        v.tx = tx; v.ty = ty;
        v.highlighted = (i == animA || i == animB);
        v.swapping    = doingSwap && v.highlighted;
        if (v.alpha < 0.01f) {
            v.x = tx; v.y = ty;   // init
            v.alpha = 0.01f;
        }
    }
}

void HeapScreen::SetMsg(const char* msg, Color c, float dur) {
    message = msg; msgColor = c; msgTimer = dur;
}

Screen HeapScreen::Update() {
    float dt = GetFrameTime();

    // Animate nodes to target positions
    for (auto& v : vis) {
        v.x     += (v.tx - v.x) * 10.0f * dt;
        v.y     += (v.ty - v.y) * 10.0f * dt;
        v.alpha += (1.0f - v.alpha) * 8.0f * dt;
    }

    // Animation countdown
    if (animTimer > 0) {
        animTimer -= dt;
        if (animTimer <= 0) {
            animA = animB = -1;
            doingSwap = false;
            ComputePositions();
        }
    }

    if (msgTimer > 0) msgTimer -= dt;

    if (btnBack.Update() || IsKeyPressed(KEY_ESCAPE)) return Screen::Home;
    input.Update();

    bool doIns    = btnInsert.Update() || (input.focused && IsKeyPressed(KEY_ENTER));
    bool doDelMax = btnDelMax.Update();

    if (doIns && !input.IsEmpty() && animTimer <= 0) {
        int v; try { v = std::stoi(input.text); } catch(...) { SetMsg("Invalid number!", Pal::BtnDanger); return Screen::Heap; }
        if ((int)heap.size() >= MAX_SIZE) {
            SetMsg("Heap full (max 15)!", Pal::BtnDanger);
        } else {
            heap.push_back(v);
            // Heapify up
            int i = (int)heap.size() - 1;
            while (i > 0) {
                int parent = (i - 1) / 2;
                if (heap[i] > heap[parent]) {
                    std::swap(heap[i], heap[parent]);
                    i = parent;
                } else break;
            }
            ComputePositions();
            char buf[32]; snprintf(buf, sizeof(buf), "Inserted %d.", v);
            SetMsg(buf);
            input.Clear();
        }
    } else if (doDelMax && !heap.empty() && animTimer <= 0) {
        int maxVal = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        // Heapify down
        int i = 0, n = (int)heap.size();
        while (true) {
            int largest = i;
            int l = 2*i+1, r = 2*i+2;
            if (l < n && heap[l] > heap[largest]) largest = l;
            if (r < n && heap[r] > heap[largest]) largest = r;
            if (largest == i) break;
            std::swap(heap[i], heap[largest]);
            i = largest;
        }
        ComputePositions();
        char buf[32]; snprintf(buf, sizeof(buf), "Deleted max: %d.", maxVal);
        SetMsg(buf, Pal::BtnDanger);
    }

    return Screen::Heap;
}

void HeapScreen::Draw() const {
    ClearBackground(Pal::BG);

    // Header
    DrawRectangleRec({0, 0, 1280, 72}, Pal::Surface);
    DrawLineEx({0, 72}, {1280, 72}, 1.0f, Pal::Border);
    DrawTextEx(fontBold,    "Max Heap",  {130, 20}, 28.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, "Insert values  |  Delete maximum",
               {130, 52}, 13.5f, 1.0f, Pal::TxtLight);
    btnBack.Draw();

    int n = (int)heap.size();

    if (n == 0) {
        DrawTextCentered(fontRegular, "Heap is empty — insert a value below",
                         360, 17.0f, Pal::TxtLight);
    }

    // Draw tree edges
    for (int i = 0; i < n; i++) {
        int l = 2*i+1, r = 2*i+2;
        const auto& vi = vis[i];
        if (l < n) DrawLineEx({vi.x, vi.y}, {vis[l].x, vis[l].y}, 1.5f, Pal::EdgeColor);
        if (r < n) DrawLineEx({vi.x, vi.y}, {vis[r].x, vis[r].y}, 1.5f, Pal::EdgeColor);
    }

    // Draw tree nodes
    for (int i = 0; i < n; i++) {
        const auto& v = vis[i];
        Color fillC = v.highlighted ? Pal::NodeHL : Pal::NodeFill;
        Color bordC = v.highlighted ? Color{200,160,0,255} : Pal::NodeBorder;
        Color textC = Pal::TxtDark;
        if (i == 0 && n > 0) { fillC = Pal::Indigo; bordC = Pal::IndigoDark; textC = WHITE; }

        unsigned char a = (unsigned char)(v.alpha * 255);
        fillC.a = bordC.a = textC.a = a;

        DrawCircleV({v.x, v.y}, HNODE_R + 2,  bordC);
        DrawCircleV({v.x, v.y}, HNODE_R,       fillC);

        char buf[8]; snprintf(buf, sizeof(buf), "%d", heap[i]);
        Vector2 ts = MeasureTextEx(fontBold, buf, 16.0f, 1.0f);
        DrawTextEx(fontBold, buf,
            {v.x - ts.x/2, v.y - ts.y/2}, 16.0f, 1.0f, textC);
    }

    // Array view (below tree)
    float arrayY = 540.0f;
    DrawTextEx(fontRegular, "Array:", {50, arrayY}, 14.0f, 1.0f, Pal::TxtMid);
    float cellW = 48.0f, cellH = 36.0f;
    float arrStartX = 120.0f;
    for (int i = 0; i < n; i++) {
        Rectangle cell = {arrStartX + i * cellW, arrayY - 4, cellW - 2, cellH};
        Color bg = (i == 0) ? Pal::Indigo : Pal::PanelDark;
        Color tc = (i == 0) ? WHITE : Pal::TxtDark;
        DrawRectangleRounded(cell, 0.2f, 6, bg);
        char buf[8]; snprintf(buf, sizeof(buf), "%d", heap[i]);
        DrawTextInRect(fontBold, buf, cell, 14.0f, tc);
        // index below
        char idx[4]; snprintf(idx, sizeof(idx), "%d", i);
        Vector2 ts = MeasureTextEx(fontRegular, idx, 11.0f, 1.0f);
        DrawTextEx(fontRegular, idx,
            {cell.x + cell.width/2 - ts.x/2, arrayY + cellH - 2}, 11.0f, 1.0f, Pal::TxtLight);
    }

    // Bottom panel
    DrawRectangleRec({0, 616, 1280, 104}, Pal::Panel);
    DrawLineEx({0, 616}, {1280, 616}, 1.0f, Pal::Border);
    btnInsert.Draw();
    input.Draw();
    btnDelMax.Draw();

    // Message
    if (msgTimer > 0 && !message.empty()) {
        float alpha = msgTimer < 0.5f ? msgTimer / 0.5f : 1.0f;
        Color c = msgColor; c.a = (unsigned char)(alpha * 220);
        DrawTextEx(fontRegular, message.c_str(), {620, 646}, 16.0f, 1.0f, c);
    }

    char cnt[32]; snprintf(cnt, sizeof(cnt), "Size: %d / %d", n, MAX_SIZE);
    DrawTextEx(fontRegular, cnt, {1150, 646}, 14.0f, 1.0f, Pal::TxtLight);
}
