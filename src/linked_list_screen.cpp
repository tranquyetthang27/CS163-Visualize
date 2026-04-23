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
      msgTimer(0.0f), msgColor(Pal::BtnSuccess),
      btnNext({252, 630, 110, 40}, "Next >",  Pal::BtnSuccess, Pal::BtnSuccHov),
      btnPrev({132, 630, 110, 40}, "< Prev",  Pal::BtnNeutral, Pal::BtnNeutHov),
      btnStop({20,  630, 100, 40}, "Stop",    Pal::BtnDanger,  Pal::BtnDangHov) {}

// ── Helpers ──────────────────────────────────────────────────────

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

// ── Step-by-step ─────────────────────────────────────────────────

void LinkedListScreen::ApplyFrame(int i) {
    const StepFrame& f = stepFrames[i];
    for (int j = 0; j < (int)nodes.size() && j < (int)f.nodeStates.size(); j++)
        nodes[j].state = f.nodeStates[j];
    hasPendingNode = f.showPending;
    SetMsg(f.desc.c_str(), {63, 81, 181, 255}, 120.0f);
}

void LinkedListScreen::ExitStepMode(bool execute) {
    if (execute && stepFinalAction)   stepFinalAction();
    else if (!execute && stepCancelAction) stepCancelAction();
    stepMode  = false;
    stepIdx   = 0;
    stepFrames.clear();
    stepFinalAction  = nullptr;
    stepCancelAction = nullptr;
    hasPendingNode   = false;
}

void LinkedListScreen::PrepareInsertSteps(int insertIdx, int v) {
    int n = (int)nodes.size();
    stepFrames.clear();

    // ── Position the pending node above the insertion point ──────
    pendingNode.value = v;
    pendingNode.state = LLState::Highlighted;
    pendingNode.alpha = 0.0f;
    pendingInsertIdx  = insertIdx;

    float pendX;
    if (n == 0) {
        pendX = 640.0f;
    } else if (insertIdx == 0) {
        pendX = nodes[0].tx - (2 * NODE_R + NODE_GAP);
        if (pendX < NODE_R + 8) pendX = nodes[0].tx; // clamp
    } else if (insertIdx >= n) {
        pendX = nodes[n-1].tx + (2 * NODE_R + NODE_GAP);
    } else {
        pendX = (nodes[insertIdx-1].tx + nodes[insertIdx].tx) * 0.5f;
    }
    pendingNode.x = pendingNode.tx = pendX;
    pendingNode.y = pendingNode.ty = NODE_Y - 90.0f;

    auto allNormal = [&]() { return std::vector<LLState>(n, LLState::Normal); };

    // ── Build frames ─────────────────────────────────────────────
    if (insertIdx == 0) {
        // Insert at Head
        {
            StepFrame f;
            f.desc = "Allocate new node [" + std::to_string(v) + "]";
            f.nodeStates  = allNormal();
            f.showPending = true;
            stepFrames.push_back(f);
        }
        if (n > 0) {
            StepFrame f;
            f.desc = "Set new_node.next = head [" + std::to_string(nodes[0].value) + "]";
            f.nodeStates       = allNormal();
            f.nodeStates[0]    = LLState::Highlighted;
            f.showPending      = true;
            stepFrames.push_back(f);
        }
        {
            StepFrame f;
            f.desc        = "Update head = new_node  →  Insert complete!";
            f.nodeStates  = allNormal();
            f.showPending = true;
            f.isFinal     = true;
            stepFrames.push_back(f);
        }
    } else if (insertIdx >= n) {
        // Insert at Tail  —  traverse first
        for (int i = 0; i < n; i++) {
            StepFrame f;
            if (i == 0)
                f.desc = "Start traversal at head (index 0)  [val=" + std::to_string(nodes[0].value) + "]";
            else
                f.desc = "node[" + std::to_string(i-1) + "].next != null  →  advance to index " + std::to_string(i) +
                         "  [val=" + std::to_string(nodes[i].value) + "]";
            f.nodeStates    = allNormal();
            f.nodeStates[i] = LLState::Highlighted;
            stepFrames.push_back(f);
        }
        {
            StepFrame f;
            f.desc = "Reached last node  →  allocate new node [" + std::to_string(v) + "]";
            f.nodeStates       = allNormal();
            if (n > 0) f.nodeStates[n-1] = LLState::Highlighted;
            f.showPending      = true;
            stepFrames.push_back(f);
        }
        {
            StepFrame f;
            f.desc        = (n > 0 ? "node[" + std::to_string(n-1) + "].next = new_node" : "head = new_node") +
                             "  →  Insert complete!";
            f.nodeStates  = allNormal();
            if (n > 0) f.nodeStates[n-1] = LLState::Highlighted;
            f.showPending = true;
            f.isFinal     = true;
            stepFrames.push_back(f);
        }
    } else {
        // Insert in middle at insertIdx
        for (int i = 0; i < insertIdx; i++) {
            StepFrame f;
            if (i == 0)
                f.desc = "Start traversal at head (index 0)  [val=" + std::to_string(nodes[0].value) + "]";
            else
                f.desc = "node[" + std::to_string(i-1) + "].next != null  →  advance to index " + std::to_string(i) +
                         "  [val=" + std::to_string(nodes[i].value) + "]";
            f.nodeStates    = allNormal();
            f.nodeStates[i] = LLState::Highlighted;
            stepFrames.push_back(f);
        }
        {
            StepFrame f;
            f.desc = "Predecessor at index " + std::to_string(insertIdx-1) +
                     "  →  allocate new node [" + std::to_string(v) + "]";
            f.nodeStates                  = allNormal();
            f.nodeStates[insertIdx-1]     = LLState::Highlighted;
            f.showPending                 = true;
            stepFrames.push_back(f);
        }
        {
            StepFrame f;
            f.desc = "new_node.next = node[" + std::to_string(insertIdx) + "];  "
                     "node[" + std::to_string(insertIdx-1) + "].next = new_node  →  Insert complete!";
            f.nodeStates              = allNormal();
            f.nodeStates[insertIdx-1] = LLState::Highlighted;
            f.nodeStates[insertIdx]   = LLState::Highlighted;
            f.showPending             = true;
            f.isFinal                 = true;
            stepFrames.push_back(f);
        }
    }

    // ── Actions ──────────────────────────────────────────────────
    stepFinalAction = [this, insertIdx, v]() {
        LLNode nd;
        nd.value = v;
        // spawn from where pending node was shown
        nd.x     = hasPendingNode ? pendingNode.x : (insertIdx == 0 ? -100.0f : 1400.0f);
        nd.y     = hasPendingNode ? pendingNode.y : NODE_Y;
        nd.tx    = nd.x;
        nd.ty    = NODE_Y;
        nd.alpha = hasPendingNode ? pendingNode.alpha : 0.0f;
        nd.state = LLState::Highlighted;
        nodes.insert(nodes.begin() + insertIdx, nd);
        LayoutNodes();
        char buf[64];
        snprintf(buf, sizeof(buf), "Inserted %d at index %d.", v, insertIdx);
        SetMsg(buf, Pal::BtnSuccess, 3.0f);
        input.Clear();
    };

    stepCancelAction = [this]() {
        for (auto& nd : nodes) nd.state = LLState::Normal;
        SetMsg("Insert cancelled.", Pal::TxtMid, 2.0f);
    };

    // Enter step mode
    stepMode = true;
    stepIdx  = 0;
    ApplyFrame(0);
}

// ── Update ───────────────────────────────────────────────────────

Screen LinkedListScreen::Update() {
    float dt = GetFrameTime();

    // Animate existing nodes
    for (auto& nd : nodes) {
        nd.x     += (nd.tx - nd.x) * 8.0f * dt;
        nd.y     += (nd.ty - nd.y) * 8.0f * dt;
        nd.alpha += (1.0f - nd.alpha) * 8.0f * dt;
    }
    // Animate pending node
    if (hasPendingNode) {
        pendingNode.alpha += (1.0f - pendingNode.alpha) * 8.0f * dt;
        pendingNode.x     += (pendingNode.tx - pendingNode.x) * 8.0f * dt;
        pendingNode.y     += (pendingNode.ty - pendingNode.y) * 8.0f * dt;
    }
    if (msgTimer > 0) msgTimer -= dt;

    // Back always works
    if (btnBack.Update() || (IsKeyPressed(KEY_ESCAPE) && !stepMode))
        return Screen::Home;

    // ── Step mode: only step controls active ─────────────────────
    if (stepMode) {
        if (btnStop.Update()) {
            ExitStepMode(false);
        } else if (btnNext.Update() || IsKeyPressed(KEY_RIGHT)) {
            if (stepFrames[stepIdx].isFinal) {
                ExitStepMode(true);
            } else {
                stepIdx = stepIdx + 1 < (int)stepFrames.size() ? stepIdx + 1 : stepIdx;
                ApplyFrame(stepIdx);
            }
        } else if (btnPrev.Update() || IsKeyPressed(KEY_LEFT)) {
            if (stepIdx > 0) {
                stepIdx--;
                ApplyFrame(stepIdx);
            }
        }
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

    // ── Insert (step-by-step) ─────────────────────────────────────
    if (insHead) {
        int v; if (parseVal(v)) {
            if ((int)nodes.size() >= MAX_NODES) SetMsg("List is full (max 12)!", {229,57,53,255});
            else PrepareInsertSteps(0, v);
        }
    } else if (insTail) {
        int v; if (parseVal(v)) {
            if ((int)nodes.size() >= MAX_NODES) SetMsg("List is full (max 12)!", {229,57,53,255});
            else PrepareInsertSteps((int)nodes.size(), v);
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
                PrepareInsertSteps(idx, v);
            }
        }
    }
    // ── Delete (instant, unchanged) ───────────────────────────────
    else if (delHead) {
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
    }
    // ── Search ────────────────────────────────────────────────────
    else if (doSearch) {
        int v; if (parseVal(v)) {
            bool found = false;
            for (auto& nd : nodes) nd.state = LLState::Normal;
            for (auto& nd : nodes) {
                if (nd.value == v) { nd.state = LLState::Found; found = true; }
            }
            SetMsg(found ? "Found!" : "Not found!", found ? Pal::BtnSuccess : Pal::BtnDanger);
        }
    }
    // ── Update ────────────────────────────────────────────────────
    else if (doUpdate) {
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

    // Header bar
    DrawRectangleRec({0, 0, 1280, 72}, Pal::Surface);
    DrawLineEx({0, 72}, {1280, 72}, 1.0f, Pal::Border);
    DrawTextEx(fontBold, "Linked List", {130, 20}, 28.0f, 1.0f, Pal::TxtDark);
    if (stepMode) {
        DrawTextEx(fontRegular, "Step-by-step mode  |  Use  < Prev / Next >  to navigate  |  Stop to cancel",
                   {130, 52}, 13.5f, 1.0f, Pal::BtnOrange);
    } else {
        DrawTextEx(fontRegular, "Insert  |  Delete  |  Search  |  Update: 'index value' (e.g. '2 50')",
                   {130, 52}, 13.5f, 1.0f, Pal::TxtLight);
    }
    btnBack.Draw();

    // Empty state
    if (nodes.empty() && !hasPendingNode) {
        DrawTextCentered(fontRegular, "List is empty — insert a value below",
                         NODE_Y + 5, 17.0f, Pal::TxtLight);
    }

    // Draw arrows between existing nodes
    int n = (int)nodes.size();
    for (int i = 0; i < n - 1; i++) {
        float ax = nodes[i].x + NODE_R;
        float ay = nodes[i].y;
        float bx = nodes[i+1].x - NODE_R;
        DrawLineEx({ax, ay}, {bx - 6, ay}, 2.0f, BLACK);
        DrawTriangle({bx, ay}, {bx-8, ay-5}, {bx-8, ay+5}, BLACK);
    }

    // Draw pending node (insert preview)
    if (hasPendingNode) {
        unsigned char a = (unsigned char)(pendingNode.alpha * 255);
        Color fillC = Pal::NodeHL;   fillC.a = a;
        Color bordC = {200, 160, 0, 255}; bordC.a = a;
        Color textC = Pal::TxtDark;  textC.a = a;
        Color lblC  = {200, 130, 0, 255}; lblC.a = a;

        DrawCircleV({pendingNode.x, pendingNode.y}, NODE_R + 2, bordC);
        DrawCircleV({pendingNode.x, pendingNode.y}, NODE_R,     fillC);
        Rectangle r = {pendingNode.x - NODE_R, pendingNode.y - NODE_R, 2*NODE_R, 2*NODE_R};
        char buf[16]; snprintf(buf, sizeof(buf), "%d", pendingNode.value);
        DrawTextInRect(fontBold, buf, r, 18.0f, textC);

        // "new" label above
        DrawTextEx(fontRegular, "new", {pendingNode.x - 14, pendingNode.y - NODE_R - 18}, 13.0f, 1.0f, lblC);

        // dashed downward arrow to insertion point
        float arrowX = pendingNode.x;
        float arrowY0 = pendingNode.y + NODE_R + 4;
        float arrowY1 = NODE_Y - NODE_R - 6;
        if (arrowY1 > arrowY0 + 8) {
            Color ac = {200, 130, 0, a};
            // dashes
            for (float y = arrowY0; y < arrowY1 - 6; y += 10) {
                float yEnd = y + 6 < arrowY1 - 6 ? y + 6 : arrowY1 - 6;
                DrawLineEx({arrowX, y}, {arrowX, yEnd}, 1.5f, ac);
            }
            DrawTriangle({arrowX, arrowY1}, {arrowX-5, arrowY1-8}, {arrowX+5, arrowY1-8}, ac);
        }
    }

    // Draw existing nodes
    for (const auto& nd : nodes) {
        Color fillC = Pal::NodeFill;
        Color bordC = Pal::NodeBorder;
        Color textC = Pal::TxtDark;

        if (nd.state == LLState::Found) {
            fillC = Pal::NodeFound;
            bordC = {36, 128, 54, 255};
            textC = WHITE;
        } else if (nd.state == LLState::Highlighted) {
            fillC = Pal::NodeHL;
            bordC = {200, 160, 0, 255};
        } else if (nd.state == LLState::Removing) {
            fillC = {255, 200, 200, 255};
            bordC = Pal::BtnDanger;
        }

        unsigned char a = (unsigned char)(nd.alpha * 255);
        fillC.a = a; bordC.a = a; textC.a = a;

        DrawCircleV({nd.x, nd.y}, NODE_R + 2, bordC);
        DrawCircleV({nd.x, nd.y}, NODE_R,     fillC);
        Rectangle r = {nd.x - NODE_R, nd.y - NODE_R, 2*NODE_R, 2*NODE_R};
        char buf[16]; snprintf(buf, sizeof(buf), "%d", nd.value);
        DrawTextInRect(fontBold, buf, r, 18.0f, textC);
    }

    // Bottom control panel
    DrawRectangleRec({0, 610, 1280, 110}, Pal::Panel);
    DrawLineEx({0, 610}, {1280, 610}, 1.0f, Pal::Border);

    if (stepMode) {
        // Step controls
        btnStop.Draw();
        btnPrev.Draw();
        btnNext.Draw();

        // Step description
        if (!message.empty()) {
            DrawTextEx(fontBold, message.c_str(), {376, 628}, 14.5f, 1.0f, {63, 81, 181, 255});
        }

        // Step counter + hint
        char cnt[48];
        snprintf(cnt, sizeof(cnt), "Step %d / %d", stepIdx + 1, (int)stepFrames.size());
        DrawTextEx(fontRegular, cnt, {1100, 630}, 14.0f, 1.0f, Pal::TxtMid);

        if (!stepFrames.empty() && stepFrames[stepIdx].isFinal) {
            DrawTextEx(fontRegular, "Press Next > to confirm insert",
                       {1050, 652}, 12.0f, 1.0f, Pal::BtnSuccess);
        } else {
            DrawTextEx(fontRegular, "← → arrow keys also work",
                       {1068, 652}, 12.0f, 1.0f, Pal::TxtLight);
        }
    } else {
        // Normal controls
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

        // Message
        if (msgTimer > 0 && !message.empty()) {
            float alpha = msgTimer < 0.5f ? msgTimer / 0.5f : 1.0f;
            Color c = msgColor;
            c.a = (unsigned char)(alpha * 220);
            DrawTextEx(fontRegular, message.c_str(), {960, 640}, 16.0f, 1.0f, c);
        }

        // Node count
        char cnt[32]; snprintf(cnt, sizeof(cnt), "Nodes: %d / %d", (int)nodes.size(), MAX_NODES);
        DrawTextEx(fontRegular, cnt, {1150, 640}, 14.0f, 1.0f, Pal::TxtLight);
    }
}
