#include "graph_screen.h"
#include "font.h"
#include "colors.h"
#include <cstdio>

namespace {

const char* ModeTitle(GraphInputMode mode) {
    switch (mode) {
        case GraphInputMode::EdgeList:        return "Edge List";
        case GraphInputMode::AdjacencyMatrix: return "Adjacency Matrix";
        case GraphInputMode::AdjacencyList:   return "Adjacency List";
    }
    return "Edge List";
}

const char* ModeHint(GraphInputMode mode) {
    switch (mode) {
        case GraphInputMode::EdgeList:
            return "Enter source, destination, and weight for each edge.";
        case GraphInputMode::AdjacencyMatrix:
            return "Fill the matrix with weights. Use 0 for no edge.";
        case GraphInputMode::AdjacencyList:
            return "Type neighbors for each node in list form.";
    }
    return "";
}

bool DrawModeTab(Rectangle rect, const char* label, bool active) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);
    Color base = active ? Pal::BtnPrimary : Pal::Surface;
    Color hover = active ? Pal::BtnPrimHov : Pal::PanelDark;
    Color fill = hovered ? hover : base;
    Color border = active ? Pal::BtnPrimary : Pal::Border;

    DrawRectangleRounded(rect, 0.22f, 8, fill);
    DrawRectangleRoundedLines(rect, 0.22f, 8, border);

    Vector2 ts = MeasureTextEx(fontBold, label, 15.0f, 1.0f);
    DrawTextEx(fontBold, label,
               {rect.x + rect.width / 2.0f - ts.x / 2.0f, rect.y + rect.height / 2.0f - ts.y / 2.0f},
               15.0f, 1.0f, active ? WHITE : Pal::TxtDark);

    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void DrawSectionHeader(const char* title, const char* hint, float x, float y) {
    DrawTextEx(fontBold, title, {x, y}, 18.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, hint, {x, y + 20.0f}, 12.8f, 1.0f, Pal::TxtLight);
}

}

GraphScreen::GraphScreen()
    : inputMode(GraphInputMode::EdgeList),
      btnBack({20, 20, 100, 36}, "< Back", Pal::BtnNeutral, Pal::BtnNeutHov),
      btnDelete({440, 636, 150, 40}, "Delete", Pal::BtnDanger, Pal::BtnDangHov),
      btnEdit({690, 636, 150, 40}, "Edit", Pal::BtnPrimary, Pal::BtnPrimHov),
      msgTimer(0), msgColor(Pal::TxtMid)
{
    nodes[0] = {200, 200, "A"};
    nodes[1] = {450, 140, "B"};
    nodes[2] = {700, 200, "C"};
    nodes[3] = {800, 400, "D"};
    nodes[4] = {560, 480, "E"};
    nodes[5] = {300, 450, "F"};
    nodes[6] = {130, 370, "G"};

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

    for (int i = 0; i < GRAPH_E; i++) {
        float y = 334.0f + i * 25.0f;
        edgeFromFields[i]   = InputField({892.0f, y, 46.0f, 22.0f}, "u", 2);
        edgeToFields[i]     = InputField({948.0f, y, 46.0f, 22.0f}, "v", 2);
        edgeWeightFields[i]  = InputField({1004.0f, y, 88.0f, 22.0f}, "w", 4);
    }

    for (int r = 0; r < GRAPH_N; r++) {
        for (int c = 0; c < GRAPH_N; c++) {
            float x = 929.0f + c * 33.0f;
            float y = 334.0f + r * 33.0f;
            matrixFields[r][c] = InputField({x, y, 28.0f, 22.0f}, "0", 3);
        }
    }

    for (int i = 0; i < GRAPH_N; i++) {
        float y = 338.0f + i * 36.0f;
        adjListFields[i] = InputField({952.0f, y, 240.0f, 24.0f}, "neighbor,weight", 32);
    }

    SetMsg("Choose an input style on the right.", Pal::TxtMid, 60.0f);
}

void GraphScreen::ClearInputFocus() {
    for (int i = 0; i < GRAPH_E; i++) {
        edgeFromFields[i].focused = false;
        edgeToFields[i].focused = false;
        edgeWeightFields[i].focused = false;
    }
    for (int r = 0; r < GRAPH_N; r++) {
        adjListFields[r].focused = false;
        for (int c = 0; c < GRAPH_N; c++) {
            matrixFields[r][c].focused = false;
        }
    }
}

void GraphScreen::SetInputMode(GraphInputMode mode) {
    if (inputMode == mode) {
        return;
    }
    inputMode = mode;
    ClearInputFocus();
    SetMsg(ModeHint(mode), Pal::TxtMid, 3.0f);
}

void GraphScreen::SetMsg(const char* msg, Color c, float dur) {
    message = msg;
    msgColor = c;
    msgTimer = dur;
}

Screen GraphScreen::Update() {
    float dt = GetFrameTime();
    if (msgTimer > 0.0f) {
        msgTimer -= dt;
        if (msgTimer < 0.0f) {
            msgTimer = 0.0f;
        }
    }

    if (btnBack.Update() || IsKeyPressed(KEY_ESCAPE)) {
        return Screen::Home;
    }

    Rectangle tabRects[3] = {
        {884, 146, 360, 34},
        {884, 188, 360, 34},
        {884, 230, 360, 34}
    };

    if (CheckCollisionPointRec(GetMousePosition(), tabRects[0]) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        SetInputMode(GraphInputMode::EdgeList);
    }
    if (CheckCollisionPointRec(GetMousePosition(), tabRects[1]) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        SetInputMode(GraphInputMode::AdjacencyMatrix);
    }
    if (CheckCollisionPointRec(GetMousePosition(), tabRects[2]) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        SetInputMode(GraphInputMode::AdjacencyList);
    }

    if (inputMode == GraphInputMode::EdgeList) {
        for (int i = 0; i < GRAPH_E; i++) {
            edgeFromFields[i].Update();
            edgeToFields[i].Update();
            edgeWeightFields[i].Update();
        }
    } else if (inputMode == GraphInputMode::AdjacencyMatrix) {
        for (int r = 0; r < GRAPH_N; r++) {
            for (int c = 0; c < GRAPH_N; c++) {
                matrixFields[r][c].Update();
            }
        }
    } else {
        for (int i = 0; i < GRAPH_N; i++) {
            adjListFields[i].Update();
        }
    }

    btnDelete.Update();
    btnEdit.Update();

    return Screen::MST;
}

void GraphScreen::Draw() const {
    ClearBackground(Pal::BG);

    DrawRectangleRec({0, 0, 1280, 72}, Pal::Surface);
    DrawLineEx({0, 72}, {1280, 72}, 1.0f, Pal::Border);
    DrawTextEx(fontBold, "Graph Input", {130, 20}, 28.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, "Left side keeps the preview graph; the right side is for choosing input format.",
               {130, 52}, 13.2f, 1.0f, Pal::TxtLight);
    btnBack.Draw();

    DrawLineEx({860, 72}, {860, 610}, 1.0f, Pal::Border);

    for (int i = 0; i < GRAPH_E; i++) {
        const GEdge& e = edges[i];
        Vector2 a = {nodes[e.u].x, nodes[e.u].y};
        Vector2 b = {nodes[e.v].x, nodes[e.v].y};

        Color ec = Pal::EdgeColor;
        float thick = 1.5f;
        if (e.inMST) {
            ec = Pal::MSTEdge;
            thick = 3.5f;
        } else if (e.highlighted) {
            ec = {100, 150, 255, 255};
            thick = 2.5f;
        } else if (e.skipped) {
            ec = {229, 57, 53, 100};
            thick = 1.5f;
        }

        DrawLineEx(a, b, thick, ec);

        float mx = (a.x + b.x) / 2.0f + 6.0f;
        float my = (a.y + b.y) / 2.0f - 10.0f;
        char wbuf[8];
        snprintf(wbuf, sizeof(wbuf), "%d", e.w);
        DrawTextEx(fontBold, wbuf, {mx, my}, 14.0f, 1.0f, e.inMST ? Pal::MSTEdge : Pal::TxtMid);
    }

    float nr = 22.0f;
    for (int i = 0; i < GRAPH_N; i++) {
        const GNode& nd = nodes[i];

        bool inMst = false;
        for (const auto& e : edges) {
            if (e.inMST && (e.u == i || e.v == i)) {
                inMst = true;
                break;
            }
        }

        Color fillC = inMst ? Pal::Indigo : Pal::NodeFill;
        Color bordC = inMst ? Pal::IndigoDark : Pal::NodeBorder;
        Color textC = inMst ? WHITE : Pal::TxtDark;

        DrawCircleV({nd.x, nd.y}, nr + 2, bordC);
        DrawCircleV({nd.x, nd.y}, nr, fillC);

        Vector2 ts = MeasureTextEx(fontBold, nd.label, 17.0f, 1.0f);
        DrawTextEx(fontBold, nd.label, {nd.x - ts.x / 2.0f, nd.y - ts.y / 2.0f}, 17.0f, 1.0f, textC);
    }

    DrawRectangleRec({872, 86, 388, 516}, Pal::Panel);
    DrawRectangleRoundedLines({872, 86, 388, 516}, 0.12f, 8, Pal::Border);

    DrawTextEx(fontBold, "Input Graph", {892, 102}, 18.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, "Pick one format below and type directly into the table.",
               {892, 124}, 12.6f, 1.0f, Pal::TxtLight);

    Rectangle tabRects[3] = {
        {884, 146, 360, 34},
        {884, 188, 360, 34},
        {884, 230, 360, 34}
    };
    DrawModeTab(tabRects[0], "Edge List", inputMode == GraphInputMode::EdgeList);
    DrawModeTab(tabRects[1], "Adjacency Matrix", inputMode == GraphInputMode::AdjacencyMatrix);
    DrawModeTab(tabRects[2], "Adjacency List", inputMode == GraphInputMode::AdjacencyList);

    DrawRectangleRounded({884, 276, 360, 304}, 0.12f, 8, Pal::Surface);
    DrawRectangleRoundedLines({884, 276, 360, 304}, 0.12f, 8, Pal::Border);

    if (inputMode == GraphInputMode::EdgeList) {
        DrawSectionHeader("Edge List", "U, V, and weight for each edge.", 902, 292);

        DrawTextEx(fontBold, "#", {894, 320}, 12.0f, 1.0f, Pal::TxtLight);
        DrawTextEx(fontBold, "U", {904, 320}, 12.0f, 1.0f, Pal::TxtLight);
        DrawTextEx(fontBold, "V", {960, 320}, 12.0f, 1.0f, Pal::TxtLight);
        DrawTextEx(fontBold, "W", {1016, 320}, 12.0f, 1.0f, Pal::TxtLight);

        for (int i = 0; i < GRAPH_E; i++) {
            char rowNo[8];
            snprintf(rowNo, sizeof(rowNo), "%02d", i + 1);
            DrawTextEx(fontRegular, rowNo, {894, 337.0f + i * 25.0f}, 12.0f, 1.0f, Pal::TxtMid);
            edgeFromFields[i].Draw();
            edgeToFields[i].Draw();
            edgeWeightFields[i].Draw();
        }
    } else if (inputMode == GraphInputMode::AdjacencyMatrix) {
        DrawSectionHeader("Adjacency Matrix", "Rows and columns use nodes A through G.", 902, 292);

        for (int c = 0; c < GRAPH_N; c++) {
            char label[4];
            snprintf(label, sizeof(label), "%c", nodes[c].label[0]);
            DrawTextEx(fontBold, label, {936.0f + c * 33.0f, 322}, 12.0f, 1.0f, Pal::TxtLight);
        }

        for (int r = 0; r < GRAPH_N; r++) {
            char label[4];
            snprintf(label, sizeof(label), "%c", nodes[r].label[0]);
            DrawTextEx(fontBold, label, {905, 339.0f + r * 33.0f}, 12.0f, 1.0f, Pal::TxtLight);
            for (int c = 0; c < GRAPH_N; c++) {
                matrixFields[r][c].Draw();
            }
        }
    } else {
        DrawSectionHeader("Adjacency List", "Write the neighbors for each node in one row.", 902, 292);

        for (int i = 0; i < GRAPH_N; i++) {
            char label[4];
            snprintf(label, sizeof(label), "%c", nodes[i].label[0]);
            DrawTextEx(fontBold, label, {902, 345.0f + i * 36.0f}, 13.0f, 1.0f, Pal::TxtMid);
            adjListFields[i].Draw();
        }
    }

    DrawRectangleRec({0, 610, 1280, 110}, Pal::Panel);
    DrawLineEx({0, 610}, {1280, 610}, 1.0f, Pal::Border);

    btnDelete.Draw();
    btnEdit.Draw();

    DrawTextEx(fontRegular, "Delete and Edit are UI placeholders for now.", {20, 645}, 13.0f, 1.0f, Pal::TxtLight);

    if (msgTimer > 0.0f && !message.empty()) {
        float alpha = msgTimer < 0.5f ? msgTimer / 0.5f : 1.0f;
        Color c = msgColor;
        c.a = (unsigned char)(alpha * 220.0f);
        DrawTextEx(fontRegular, message.c_str(), {20, 680}, 14.5f, 1.0f, c);
    }
}
