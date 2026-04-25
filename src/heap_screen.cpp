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
      btnMode   ({620, 636 , 140, 40}, "Mode: Step", Pal::BtnNeutral, Pal::BtnNeutHov),
      btnLoad   ({780, 636 , 140, 40}, "Load File", Pal::BtnNeutral, Pal::BtnNeutHov),
      msgTimer(0), msgColor(Pal::BtnSuccess),
      animA(-1), animB(-1), animC(-1), idCurrent(0), stepTimer(0), doingInsert(false), doingDelete(false), isHighlight(false), isStepByStep(true)
{}

void HeapScreen::InstantInsert(){
    int n = (int)heap.size() - 1;
    while(n && heap[n] > heap[(n - 1) / 2]){
        std::swap(heap[n], heap[(n - 1) / 2]);
        n = (n - 1) / 2;
    }
    ComputePositions();
    char buf[32]; snprintf(buf, sizeof(buf), "Inserted %d.", heap[idCurrent]);
    SetMsg(buf);
}

void HeapScreen::InstantDel(){
    heap[0] = heap.back();
    heap.pop_back();
    int n = (int)heap.size();
    int id = 0;
    while(true){
        int largest = id;
        int l = 2 * id + 1;
        int r = 2 * id + 2;
        if(l < n && heap[l] > heap[largest])largest = l;
        if(r < n && heap[r] > heap[largest])largest = r;
        if(largest == id){
            ComputePositions();
            char buf[32]; snprintf(buf, sizeof(buf), "Deleted max (Instant).");
            SetMsg(buf, Pal::BtnDanger);
            break;
        }
        else{
            std::swap(heap[id], heap[largest]);
            id = largest;
        }
    }
}

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
        v.highlighted = (i == animA || i == animB || i == animC);
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

    if(doingInsert || doingDelete){
        stepTimer += dt;
        if(stepTimer < 0.5f)return Screen::Heap;
        stepTimer = 0;
        if(doingInsert){
            if(idCurrent == 0){
                doingInsert = false;
                char buf[32]; snprintf(buf, sizeof(buf), "Inserted %d.", heap[idCurrent]);
                SetMsg(buf);
            }
            else{
                int parent = (idCurrent - 1) >> 1;
                animA = idCurrent;
                animB = parent;
                if(!isHighlight){
                    ComputePositions();
                    isHighlight = true;
                }
                else{
                    if(heap[idCurrent] > heap[parent]){
                        std::swap(heap[idCurrent], heap[parent]);
                        std::swap(vis[idCurrent], vis[parent]);
                        idCurrent = parent;
                        animA = animB = animC = -1;
                        ComputePositions();
                        isHighlight = false;
                    }
                    else{
                        doingInsert = false;
                        char buf[32]; snprintf(buf, sizeof(buf), "Inserted %d.", heap[idCurrent]);
                        idCurrent = 0;
                        SetMsg(buf);
                    }
                }
            }
        }
        if(doingDelete){
            int n = (int)heap.size();
            int largest = idCurrent;
            int l = 2*idCurrent+1, r = 2*idCurrent+2;
            animA = idCurrent, animB = l, animC = r;
            if(!isHighlight){
                ComputePositions();
                isHighlight = true;
            }
            else{
                if (l < n && heap[l] > heap[largest]) largest = l;
                if (r < n && heap[r] > heap[largest]) largest = r;
                if (largest == idCurrent){
                    animA = animB = animC = -1;
                    idCurrent = 0;
                    doingDelete = false;
                    animA = animB = animC = -1;
                    ComputePositions();
                    char buf[32]; snprintf(buf, sizeof(buf), "Deleted max.");
                    SetMsg(buf, Pal::BtnDanger);
                }
                else{
                    std::swap(heap[idCurrent], heap[largest]);
                    std::swap(vis[idCurrent], vis[largest]);
                    idCurrent = largest;
                    animA = animB = animC = -1;
                    animA = idCurrent;
                    animB = largest;
                    ComputePositions();
                    isHighlight = false;
                }
            }
        }
    }


    if (msgTimer > 0) msgTimer -= dt;

    if(doingInsert || doingDelete)return Screen::Heap;

    if (btnBack.Update() || IsKeyPressed(KEY_ESCAPE)) return Screen::Home;
    bool doIns;
    bool doDelMax;
    if(loadQueue.empty()){
        input.Update();

        if(btnMode.Update()){
            isStepByStep = !isStepByStep;
            if(btnMode.label == "Mode: Step"){
                btnMode.label = "Mode: Instant";
                btnMode.baseColor = Pal::BtnPrimary;
            }
            else{
                btnMode.label = "Mode: Step";
                btnMode.baseColor = Pal::BtnNeutral;
            }
        }

        doIns    = btnInsert.Update() || (input.focused && IsKeyPressed(KEY_ENTER));
        doDelMax = btnDelMax.Update();
    }

    if ( (doIns && !input.IsEmpty()) || !loadQueue.empty() ) {
        int v; 
        
        if(!loadQueue.empty()){
            v = loadQueue.front();
            loadQueue.pop();
        }
        else try { v = std::stoi(input.text); } catch(...) { SetMsg("Invalid number!", Pal::BtnDanger); return Screen::Heap; }
        if ((int)heap.size() >= MAX_SIZE) {
            SetMsg("Heap full (max 15)!", Pal::BtnDanger);
        } else {
            heap.push_back(v);
            if(isStepByStep){
                // Heapify up
                idCurrent = (int)heap.size() - 1;
                isHighlight = false;
                doingInsert = true;
                ComputePositions();
            }
            else{
                InstantInsert();
            }
            input.Clear();
        }
    } else if (doDelMax && !heap.empty() ) {
        if(isStepByStep){
            int maxVal = heap[0];
            heap[0] = heap.back();
            std::swap(vis[0], vis[(int)heap.size() - 1]);
            heap.pop_back();
            // Heapify down
            isHighlight = false;
            idCurrent = 0;   
            doingDelete = true;
            ComputePositions();
        }
        else{
            InstantDel();
        }
    }

    if (btnLoad.Update()) {
        std::string fullPath = std::string(PROJECT_ROOT_PATH) + "data.txt";
        std::vector<int> intList = InitFile::loadNumbers(fullPath);

        if (intList.empty()) {
            SetMsg("Failed to load or file empty", Pal::BtnDanger);
            return Screen::Heap;
        }

        if (isStepByStep) {
            for (const int& w : intList) loadQueue.push(w);
            SetMsg("Processing file step-by-step...", Pal::BtnOrange);
        } else {
            for (const int& w : intList) loadQueue.push(w);
            SetMsg("File loaded successfully (Instant)!");
        }
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
        // if (i == 0 && n > 0) { fillC = Pal::Indigo; bordC = Pal::IndigoDark; textC = WHITE; }

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
    btnMode.Draw();
    btnLoad.Draw();
    // Message
    if (msgTimer > 0 && !message.empty()) {
        float alpha = msgTimer < 0.5f ? msgTimer / 0.5f : 1.0f;
        Color c = msgColor; c.a = (unsigned char)(alpha * 220);
        DrawTextEx(fontRegular, message.c_str(), {900, 646}, 16.0f, 1.0f, c);
    }

    char cnt[32]; snprintf(cnt, sizeof(cnt), "Size: %d / %d", n, MAX_SIZE);
    DrawTextEx(fontRegular, cnt, {1150, 646}, 14.0f, 1.0f, Pal::TxtLight);
}
