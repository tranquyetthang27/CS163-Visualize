#include "linked_list_screen.h"
#include "font.h"
#include "colors.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <stdexcept>

static constexpr float NODE_W  = 64.0f;
static constexpr float NODE_H  = 44.0f;
static constexpr float NODE_GAP = 50.0f;
static constexpr float NODE_Y   = 340.0f;
static constexpr int   MAX_NODES = 12;

LinkedListScreen::LinkedListScreen()
    : input({430, 630, 180, 40}, "Enter value...", 6),
      btnInsHead({20,  630, 120, 40}, "Insert Head", Pal::BtnPrimary, Pal::BtnPrimHov),
      btnInsTail({150, 630, 120, 40}, "Insert Tail", Pal::BtnSuccess, Pal::BtnSuccHov),
      btnDel    ({620, 630, 100, 40}, "Delete",      Pal::BtnDanger,  Pal::BtnDangHov),
      btnSearch ({730, 630, 100, 40}, "Search",      Pal::BtnOrange,  Pal::BtnOrangeHov),
      btnBack   ({20,  20,  100, 36}, "< Back",      Pal::BtnNeutral, Pal::BtnNeutHov),
      msgTimer(0.0f), msgColor(Pal::BtnSuccess) {}

void LinkedListScreen::LayoutNodes() {
    int n = (int)nodes.size();
    float totalW = n * NODE_W + (n > 0 ? (n - 1) * NODE_GAP : 0);
    float startX = 640.0f - totalW / 2.0f;
    for (int i = 0; i < n; i++) {
        nodes[i].tx = startX + i * (NODE_W + NODE_GAP);
        nodes[i].ty = NODE_Y;
    }
}

void LinkedListScreen::SetMsg(const char* msg, Color c, float dur) {
    message  = msg;
    msgColor = c;
    msgTimer = dur;
}

Screen LinkedListScreen::Update() {
    float dt = GetFrameTime();

    // Animate node positions
    for (auto& nd : nodes) {
        nd.x     += (nd.tx - nd.x) * 8.0f * dt;
        nd.y     += (nd.ty - nd.y) * 8.0f * dt;
        nd.alpha += (1.0f - nd.alpha) * 8.0f * dt;
    }
    if (msgTimer > 0) msgTimer -= dt;

    // Back
    if (btnBack.Update() || IsKeyPressed(KEY_ESCAPE)) return Screen::Home;

    input.Update();
    bool insHead = btnInsHead.Update();
    bool insTail = btnInsTail.Update();
    bool doDel   = btnDel.Update();
    bool doSearch = btnSearch.Update();

    auto parseVal = [&](int& out) -> bool {
        if (input.IsEmpty()) { SetMsg("Enter a value!", {229,57,53,255}); return false; }
        try { out = std::stoi(input.text); }
        catch (...) { SetMsg("Invalid number!", {229,57,53,255}); return false; }
        return true;
    };

    if (insHead || (IsKeyPressed(KEY_ENTER) && insHead)) {
        int v; if (parseVal(v)) {
            if ((int)nodes.size() >= MAX_NODES) {
                SetMsg("List is full (max 12)!", {229,57,53,255});
            } else {
                LLNode nd;
                nd.value = v;
                nd.x = nd.tx = -100; nd.y = nd.ty = NODE_Y;
                nd.alpha = 0; nd.state = LLState::Normal;
                nodes.insert(nodes.begin(), nd);
                LayoutNodes();
                SetMsg("Inserted at head.");
                input.Clear();
            }
        }
    } else if (insTail) {
        int v; if (parseVal(v)) {
            if ((int)nodes.size() >= MAX_NODES) {
                SetMsg("List is full (max 12)!", {229,57,53,255});
            } else {
                LLNode nd;
                nd.value = v;
                nd.x = nd.tx = 1400; nd.y = nd.ty = NODE_Y;
                nd.alpha = 0; nd.state = LLState::Normal;
                nodes.push_back(nd);
                LayoutNodes();
                SetMsg("Inserted at tail.");
                input.Clear();
            }
        }
    } else if (doDel) {
        int v; if (parseVal(v)) {
            bool found = false;
            for (auto it = nodes.begin(); it != nodes.end(); ++it) {
                if (it->value == v) {
                    nodes.erase(it);
                    LayoutNodes();
                    SetMsg("Deleted node.");
                    input.Clear();
                    found = true;
                    break;
                }
            }
            if (!found) SetMsg("Value not found!", {229,57,53,255});
        }
    } else if (doSearch) {
        int v; if (parseVal(v)) {
            bool found = false;
            for (auto& nd : nodes) nd.state = LLState::Normal;
            for (auto& nd : nodes) {
                if (nd.value == v) {
                    nd.state = LLState::Found;
                    found = true;
                }
            }
            SetMsg(found ? "Found!" : "Not found!", found ? Pal::BtnSuccess : Pal::BtnDanger);
        }
    }

    return Screen::LinkedList;
}

void LinkedListScreen::Draw() const {
    ClearBackground(Pal::BG);

    // Header bar
    DrawRectangleRec({0, 0, 1280, 72}, Pal::Surface);
    DrawLineEx({0, 72}, {1280, 72}, 1.0f, Pal::Border);
    DrawTextEx(fontBold, "Linked List",
               {130, 20}, 28.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, "Insert Head / Tail  |  Delete  |  Search",
               {130, 52}, 13.5f, 1.0f, Pal::TxtLight);

    btnBack.Draw();

    // Empty state
    if (nodes.empty()) {
        DrawTextCentered(fontRegular, "List is empty — insert a value below",
                         NODE_Y + 5, 17.0f, Pal::TxtLight);
    }

    // Draw arrows between nodes first (behind nodes)
    int n = (int)nodes.size();
    for (int i = 0; i < n - 1; i++) {
        float ax = nodes[i].x + NODE_W;
        float ay = nodes[i].y + NODE_H / 2.0f;
        float bx = nodes[i + 1].x;
        DrawLineEx({ax, ay}, {bx - 6, ay}, 2.0f, Pal::EdgeColor);
        DrawTriangle({bx, ay}, {bx - 8, ay - 5}, {bx - 8, ay + 5}, Pal::EdgeColor);
    }

    // Draw nodes
    for (const auto& nd : nodes) {
        Color fillC  = Pal::NodeFill;
        Color bordC  = Pal::NodeBorder;
        Color textC  = Pal::TxtDark;

        if (nd.state == LLState::Found) {
            fillC = Pal::NodeFound;
            bordC = {36, 128, 54, 255};
            textC = WHITE;
        } else if (nd.state == LLState::Highlighted) {
            fillC = Pal::NodeHL;
            bordC = {200, 160, 0, 255};
        }

        unsigned char a = (unsigned char)(nd.alpha * 255);
        fillC.a = a; bordC.a = a; textC.a = a;

        Rectangle r = {nd.x, nd.y, NODE_W, NODE_H};
        // Border
        Rectangle bo = {r.x - 2, r.y - 2, r.width + 4, r.height + 4};
        DrawRectangleRounded(bo, 0.25f, 8, bordC);
        DrawRectangleRounded(r,  0.25f, 8, fillC);

        // Value text
        char buf[16]; snprintf(buf, sizeof(buf), "%d", nd.value);
        DrawTextInRect(fontBold, buf, r, 18.0f, textC);
    }

    // Bottom control panel
    DrawRectangleRec({0, 610, 1280, 110}, Pal::Panel);
    DrawLineEx({0, 610}, {1280, 610}, 1.0f, Pal::Border);

    btnInsHead.Draw();
    btnInsTail.Draw();
    input.Draw();
    btnDel.Draw();
    btnSearch.Draw();

    // Message
    if (msgTimer > 0 && !message.empty()) {
        float alpha = msgTimer < 0.5f ? msgTimer / 0.5f : 1.0f;
        Color c = msgColor;
        c.a = (unsigned char)(alpha * 220);
        DrawTextEx(fontRegular, message.c_str(), {845, 640}, 16.0f, 1.0f, c);
    }

    // Node count
    char cnt[32]; snprintf(cnt, sizeof(cnt), "Nodes: %d / %d", (int)nodes.size(), MAX_NODES);
    DrawTextEx(fontRegular, cnt, {1150, 640}, 14.0f, 1.0f, Pal::TxtLight);
}
