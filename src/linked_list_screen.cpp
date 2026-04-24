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

LinkedListScreen::LinkedListScreen()
    : input({950, 16, 180, 40}, "Enter value...", 6),
      btnInsert ({355, 630, 120, 40}, "Insert",      Pal::BtnPrimary, Pal::BtnPrimHov),
      btnInsHead({355, 565,  57, 38}, "Head",        Pal::BtnPrimary, Pal::BtnPrimHov),
      btnInsTail({417, 565,  57, 38}, "Tail",        Pal::BtnSuccess, Pal::BtnSuccHov),
      btnInsIdx ({479, 565,  57, 38}, "Index",       Pal::Teal,       Pal::TealDark),
      btnDel    ({505, 630, 120, 40}, "Delete",      Pal::BtnDanger,  Pal::BtnDangHov),
      btnDelHead({505, 565,  47, 38}, "Head",        Pal::BtnDanger,  Pal::BtnDangHov),
      btnDelTail({557, 565,  47, 38}, "Tail",        Pal::BtnOrange,  Pal::BtnOrangeHov),
      btnDelIdx ({609, 565,  57, 38}, "Index",       Pal::Teal,       Pal::TealDark),
      btnSearch ({655, 630, 120, 40}, "Search",      Pal::BtnOrange,  Pal::BtnOrangeHov),
      btnUpdate ({805, 630, 120, 40}, "Update",      Pal::Teal,       Pal::TealDark),
      btnBack   ({20,  20,  100, 36}, "< Back",      Pal::BtnNeutral, Pal::BtnNeutHov),
      insertMenuOpen(false), deleteMenuOpen(false),
      msgTimer(0.0f), msgColor(Pal::BtnSuccess),
      btnShowCode({1140, 18, 120, 34}, "Show Code", Pal::BtnNeutral, Pal::BtnNeutHov) {}

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
    message = msg; msgColor = c; msgTimer = dur;
}

//nsert step

void LinkedListScreen::StartInsertStep(int idx, int v) {
    LLNode nd;
    nd.value = v; nd.alpha = 0.0f; nd.state = LLState::Normal;
    nd.x  = nodes.empty() ? 640.0f : (idx == 0 ? nodes[0].tx : nodes[idx > 0 ? idx-1 : 0].tx);
    nd.tx = nd.x;
    nd.y  = NODE_Y - 80.0f; nd.ty = NODE_Y;
    nodes.insert(nodes.begin() + idx, nd);
    LayoutNodes();
    for (auto& n : nodes) n.state = LLState::Normal;

    stepOp    = StepOp::Insert;
    stepActive = true;
    stepPhase  = 0;
    stepTimer  = 0.7f;
    stepIdx    = idx;
    input.Clear();
}

//Delete step

void LinkedListScreen::StartDeleteStep(int idx) {
    for (auto& n : nodes) n.state = LLState::Normal;

    stepOp     = StepOp::Delete;
    stepActive = true;
    stepPhase  = 0;
    stepIdx    = idx;

    // Phase 0: if idx==0 mark immediately as Removing, else start traversal
    if (idx == 0) {
        nodes[0].state = LLState::Removing;
        stepTimer = 0.7f;
    } else {
        nodes[0].state = LLState::Highlighted;
        stepTimer = 0.7f;
    }
    input.Clear();
}


void LinkedListScreen::AdvanceStep() {
    //Insert
    if (stepOp == StepOp::Insert) {
        stepPhase++;
        if (stepPhase == 1 && stepIdx == 0) stepPhase = 2; // no predecessor for head

        for (auto& n : nodes) n.state = LLState::Normal;
        switch (stepPhase) {
            case 1:
                nodes[stepIdx - 1].state = LLState::Highlighted;
                stepTimer = 0.7f; break;
            case 2:
                stepTimer = 0.7f; break; // arrows appear
            case 3:
                nodes[stepIdx].state = LLState::Highlighted;
                stepTimer = 0.7f; break;
            default:
                nodes[stepIdx].state = LLState::Normal;
                stepActive = false; stepOp = StepOp::None;
                char buf[64];
                snprintf(buf, sizeof(buf), "Inserted %d at index %d.", nodes[stepIdx].value, stepIdx);
                SetMsg(buf, Pal::BtnSuccess, 3.0f);
                break;
        }
        return;
    }

    //DeLete
    if (stepOp == StepOp::Delete) {
        stepPhase++;
        for (auto& n : nodes) n.state = LLState::Normal;

        if (stepPhase < stepIdx) {
            // Still traversing: move pointer to next node
            nodes[stepPhase].state = LLState::Highlighted;
            stepTimer = 0.7f;

        } else if (stepPhase == stepIdx) {
            // Reached target: mark for deletion
            nodes[stepIdx].state = LLState::Removing;
            stepTimer = 0.7f;

        } else if (stepPhase == stepIdx + 1) {
            // Remove node, then color predecessor (orange) and successor (green)
            int pred = stepIdx - 1;
            nodes.erase(nodes.begin() + stepIdx);
            LayoutNodes();
            if (pred >= 0)
                nodes[pred].state = LLState::Predecessor;
            if (stepIdx < (int)nodes.size())
                nodes[stepIdx].state = LLState::Highlighted;
            stepTimer = 0.7f;

        } else {
            // Done
            for (auto& n : nodes) n.state = LLState::Normal;
            stepActive = false; stepOp = StepOp::None;
            SetMsg("Deleted successfully.", Pal::BtnDanger, 3.0f);
        }
    }
}

//Update

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
    if (btnShowCode.Update()) {
        showCode = !showCode;
        btnShowCode.label = showCode ? "Hide Code" : "Show Code";
    }

    if (stepActive) {
        stepTimer -= dt;
        if (stepTimer <= 0.0f) AdvanceStep();
        return Screen::LinkedList;
    }

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

    // Insert
    if (insHead) {
        int v; if (parseVal(v)) {
            StartInsertStep(0, v);
        }
    } else if (insTail) {
        int v; if (parseVal(v)) {
            StartInsertStep((int)nodes.size(), v);
        }
    } else if (insIdx) {
        if (input.IsEmpty()) { SetMsg("Format: 'index value'  e.g. '1 50'", {229,57,53,255}); }
        else {
            int idx, v;
            if (sscanf(input.text.c_str(), "%d %d", &idx, &v) != 2)
                SetMsg("Format: 'index value'  e.g. '1 50'", {229,57,53,255});
            else if (idx < 0 || idx > (int)nodes.size()) {
                char buf[64]; snprintf(buf, sizeof(buf), "Index out of range (0-%d)!", (int)nodes.size());
                SetMsg(buf, {229,57,53,255});
            } else StartInsertStep(idx, v);
        }
    }
    // Delete
    else if (delHead) {
        if (nodes.empty()) SetMsg("List is empty!", {229,57,53,255});
        else StartDeleteStep(0);
    } else if (delTail) {
        if (nodes.empty()) SetMsg("List is empty!", {229,57,53,255});
        else StartDeleteStep((int)nodes.size() - 1);
    } else if (delIdx) {
        if (input.IsEmpty()) { SetMsg("Enter index to delete!", {229,57,53,255}); }
        else {
            int idx;
            if (sscanf(input.text.c_str(), "%d", &idx) != 1)
                SetMsg("Invalid index!", {229,57,53,255});
            else if (nodes.empty())
                SetMsg("List is empty!", {229,57,53,255});
            else if (idx < 0 || idx >= (int)nodes.size()) {
                char buf[64]; snprintf(buf, sizeof(buf), "Index out of range (0-%d)!", (int)nodes.size()-1);
                SetMsg(buf, {229,57,53,255});
            } else StartDeleteStep(idx);
        }
    }
    // Search
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
    // Update
    else if (doUpdate) {
        if (input.IsEmpty()) { SetMsg("Format: 'index value'  e.g. '2 50'", {229,57,53,255}); }
        else {
            int idx, newVal;
            if (sscanf(input.text.c_str(), "%d %d", &idx, &newVal) != 2)
                SetMsg("Format: 'index value'  e.g. '2 50'", {229,57,53,255});
            else if (nodes.empty())
                SetMsg("List is empty!", {229,57,53,255});
            else if (idx < 0 || idx >= (int)nodes.size()) {
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

//Draw

void LinkedListScreen::Draw() const {
    ClearBackground(Pal::BG);

    DrawRectangleRec({0, 0, 1280, 72}, Pal::Surface);
    DrawLineEx({0, 72}, {1280, 72}, 1.0f, Pal::Border);
    DrawTextEx(fontBold, "Linked List", {130, 20}, 28.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular,
               "Insert  |  Delete  |  Search  |  Update: 'index value' (e.g. '2 50')",
               {130, 52}, 13.5f, 1.0f, Pal::TxtLight);
    btnBack.Draw();
    btnShowCode.Draw();

    if (nodes.empty()) {
        DrawTextCentered(fontRegular, "List is empty — insert a value below",
                         NODE_Y + 5, 17.0f, Pal::TxtLight);
    }

    int n = (int)nodes.size();

    // Arrows and hide arrows to/from new node until insert phase 2
    for (int i = 0; i < n - 1; i++) {
        if (stepActive && stepOp == StepOp::Insert && stepPhase < 2) {
            if (i == stepIdx || i + 1 == stepIdx) continue;
        }
        float ax = nodes[i].x + NODE_R, ay = nodes[i].y;
        float bx = nodes[i+1].x - NODE_R;
        DrawLineEx({ax, ay}, {bx - 6, ay}, 2.0f, BLACK);
        DrawTriangle({bx, ay}, {bx-8, ay-5}, {bx-8, ay+5}, BLACK);
    }

    // Nodes
    for (const auto& nd : nodes) {
        Color fillC = Pal::NodeFill, bordC = Pal::NodeBorder, textC = Pal::TxtDark;
        switch (nd.state) {
            case LLState::Highlighted:
                fillC = {76, 175, 80, 255}; bordC = {36, 128, 54, 255}; textC = WHITE; break;
            case LLState::Found:
                fillC = Pal::NodeFound; bordC = {36,128,54,255}; textC = WHITE; break;
            case LLState::Removing:
                fillC = {239, 83, 80, 255}; bordC = {198, 40, 40, 255}; textC = WHITE; break;
            case LLState::Predecessor:
                fillC = {255, 204, 128, 255}; bordC = {245, 124, 0, 255}; textC = Pal::TxtDark; break;
            default: break;
        }
        unsigned char a = (unsigned char)(nd.alpha * 255);
        fillC.a = a; bordC.a = a; textC.a = a;

        DrawCircleV({nd.x, nd.y}, NODE_R + 2, bordC);
        DrawCircleV({nd.x, nd.y}, NODE_R,     fillC);
        Rectangle r = {nd.x - NODE_R, nd.y - NODE_R, 2*NODE_R, 2*NODE_R};
        char buf[16]; snprintf(buf, sizeof(buf), "%d", nd.value);
        DrawTextInRect(fontBold, buf, r, 18.0f, textC);
    }

    // Code panel
    if (showCode) {
        const char* codeLines[] = {
            "node* newNode = new node(val);",
            "node* cur = head;",
            "for (int i = 0; i < idx; i++)",
            "    cur = cur->next;",
            "newNode->next = cur->next;",
            "cur->next = newNode;"
        };
        constexpr int NUM_LINES = 6;

        // Which lines to highlight per phase
        bool hl[NUM_LINES] = {};
        switch (stepPhase) {
            case 0: hl[0] = true; break;
            case 1: hl[1] = hl[2] = hl[3] = true; break;
            case 2: hl[4] = hl[5] = true; break;
            case 3: for (auto& h : hl) h = true; break;
        }

        constexpr float PX = 878, PY = 86;
        constexpr float PW = 385, LH = 22;
        float PH = 14 + NUM_LINES * LH + 8;

        DrawRectangleRounded({PX, PY, PW, PH}, 0.1f, 8, {28, 35, 51, 235});
        DrawRectangleRoundedLines({PX, PY, PW, PH}, 0.1f, 8, {63, 81, 181, 200});

        for (int i = 0; i < NUM_LINES; i++) {
            float ly = PY + 18 + i * LH;
            if (hl[i]) {
                DrawRectangleRec({PX + 4, ly - 1, PW - 8, LH - 2}, {76, 175, 80, 55});
                DrawTextEx(fontRegular, codeLines[i], {PX + 10, ly + 2}, 13.0f, 1.0f, {144, 238, 144, 255});
            } else {
                DrawTextEx(fontRegular, codeLines[i], {PX + 10, ly + 2}, 13.0f, 1.0f, {160, 180, 200, 180});
            }
        }
    }

    // Bottom panel
    DrawRectangleRec({0, 610, 1280, 110}, Pal::Panel);
    DrawLineEx({0, 610}, {1280, 610}, 1.0f, Pal::Border);

    btnInsert.Draw();
    if (insertMenuOpen) {
        DrawRectangleRounded({350, 558, 191, 52}, 0.2f, 8, Pal::Surface);
        DrawRectangleRoundedLines({350, 558, 191, 52}, 0.2f, 8, Pal::Border);
        btnInsHead.Draw(); btnInsTail.Draw(); btnInsIdx.Draw();
    }
    input.Draw();
    btnDel.Draw();
    if (deleteMenuOpen) {
        DrawRectangleRounded({500, 558, 171, 52}, 0.2f, 8, Pal::Surface);
        DrawRectangleRoundedLines({500, 558, 171, 52}, 0.2f, 8, Pal::Border);
        btnDelHead.Draw(); btnDelTail.Draw(); btnDelIdx.Draw();
    }
    btnSearch.Draw();
    btnUpdate.Draw();

    if (msgTimer > 0 && !message.empty()) {
        float alpha = msgTimer < 0.5f ? msgTimer / 0.5f : 1.0f;
        Color c = msgColor; c.a = (unsigned char)(alpha * 220);
        DrawTextEx(fontRegular, message.c_str(), {480, 648}, 16.0f, 1.0f, c);
    }

    char cnt[32]; snprintf(cnt, sizeof(cnt), "Nodes: %d", (int)nodes.size());
    DrawTextEx(fontRegular, cnt, {1150, 640}, 14.0f, 1.0f, Pal::TxtLight);
}
