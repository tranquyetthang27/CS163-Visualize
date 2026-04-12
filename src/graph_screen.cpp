#include "graph_screen.h"
#include "font.h"
#include "colors.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>

GraphScreen::GraphScreen()
    : stepIdx(0), mstCost(0), mstDone(false),
      btnStep ({20,  636, 150, 40}, "Next Step",  Pal::BtnPrimary, Pal::BtnPrimHov),
      btnReset({185, 636, 120, 40}, "Reset",      Pal::BtnDanger,  Pal::BtnDangHov),
      btnBack ({20,  20,  100, 36}, "< Back",     Pal::BtnNeutral, Pal::BtnNeutHov),
      msgTimer(0), msgColor(Pal::BtnSuccess)
{
    // 7 nodes placed aesthetically in a 1280x720 canvas
    // Left panel 0..860 for graph, right panel 870..1260 for edge list
    nodes[0] = {200, 200, "A"};
    nodes[1] = {450, 140, "B"};
    nodes[2] = {700, 200, "C"};
    nodes[3] = {800, 400, "D"};
    nodes[4] = {560, 480, "E"};
    nodes[5] = {300, 450, "F"};
    nodes[6] = {130, 370, "G"};
    // 11 edges (u, v, weight)
    edges[0]  = {0, 1, 7,  false, false, false};
    edges[1]  = {0, 5, 9,  false, false, false};
    edges[2]  = {0, 6, 14, false, false, false};
    edges[3]  = {1, 2, 8,  false, false, false};
    edges[4]  = {1, 5, 10, false, false, false};
    edges[5]  = {2, 3, 7,  false, false, false};
    edges[6]  = {2, 4, 2,  false, false, false};
    edges[7]  = {3, 4, 6,  false, false, false};
    edges[8]  = {4, 5, 11, false, false, false};
    edges[9]  = {5, 6, 2,  false, false, false};
    edges[10] = {3, 6, 9,  false, false, false};

    // Sort edge indices by weight
    for (int i = 0; i < GRAPH_E; i++) sortedEdges[i] = i;
    std::sort(sortedEdges, sortedEdges + GRAPH_E,
              [this](int a, int b){ return edges[a].w < edges[b].w; });

    ResetKruskal();
    SetMsg("Press \"Next Step\" to run Kruskal's MST algorithm.", Pal::TxtMid, 60.0f);
}

void GraphScreen::ResetKruskal() {
    for (int i = 0; i < GRAPH_N; i++) { 
        parent[i] = i; 
        ufRank[i] = 0; 
    }
    for (int i=0; i<GRAPH_E; i++) { 
        edges[i].inMST = false; 
        edges[i].skipped = false; 
        edges[i].highlighted = false; 
    }
    stepIdx = 0; 
    mstCost = 0; 
    mstDone = false;
}

int GraphScreen::Find(int x) {
    if (parent[x] != x) parent[x] = Find(parent[x]);
    return parent[x];
}

bool GraphScreen::Union(int x, int y) {
    int rx = Find(x), ry = Find(y);
    if (rx == ry) return false;
    if (ufRank[rx] < ufRank[ry]) {
        std::swap(rx, ry);
    }
    parent[ry] = rx;
    if (ufRank[rx] == ufRank[ry]) {
        ufRank[rx]++;
    }
    return true;
}

void GraphScreen::SetMsg(const char* msg, Color c, float dur) {
    message = msg; 
    msgColor = c; 
    msgTimer = dur;
}

Screen GraphScreen::Update() {
    float dt = GetFrameTime();
    if (msgTimer > 0) msgTimer -= dt;

    if (btnBack.Update() || IsKeyPressed(KEY_ESCAPE)) {
        return Screen::Home;
    }

    // Clear highlight
    for (auto& e : edges) {
        e.highlighted = false;
    }

    if (btnReset.Update()) {
        ResetKruskal();
        SetMsg("Reset. Press \"Next Step\" to start.", Pal::TxtMid, 60.0f);
    }

    if (btnStep.Update() && !mstDone) {
        if (stepIdx >= GRAPH_E) {
            mstDone = true;
            char buf[64]; 
            snprintf(buf, sizeof(buf), "MST complete! Total cost = %d.", mstCost);
            SetMsg(buf, Pal::BtnSuccess, 60.0f);
        } else {
            int ei = sortedEdges[stepIdx++];
            GEdge& e = edges[ei];
            e.highlighted = true;
            if (Union(e.u, e.v)) {
                e.inMST = true;
                mstCost += e.w;
                char buf[80];
                snprintf(buf, sizeof(buf),
                    "Added edge %s-%s (w=%d)  |  MST cost so far: %d",
                    nodes[e.u].label, nodes[e.v].label, e.w, mstCost);
                SetMsg(buf, Pal::BtnSuccess);
            } else {
                e.skipped = true;
                char buf[80];
                snprintf(buf, sizeof(buf),
                    "Skipped edge %s-%s (w=%d) — would create cycle.",
                    nodes[e.u].label, nodes[e.v].label, e.w);
                SetMsg(buf, Pal::BtnDanger);
            }

            // Check if MST complete (n-1 edges)
            int mstEdges = 0;
            for (auto& ed : edges) {
                if (ed.inMST) {
                    mstEdges++;
                }
            }
            if (mstEdges == GRAPH_N - 1) {
                mstDone = true;
                char buf[64]; 
                snprintf(buf, sizeof(buf), "MST complete! Total cost = %d.", mstCost);
                SetMsg(buf, Pal::BtnSuccess, 60.0f);
            }
        }
    }

    return Screen::MST;
}

static void DrawArrowEdge(Vector2 from, Vector2 to, float thick, Color c) {
    DrawLineEx(from, to, thick, c);
    // midpoint weight label handled separately
}

void GraphScreen::Draw() const {
    ClearBackground(Pal::BG);

    // Header
    DrawRectangleRec({0, 0, 1280, 72}, Pal::Surface);
    DrawLineEx({0, 72}, {1280, 72}, 1.0f, Pal::Border);
    DrawTextEx(fontBold, "Minimum Spanning Tree", {130, 20}, 28.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, "Kruskal's algorithm — greedy edge selection",
               {130, 52}, 13.5f, 1.0f, Pal::TxtLight);
    btnBack.Draw();

    // Divider: graph area | edge list
    DrawLineEx({870, 72}, {870, 610}, 1.0f, Pal::Border);

    // ---- Draw edges ----
    for (int i = 0; i < GRAPH_E; i++) {
        const GEdge& e = edges[i];
        Vector2 a = {nodes[e.u].x, nodes[e.u].y};
        Vector2 b = {nodes[e.v].x, nodes[e.v].y};

        Color ec;
        float thick;
        if (e.inMST) {
            ec = Pal::MSTEdge; 
            thick = 3.5f;
        } else if (e.highlighted) {
            ec = {100, 150, 255, 255}; 
            thick = 2.5f;
        } else if (e.skipped) {
            ec = {229, 57, 53, 100}; 
            thick = 1.5f;
        } else {
            ec = Pal::EdgeColor; 
            thick = 1.5f;
        }
        DrawLineEx(a, b, thick, ec);

        // Weight label at midpoint
        float mx = (a.x + b.x) / 2.0f + 6;
        float my = (a.y + b.y) / 2.0f - 10;
        char wbuf[8]; snprintf(wbuf, sizeof(wbuf), "%d", e.w);
        Color wc;
        if (e.inMST) {
            wc = Pal::MSTEdge;
        } else {
            wc = Pal::TxtMid;
        }
        DrawTextEx(fontBold, wbuf, {mx, my}, 14.0f, 1.0f, wc);
    }

    // ---- Draw nodes ----
    float nr = 22.0f;
    for (int i = 0; i < GRAPH_N; i++) {
        const GNode& nd = nodes[i];

        // Check if node is part of MST so far
        bool inMst = false;
        for (auto& e : edges)
            if (e.inMST && (e.u==i || e.v==i)) {
                inMst = true;
                break;
            }

        Color fillC = inMst ? Pal::Indigo : Pal::NodeFill;
        Color bordC = inMst ? Pal::IndigoDark : Pal::NodeBorder;
        Color textC = inMst ? WHITE : Pal::TxtDark;

        DrawCircleV({nd.x, nd.y}, nr + 2, bordC);
        DrawCircleV({nd.x, nd.y}, nr,     fillC);

        Vector2 ts = MeasureTextEx(fontBold, nd.label, 17.0f, 1.0f);
        DrawTextEx(fontBold, nd.label,
            {nd.x - ts.x/2, nd.y - ts.y/2}, 17.0f, 1.0f, textC);
    }

    // ---- Right panel: sorted edge list ----
    DrawTextEx(fontBold, "Edges (sorted by weight):",
               {885, 85}, 14.0f, 1.0f, Pal::TxtDark);

    for (int i = 0; i < GRAPH_E; i++) {
        int ei = sortedEdges[i];
        const GEdge& e = edges[ei];
        float ry = 112.0f + i * 42.0f;
        Rectangle row = {876, ry, 388, 36};

        Color rowBg = Pal::Panel;
        if (e.inMST) {
            rowBg = {200, 240, 210, 255};
            } else if (e.skipped) {
                rowBg = {255, 230, 230, 255};
            } else if (e.highlighted) {
                rowBg = {220, 230, 255, 255};
            }

        DrawRectangleRounded(row, 0.2f, 6, rowBg);

        // Status icon
        // const char* icon = e.inMST ? "[+]" : (e.skipped ? "[x]" : (i < stepIdx ? "   " : "   "));
        const char* icon;
        if (e.inMST) {
            icon = "[+]";
        } else if (e.skipped) {
            icon = "[x]";
        } else if (i < stepIdx) {
            icon = "   ";
        } else {
            icon = "   ";
        }
        Color ic;
        if (e.inMST) {
            ic = Pal::BtnSuccess;
        } else if (e.skipped) {
            ic = Pal::BtnDanger;
        } else {
            ic = Pal::TxtLight;
        }
        DrawTextEx(fontBold, icon, {882, ry + 10}, 13.0f, 1.0f, ic);

        char ebuf[32];
        snprintf(ebuf, sizeof(ebuf), "%s — %s    w = %d",
                 nodes[e.u].label, nodes[e.v].label, e.w);
        Color tc;
        if (e.inMST) {
            tc = Pal::BtnSuccess;
        } else if (e.skipped) {
            tc = Color{200, 80, 80, 255};
        } else {
            tc = Pal:: TxtDark;
        }
        DrawTextEx(fontRegular, 
                   ebuf, 
                   {910, ry + 10}, 
                   14.0f, 
                   1.0f, 
                   tc);
    }

    // MST cost summary
    if (mstCost > 0 || mstDone) {
        char cstbuf[48]; 
        snprintf(cstbuf, sizeof(cstbuf), "MST Cost: %d", mstCost);
        DrawTextEx(fontBold, cstbuf, {885, 580}, 18.0f, 1.0f,
                   mstDone ? Pal::BtnSuccess : Pal::TxtMid);
    }

    // Bottom panel
    DrawRectangleRec({0, 610, 1280, 110}, Pal::Panel);
    DrawLineEx({0, 610}, {1280, 610}, 1.0f, Pal::Border);
    btnStep.Draw();
    btnReset.Draw();

    if (mstDone) {
        DrawTextEx(fontBold, "MST Complete!", {330, 643}, 18.0f, 1.0f, Pal::BtnSuccess);
    }

    // Message
    if (msgTimer > 0 && !message.empty()) {
        float alphabet;
        if (msgTimer < 0.5f) {
            alphabet = msgTimer / 0.5f;
        } else {
            alphabet = 1.0f;
        }
        Color c = msgColor; 
        c.a = (unsigned char)(alphabet * 220);
        DrawTextEx(fontRegular, 
                   message.c_str(), 
                   {20, 680}, 
                   14.5f, 
                   1.0f, 
                   c);
    }
}
