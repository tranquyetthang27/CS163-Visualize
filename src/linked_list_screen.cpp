#include "linked_list_screen.h"
#include "font.h"
#include "colors.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <stdexcept>

static constexpr float NODE_R    = 30.0f;
static constexpr float NODE_GAP  = 44.0f;
static constexpr float NODE_Y    = 360.0f;
static constexpr int   MAX_NODES = 12;

LinkedListScreen::LinkedListScreen()
    : input({430, 630, 180, 40}, "Enter value...", 6),
      btnInsert ({20,  630, 120, 40}, "Insert",      Pal::BtnPrimary, Pal::BtnPrimHov),
      btnInsHead({20,  565,  57, 38}, "Head",        Pal::BtnPrimary, Pal::BtnPrimHov),
      btnInsTail({82,  565,  57, 38}, "Tail",        Pal::BtnSuccess, Pal::BtnSuccHov),
      btnInsIdx ({144, 565,  57, 38}, "Index",       Pal::Teal,       Pal::TealDark),
      btnDel    ({620, 630, 100, 40}, "Delete",      Pal::BtnDanger,  Pal::BtnDangHov),
      btnDelHead({620, 565,  47, 38}, "Head",        Pal::BtnDanger,  Pal::BtnDangHov),
      btnDelTail({672, 565,  47, 38}, "Tail",        Pal::BtnOrange,  Pal::BtnOrangeHov),
      btnDelIdx ({724, 565,  57, 38}, "Index",       Pal::Teal,       Pal::TealDark),
      btnSearch ({730, 630, 100, 40}, "Search",      Pal::BtnOrange,  Pal::BtnOrangeHov),
      btnUpdate ({845, 630, 100, 40}, "Update",      Pal::Teal,       Pal::TealDark),
      btnBack   ({20,  20,  100, 36}, "< Back",      Pal::BtnNeutral, Pal::BtnNeutHov),
      insertMenuOpen(false), deleteMenuOpen(false),
      msgTimer(0.0f), msgColor(Pal::BtnSuccess) {}

void LinkedListScreen::LayoutNodes() {
    int n = (int)nodes.size();
    float totalW = n * 2 * NODE_R + (n > 0 ? (n - 1) * NODE_GAP : 0);
    float startX = 640.0f - totalW / 2.0f + NODE_R;
    for (int i = 0; i < n; i++) {
        nodes[i].tx = startX + i * (2 * NODE_R + NODE_GAP);
        nodes[i].ty = NODE_Y;
    }
}

void LinkedListScreen::SetMsg(const char* msg, Color c, float dur) {
    message  = msg;
    msgColor = c;
    msgTimer = dur;
}

// ── Insert step-by-step ──────────────────────────────────────────

void LinkedListScreen::StartInsertStep(int idx, int v) {
    // Insert node immediately; steps control highlight & arrow visibility
    LLNode nd;
    nd.value = v;
    nd.alpha = 0.0f;
    nd.state = LLState::Normal;
    // spawn from above
    nd.x  = (nodes.empty() ? 640.0f : (idx == 0 ? nodes[0].tx : nodes[idx > 0 ? idx-1 : 0].tx));
    nd.tx = nd.x;
    nd.y  = NODE_Y - 80.0f;
    nd.ty = NODE_Y;
    nodes.insert(nodes.begin() + idx, nd);
    LayoutNodes();

    for (auto& n : nodes) n.state = LLState::Normal;
    stepActive  = true;
    stepPhase   = 0;
    stepTimer   = 0.7f;
    stepNewIdx  = idx;
    input.Clear();
}

void LinkedListScreen::AdvanceStep() {
    stepPhase++;

    // Skip phase 1 for head insert (no predecessor)
    if (stepPhase == 1 && stepNewIdx == 0)
        stepPhase = 2;

    for (auto& n : nodes) n.state = LLState::Normal;

    switch (stepPhase) {
        case 1:
            nodes[stepNewIdx - 1].state = LLState::Highlighted;
            stepTimer = 0.6f;
            break;
        case 2:
            // arrows appear (Draw checks stepPhase >= 2)
            stepTimer = 0.6f;
            break;
        case 3:
            nodes[stepNewIdx].state = LLState::Highlighted;
            stepTimer = 0.6f;
            break;
        default:
            nodes[stepNewIdx].state = LLState::Normal;
            stepActive = false;
            char buf[64];
            snprintf(buf, sizeof(buf), "Inserted %d at index %d.", nodes[stepNewIdx].value, stepNewIdx);
            SetMsg(buf, Pal::BtnSuccess, 3.0f);
            break;
    }
}

// ── Update ───────────────────────────────────────────────────────

Screen LinkedListScreen::Update() {
    float dt = GetFrameTime();

    for (auto& nd : nodes) {
        nd.x     += (nd.tx - nd.x) * 8.0f * dt;
        nd.y     += (nd.ty - nd.y) * 8.0f * dt;
        nd.alpha += (1.0f - nd.alpha) * 8.0f * dt;
    }
    if (msgTimer > 0) msgTimer -= dt;

    if (btnBack.Update() || (IsKeyPressed(KEY_ESCAPE) && !stepActive))
        return Screen::Home;

    // ── Step mode (auto-play) ─────────────────────────────────────
    if (stepActive) {
        stepTimer -= dt;
        if (stepTimer <= 0.0f)
            AdvanceStep();
        return Screen::LinkedList;
    }

    // ── Normal mode ──────────────────────────────────────────────
    input.Update();
    bool openInsert = btnInsert.Update();
    if (openInsert) { insertMenuOpen = !insertMenuOpen; deleteMenuOpen = false; }

    bool insHead = false, insTail = false, insIdx = false;
    if (insertMenuOpen) {
        insHead = btnInsHead.Update();
        insTail = btnInsTail.Update();
        insIdx  = btnInsIdx.Update();
        if (insHead || insTail || insIdx) insertMenuOpen = false;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !openInsert && !insHead && !insTail && !insIdx)
            insertMenuOpen = false;
    }

    bool openDel = btnDel.Update();
    if (openDel) { deleteMenuOpen = !deleteMenuOpen; insertMenuOpen = false; }

    bool delHead = false, delTail = false, delIdx = false;
    if (deleteMenuOpen) {
        delHead = btnDelHead.Update();
        delTail = btnDelTail.Update();
        delIdx  = btnDelIdx.Update();
        if (delHead || delTail || delIdx) deleteMenuOpen = false;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !openDel && !delHead && !delTail && !delIdx)
            deleteMenuOpen = false;
    }

    bool doSearch = btnSearch.Update();
    bool doUpdate = btnUpdate.Update();

    auto parseVal = [&](int& out) -> bool {
        if (input.IsEmpty()) { SetMsg("Enter a value!", {229,57,53,255}); return false; }
        try { out = std::stoi(input.text); }
        catch (...) { SetMsg("Invalid number!", {229,57,53,255}); return false; }
        return true;
    };

    if (insHead) {
        int v; if (parseVal(v)) {
            if ((int)nodes.size() >= MAX_NODES) SetMsg("List is full (max 12)!", {229,57,53,255});
            else StartInsertStep(0, v);
        }
    } else if (insTail) {
        int v; if (parseVal(v)) {
            if ((int)nodes.size() >= MAX_NODES) SetMsg("List is full (max 12)!", {229,57,53,255});
            else StartInsertStep((int)nodes.size(), v);
        }
    } else if (insIdx) {
        if (input.IsEmpty()) {
            SetMsg("Format: 'index value'  e.g. '1 50'", {229,57,53,255});
        } else {
            int idx, v;
            if (sscanf(input.text.c_str(), "%d %d", &idx, &v) != 2) {
                SetMsg("Format: 'index value'  e.g. '1 50'", {229,57,53,255});
            } else if ((int)nodes.size() >= MAX_NODES) {
                SetMsg("List is full (max 12)!", {229,57,53,255});
            } else if (idx < 0 || idx > (int)nodes.size()) {
                char buf[64]; snprintf(buf, sizeof(buf), "Index out of range (0-%d)!", (int)nodes.size());
                SetMsg(buf, {229,57,53,255});
            } else {
                StartInsertStep(idx, v);
            }
        }
    } else if (delHead) {
        if (nodes.empty()) SetMsg("List is empty!", {229,57,53,255});
        else { nodes.erase(nodes.begin()); LayoutNodes(); SetMsg("Deleted head node.", Pal::BtnDanger); }
    } else if (delTail) {
        if (nodes.empty()) SetMsg("List is empty!", {229,57,53,255});
        else { nodes.pop_back(); LayoutNodes(); SetMsg("Deleted tail node.", Pal::BtnDanger); }
    } else if (delIdx) {
        if (input.IsEmpty()) { SetMsg("Enter index to delete!", {229,57,53,255}); }
        else {
            int idx;
            if (sscanf(input.text.c_str(), "%d", &idx) != 1) {
                SetMsg("Invalid index!", {229,57,53,255});
            } else if (nodes.empty()) {
                SetMsg("List is empty!", {229,57,53,255});
            } else if (idx < 0 || idx >= (int)nodes.size()) {
                char buf[64]; snprintf(buf, sizeof(buf), "Index out of range (0-%d)!", (int)nodes.size()-1);
                SetMsg(buf, {229,57,53,255});
            } else {
                char buf[64]; snprintf(buf, sizeof(buf), "Deleted node at index %d (value=%d).", idx, nodes[idx].value);
                nodes.erase(nodes.begin() + idx);
                LayoutNodes();
                SetMsg(buf, Pal::BtnDanger);
                input.Clear();
            }
        }
    } else if (doSearch) {
        int v; if (parseVal(v)) {
            bool found = false;
            for (auto& nd : nodes) nd.state = LLState::Normal;
            for (auto& nd : nodes) {
                if (nd.value == v) { nd.state = LLState::Found; found = true; }
            }
            SetMsg(found ? "Found!" : "Not found!", found ? Pal::BtnSuccess : Pal::BtnDanger);
        }
    } else if (doUpdate) {
        if (input.IsEmpty()) { SetMsg("Format: 'index value'  e.g. '2 50'", {229,57,53,255}); }
        else {
            int idx, newVal;
            if (sscanf(input.text.c_str(), "%d %d", &idx, &newVal) != 2) {
                SetMsg("Format: 'index value'  e.g. '2 50'", {229,57,53,255});
            } else if (nodes.empty()) {
                SetMsg("List is empty!", {229,57,53,255});
            } else if (idx < 0 || idx >= (int)nodes.size()) {
                char buf[64]; snprintf(buf, sizeof(buf), "Index out of range (0-%d)!", (int)nodes.size()-1);
                SetMsg(buf, {229,57,53,255});
            } else {
                for (auto& nd : nodes) nd.state = LLState::Normal;
                nodes[idx].value = newVal;
                nodes[idx].state = LLState::Highlighted;
                char buf[64]; snprintf(buf, sizeof(buf), "Updated index %d to %d.", idx, newVal);
                SetMsg(buf, Pal::Teal);
                input.Clear();
            }
        }
    }

    return Screen::LinkedList;
}

// ── Draw ─────────────────────────────────────────────────────────

void LinkedListScreen::Draw() const {
    ClearBackground(Pal::BG);

    DrawRectangleRec({0, 0, 1280, 72}, Pal::Surface);
    DrawLineEx({0, 72}, {1280, 72}, 1.0f, Pal::Border);
    DrawTextEx(fontBold, "Linked List", {130, 20}, 28.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular,
               stepActive ? "Step-by-step insert  |  Press  Next >  to advance  |  Stop to cancel"
                          : "Insert  |  Delete  |  Search  |  Update: 'index value' (e.g. '2 50')",
               {130, 52}, 13.5f, 1.0f, stepActive ? Pal::BtnOrange : Pal::TxtLight);
    btnBack.Draw();

    if (nodes.empty()) {
        DrawTextCentered(fontRegular, "List is empty — insert a value below",
                         NODE_Y + 5, 17.0f, Pal::TxtLight);
    }

    int n = (int)nodes.size();

    // Arrows — hide arrows to/from new node until phase 2
    for (int i = 0; i < n - 1; i++) {
        if (stepActive && stepPhase < 2) {
            if (i == stepNewIdx || i + 1 == stepNewIdx) continue;
        }
        float ax = nodes[i].x + NODE_R;
        float ay = nodes[i].y;
        float bx = nodes[i+1].x - NODE_R;
        DrawLineEx({ax, ay}, {bx - 6, ay}, 2.0f, BLACK);
        DrawTriangle({bx, ay}, {bx-8, ay-5}, {bx-8, ay+5}, BLACK);
    }

    // Nodes
    for (const auto& nd : nodes) {
        Color fillC = Pal::NodeFill, bordC = Pal::NodeBorder, textC = Pal::TxtDark;
        if (nd.state == LLState::Found) {
            fillC = Pal::NodeFound; bordC = {36,128,54,255}; textC = WHITE;
        } else if (nd.state == LLState::Highlighted) {
            fillC = Pal::NodeHL; bordC = {200,160,0,255};
        } else if (nd.state == LLState::Removing) {
            fillC = {255,200,200,255}; bordC = Pal::BtnDanger;
        }
        unsigned char a = (unsigned char)(nd.alpha * 255);
        fillC.a = a; bordC.a = a; textC.a = a;

        DrawCircleV({nd.x, nd.y}, NODE_R + 2, bordC);
        DrawCircleV({nd.x, nd.y}, NODE_R,     fillC);
        Rectangle r = {nd.x - NODE_R, nd.y - NODE_R, 2*NODE_R, 2*NODE_R};
        char buf[16]; snprintf(buf, sizeof(buf), "%d", nd.value);
        DrawTextInRect(fontBold, buf, r, 18.0f, textC);
    }

    // Bottom panel
    DrawRectangleRec({0, 610, 1280, 110}, Pal::Panel);
    DrawLineEx({0, 610}, {1280, 610}, 1.0f, Pal::Border);

    if (!stepActive) {
        btnInsert.Draw();
        if (insertMenuOpen) {
            DrawRectangleRounded({15, 558, 191, 52}, 0.2f, 8, Pal::Surface);
            DrawRectangleRoundedLines({15, 558, 191, 52}, 0.2f, 8, Pal::Border);
            btnInsHead.Draw();
            btnInsTail.Draw();
            btnInsIdx.Draw();
        }
        input.Draw();
        btnDel.Draw();
        if (deleteMenuOpen) {
            DrawRectangleRounded({615, 558, 171, 52}, 0.2f, 8, Pal::Surface);
            DrawRectangleRoundedLines({615, 558, 171, 52}, 0.2f, 8, Pal::Border);
            btnDelHead.Draw();
            btnDelTail.Draw();
            btnDelIdx.Draw();
        }
        btnSearch.Draw();
        btnUpdate.Draw();

        if (msgTimer > 0 && !message.empty()) {
            float alpha = msgTimer < 0.5f ? msgTimer / 0.5f : 1.0f;
            Color c = msgColor; c.a = (unsigned char)(alpha * 220);
            DrawTextEx(fontRegular, message.c_str(), {960, 640}, 16.0f, 1.0f, c);
        }

        char cnt[32]; snprintf(cnt, sizeof(cnt), "Nodes: %d / %d", (int)nodes.size(), MAX_NODES);
        DrawTextEx(fontRegular, cnt, {1150, 640}, 14.0f, 1.0f, Pal::TxtLight);
    }
}
