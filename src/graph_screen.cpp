#include "graph_screen.h"

#include "colors.h"
#include "font.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace {

constexpr float kNodeRadius = 22.0f;
constexpr int kGraphN = 7;
constexpr int kGraphE = 11;

constexpr Rectangle kInputPanel = {872, 86, 388, 516};
constexpr Rectangle kInputContentArea = {884, 306, 360, 246};
constexpr Rectangle kInputClip = {892, 340, 334, 196};
constexpr Rectangle kScrollTrack = {1236, 314, 8, 222};

constexpr float kEdgeListSttX = 894.0f;
constexpr float kEdgeListUX = 936.0f;
constexpr float kEdgeListVX = 992.0f;
constexpr float kEdgeListWX = 1048.0f;
constexpr float kEdgeListWeightW = 88.0f;
constexpr float kEdgeListRowY = 356.0f;
constexpr float kEdgeListRowGap = 24.0f;

constexpr float kMatrixColX = 936.0f;
constexpr float kMatrixRowY = 363.0f;
constexpr float kMatrixGap = 33.0f;

constexpr float kAdjRowY = 369.0f;
constexpr float kAdjRowGap = 36.0f;

constexpr Rectangle kTabEdge = {884, 176, 360, 34};
constexpr Rectangle kTabMatrix = {884, 218, 360, 34};
constexpr Rectangle kTabAdj = {884, 260, 360, 34};

bool ParseIntStrict(const std::string& text, int* value) {
    if (text.empty()) {
        return false;
    }
    char* end = nullptr;
    long parsed = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0') {
        return false;
    }
    *value = static_cast<int>(parsed);
    return true;
}

bool ParsePairToken(const std::string& token, int* neighbor, int* weight) {
    size_t pos = token.find_first_of(",:/-");
    if (pos == std::string::npos || pos == 0 || pos + 1 >= token.size()) {
        return false;
    }
    std::string left = token.substr(0, pos);
    std::string right = token.substr(pos + 1);
    return ParseIntStrict(left, neighbor) && ParseIntStrict(right, weight);
}

bool DrawModeTab(Rectangle rect, const char* label, bool active) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);

    Color fill = active ? Pal::BtnPrimary : Pal::Surface;
    if (hovered && !active) {
        fill = Pal::PanelDark;
    } else if (hovered && active) {
        fill = Pal::BtnPrimHov;
    }

    DrawRectangleRounded(rect, 0.22f, 8, fill);
    DrawRectangleRoundedLines(rect, 0.22f, 8, active ? Pal::BtnPrimary : Pal::Border);

    Vector2 ts = MeasureTextEx(fontBold, label, 15.0f, 1.0f);
    DrawTextEx(
        fontBold,
        label,
        {rect.x + rect.width * 0.5f - ts.x * 0.5f, rect.y + rect.height * 0.5f - ts.y * 0.5f},
        15.0f,
        1.0f,
        active ? WHITE : Pal::TxtDark
    );

    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void LayoutEdgeListFields(InputField* fromFields, InputField* toFields, InputField* weightFields, float scrollY) {
    for (int i = 0; i < kGraphE; i++) {
        float y = 337.0f + i * 25.0f - scrollY;
        fromFields[i].rect = {kEdgeListUX, y, 46.0f, 22.0f};
        toFields[i].rect = {kEdgeListVX, y, 46.0f, 22.0f};
        weightFields[i].rect = {kEdgeListWX, y, kEdgeListWeightW, 22.0f};
    }
}

void LayoutMatrixFields(InputField fields[kGraphN][kGraphN], float scrollY) {
    for (int r = 0; r < kGraphN; r++) {
        for (int c = 0; c < kGraphN; c++) {
            float x = 929.0f + c * kMatrixGap;
            float y = 334.0f + r * kMatrixGap - scrollY;
            fields[r][c].rect = {x, y, 28.0f, 22.0f};
        }
    }
}

void LayoutAdjListFields(InputField* fields, float scrollY) {
    for (int i = 0; i < kGraphN; i++) {
        float y = 338.0f + i * kAdjRowGap - scrollY;
        fields[i].rect = {952.0f, y, 240.0f, 24.0f};
    }
}

void DrawFixedHeader(const char* title, const char* hint) {
    DrawTextEx(fontBold, title, {902, 292}, 18.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, hint, {902, 312}, 12.4f, 1.0f, Pal::TxtLight);
}

float DistanceToSegment(Vector2 point, Vector2 a, Vector2 b) {
    Vector2 ab = {b.x - a.x, b.y - a.y};
    Vector2 ap = {point.x - a.x, point.y - a.y};
    float ab2 = ab.x * ab.x + ab.y * ab.y;
    if (ab2 <= 0.0001f) {
        float dx = point.x - a.x;
        float dy = point.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    float t = (ap.x * ab.x + ap.y * ab.y) / ab2;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    Vector2 c = {a.x + ab.x * t, a.y + ab.y * t};
    float dx = point.x - c.x;
    float dy = point.y - c.y;
    return std::sqrt(dx * dx + dy * dy);
}

void DrawSectionHeader(const char* title, const char* hint, float x, float y) {
    DrawTextEx(fontBold, title, {x, y}, 18.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, hint, {x, y + 20.0f}, 12.4f, 1.0f, Pal::TxtLight);
}

}  // namespace

GraphScreen::GraphScreen()
    : inputMode(GraphInputMode::EdgeList),
      indexBase(0),
      selectionType(GraphSelectionType::None),
      selectedIndex(-1),
            editTarget(GraphEditTarget::None),
            editDialogOpen(false),
            inputScrollY(0.0f),
            inputScrollDragging(false),
            inputScrollDragOffset(0.0f),
      btnBack({20, 20, 100, 36}, "< Back", Pal::BtnNeutral, Pal::BtnNeutHov),
      btnDelete({440, 636, 150, 40}, "Delete", Pal::BtnDanger, Pal::BtnDangHov),
      btnEdit({690, 636, 150, 40}, "Edit", Pal::BtnPrimary, Pal::BtnPrimHov),
            btnEditOk({510, 416, 114, 34}, "OK", Pal::BtnSuccess, Pal::BtnSuccHov),
            btnEditCancel({636, 416, 114, 34}, "Cancel", Pal::BtnNeutral, Pal::BtnNeutHov),
            editField({410, 344, 330, 34}, "new value", 24),
      msgTimer(0.0f),
      msgColor(Pal::TxtMid) {
    nodes[0] = {200, 200, "A", true};
    nodes[1] = {450, 140, "B", true};
    nodes[2] = {700, 200, "C", true};
    nodes[3] = {800, 400, "D", true};
    nodes[4] = {560, 480, "E", true};
    nodes[5] = {300, 450, "F", true};
    nodes[6] = {130, 370, "G", true};

    edges = {
        {0, 1, 7, true},
        {0, 5, 9, true},
        {0, 6, 14, true},
        {1, 2, 8, true},
        {1, 5, 10, true},
        {2, 3, 7, true},
        {2, 4, 2, true},
        {3, 4, 6, true},
        {4, 5, 11, true},
        {5, 6, 2, true},
        {3, 6, 9, true}
    };

    for (int i = 0; i < GRAPH_E; i++) {
        float y = 337.0f + i * 25.0f;
        edgeFromFields[i] = InputField({904.0f, y, 46.0f, 22.0f}, "u", 3);
        edgeToFields[i] = InputField({960.0f, y, 46.0f, 22.0f}, "v", 3);
        edgeWeightFields[i] = InputField({1016.0f, y, 88.0f, 22.0f}, "w", 5);
    }

    for (int r = 0; r < GRAPH_N; r++) {
        for (int c = 0; c < GRAPH_N; c++) {
            float x = 929.0f + c * 33.0f;
            float y = 334.0f + r * 33.0f;
            matrixFields[r][c] = InputField({x, y, 28.0f, 22.0f}, "0", 4);
        }
    }

    for (int i = 0; i < GRAPH_N; i++) {
        float y = 338.0f + i * 36.0f;
        adjListFields[i] = InputField({952.0f, y, 240.0f, 24.0f}, "neighbor,weight", 64);
    }

    SyncFieldsFromGraph();
    SetMsg("Choose an input format on the right.", Pal::TxtMid, 6.0f);
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
    editField.focused = false;
}

void GraphScreen::ClearSelection() {
    selectionType = GraphSelectionType::None;
    selectedIndex = -1;
}

void GraphScreen::SetInputMode(GraphInputMode mode) {
    if (inputMode == mode) {
        return;
    }
    inputMode = mode;
    inputScrollY = 0.0f;
    ClearInputFocus();
    SyncFieldsFromGraph();
}

void GraphScreen::SetIndexBase(int base) {
    if (indexBase == base) {
        return;
    }
    indexBase = base;
    inputScrollY = 0.0f;
    SyncFieldsFromGraph();
}

void GraphScreen::SyncFieldsFromGraph(bool clearFocus) {
    if (clearFocus) {
        ClearInputFocus();
    }

    for (int i = 0; i < GRAPH_E; i++) {
        edgeFromFields[i].Clear();
        edgeToFields[i].Clear();
        edgeWeightFields[i].Clear();

        if (i < static_cast<int>(edges.size()) && edges[i].visible) {
            edgeFromFields[i].text = std::to_string(edges[i].u + indexBase);
            edgeToFields[i].text = std::to_string(edges[i].v + indexBase);
            edgeWeightFields[i].text = std::to_string(edges[i].w);
        }
    }

    for (int r = 0; r < GRAPH_N; r++) {
        for (int c = 0; c < GRAPH_N; c++) {
            matrixFields[r][c].text = "0";
        }
    }

    for (const auto& edge : edges) {
        if (!edge.visible) {
            continue;
        }
        matrixFields[edge.u][edge.v].text = std::to_string(edge.w);
        matrixFields[edge.v][edge.u].text = std::to_string(edge.w);
    }

    std::string rows[GRAPH_N];
    for (const auto& edge : edges) {
        if (!edge.visible) {
            continue;
        }

        if (!rows[edge.u].empty()) rows[edge.u] += ' ';
        rows[edge.u] += std::to_string(edge.v + indexBase) + "," + std::to_string(edge.w);

        if (!rows[edge.v].empty()) rows[edge.v] += ' ';
        rows[edge.v] += std::to_string(edge.u + indexBase) + "," + std::to_string(edge.w);
    }

    for (int i = 0; i < GRAPH_N; i++) {
        adjListFields[i].text = rows[i];
    }
}

bool GraphScreen::ApplyInputToGraph(bool showMessage) {
    std::vector<GEdge> nextEdges;

    auto addEdge = [&](int u, int v, int w) {
        if (u < 0 || v < 0 || u >= GRAPH_N || v >= GRAPH_N || u == v) {
            return;
        }
        nextEdges.push_back({u, v, w, true});
    };

    if (inputMode == GraphInputMode::EdgeList) {
        for (int i = 0; i < GRAPH_E; i++) {
            bool emptyRow = edgeFromFields[i].text.empty() && edgeToFields[i].text.empty() && edgeWeightFields[i].text.empty();
            if (emptyRow) {
                continue;
            }

            int u = 0;
            int v = 0;
            int w = 0;
            if (!ParseIntStrict(edgeFromFields[i].text, &u) || !ParseIntStrict(edgeToFields[i].text, &v) || !ParseIntStrict(edgeWeightFields[i].text, &w)) {
                if (showMessage) {
                    SetMsg("Edge List rows need numeric u, v, w.", Pal::BtnDanger, 2.5f);
                }
                return false;
            }
            addEdge(u - indexBase, v - indexBase, w);
        }
    } else if (inputMode == GraphInputMode::AdjacencyMatrix) {
        for (int r = 0; r < GRAPH_N; r++) {
            for (int c = r + 1; c < GRAPH_N; c++) {
                int w = 0;
                if (!ParseIntStrict(matrixFields[r][c].text, &w)) {
                    if (showMessage) {
                        SetMsg("Matrix values must be numbers.", Pal::BtnDanger, 2.5f);
                    }
                    return false;
                }
                if (w != 0) {
                    addEdge(r, c, w);
                }
            }
        }
    } else {
        for (int r = 0; r < GRAPH_N; r++) {
            std::istringstream row(adjListFields[r].text);
            std::string token;
            while (row >> token) {
                int neighbor = 0;
                int weight = 0;
                if (!ParsePairToken(token, &neighbor, &weight)) {
                    if (showMessage) {
                        SetMsg("Adjacency List uses neighbor,weight tokens.", Pal::BtnDanger, 2.5f);
                    }
                    return false;
                }

                neighbor -= indexBase;
                if (neighbor > r) {
                    addEdge(r, neighbor, weight);
                }
            }
        }
    }

    edges = std::move(nextEdges);
    if (selectionType == GraphSelectionType::Edge && selectedIndex >= static_cast<int>(edges.size())) {
        ClearSelection();
    }
    if (selectionType == GraphSelectionType::Node && (selectedIndex < 0 || selectedIndex >= GRAPH_N || !nodes[selectedIndex].visible)) {
        ClearSelection();
    }
    if (showMessage) {
        SetMsg("Graph updated from input.", Pal::BtnSuccess, 2.0f);
    }
    return true;
}

void GraphScreen::OpenEditDialog() {
    if (selectionType == GraphSelectionType::None || selectedIndex < 0) {
        SetMsg("Click a node or edge first.", Pal::BtnDanger, 2.5f);
        return;
    }

    editDialogOpen = true;
    editField.focused = true;
    editField.Clear();
    if (selectionType == GraphSelectionType::Node) {
        editTarget = GraphEditTarget::NodeLabel;
        editField.text = nodes[selectedIndex].label;
    } else {
        editTarget = GraphEditTarget::EdgeWeight;
        editField.text = std::to_string(edges[selectedIndex].w);
    }
}

void GraphScreen::ApplySelectedEdit() {
    if (!editDialogOpen || selectionType == GraphSelectionType::None || selectedIndex < 0) {
        return;
    }

    if (editTarget == GraphEditTarget::NodeLabel) {
        if (editField.text.empty()) {
            SetMsg("Node label cannot be empty.", Pal::BtnDanger, 2.5f);
            return;
        }
        nodes[selectedIndex].label = editField.text;
    } else if (editTarget == GraphEditTarget::EdgeWeight) {
        int weight = 0;
        if (!ParseIntStrict(editField.text, &weight)) {
            SetMsg("Edge weight must be numeric.", Pal::BtnDanger, 2.5f);
            return;
        }
        edges[selectedIndex].w = weight;
    }

    editDialogOpen = false;
    editTarget = GraphEditTarget::None;
    ClearInputFocus();
    SyncFieldsFromGraph(false);
    SetMsg("Value updated.", Pal::BtnSuccess, 2.0f);
}

void GraphScreen::DeleteSelected() {
    if (selectionType == GraphSelectionType::None || selectedIndex < 0) {
        SetMsg("Click a node or edge first.", Pal::BtnDanger, 2.5f);
        return;
    }

    if (selectionType == GraphSelectionType::Node) {
        nodes[selectedIndex].visible = false;
        for (auto& edge : edges) {
            if (edge.u == selectedIndex || edge.v == selectedIndex) {
                edge.visible = false;
            }
        }
        SetMsg("Node removed from preview.", Pal::BtnSuccess, 2.0f);
    } else {
        edges[selectedIndex].visible = false;
        SetMsg("Edge removed from preview.", Pal::BtnSuccess, 2.0f);
    }

    ClearSelection();
    SyncFieldsFromGraph(false);
}

float GraphScreen::GetInputContentHeight() const {
    switch (inputMode) {
        case GraphInputMode::EdgeList: return 356.0f;
        case GraphInputMode::AdjacencyMatrix: return 385.0f;
        case GraphInputMode::AdjacencyList: return 350.0f;
    }
    return 356.0f;
}

float GraphScreen::GetInputMaxScroll() const {
    float h = GetInputContentHeight() - kInputClip.height;
    return h > 0.0f ? h : 0.0f;
}

void GraphScreen::ClampInputScroll() {
    float maxScroll = GetInputMaxScroll();
    if (inputScrollY < 0.0f) inputScrollY = 0.0f;
    if (inputScrollY > maxScroll) inputScrollY = maxScroll;
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

    Rectangle tabEdge = kTabEdge;
    Rectangle tabMatrix = kTabMatrix;
    Rectangle tabAdj = kTabAdj;
    Rectangle idx0 = {890, 138, 74, 26};
    Rectangle idx1 = {972, 138, 74, 26};

    Vector2 mouse = GetMousePosition();
    if (!editDialogOpen) {
        if (CheckCollisionPointRec(mouse, idx0) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            SetIndexBase(0);
        }
        if (CheckCollisionPointRec(mouse, idx1) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            SetIndexBase(1);
        }

        if (CheckCollisionPointRec(mouse, tabEdge) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            SetInputMode(GraphInputMode::EdgeList);
        }
        if (CheckCollisionPointRec(mouse, tabMatrix) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            SetInputMode(GraphInputMode::AdjacencyMatrix);
        }
        if (CheckCollisionPointRec(mouse, tabAdj) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            SetInputMode(GraphInputMode::AdjacencyList);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || GetMouseWheelMove() != 0.0f) {
            if (CheckCollisionPointRec(mouse, kInputContentArea)) {
                inputScrollY -= GetMouseWheelMove() * 32.0f;
                ClampInputScroll();
            }
        }

        float maxScroll = GetInputMaxScroll();
        if (maxScroll > 0.0f) {
            float thumbH = kScrollTrack.height * (kInputClip.height / GetInputContentHeight());
            if (thumbH < 34.0f) thumbH = 34.0f;
            float trackRange = kScrollTrack.height - thumbH;
            float thumbY = kScrollTrack.y + trackRange * (inputScrollY / maxScroll);
            Rectangle thumb = {kScrollTrack.x, thumbY, kScrollTrack.width, thumbH};

            if (!inputScrollDragging && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, thumb)) {
                inputScrollDragging = true;
                inputScrollDragOffset = mouse.y - thumb.y;
            }
            if (inputScrollDragging) {
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    float nextThumbY = mouse.y - inputScrollDragOffset;
                    if (nextThumbY < kScrollTrack.y) nextThumbY = kScrollTrack.y;
                    if (nextThumbY > kScrollTrack.y + trackRange) nextThumbY = kScrollTrack.y + trackRange;
                    inputScrollY = trackRange > 0.0f ? ((nextThumbY - kScrollTrack.y) / trackRange) * maxScroll : 0.0f;
                    ClampInputScroll();
                } else {
                    inputScrollDragging = false;
                }
            }
        } else {
            inputScrollDragging = false;
            inputScrollY = 0.0f;
        }

        ClampInputScroll();
        if (inputMode == GraphInputMode::EdgeList) {
            LayoutEdgeListFields(edgeFromFields, edgeToFields, edgeWeightFields, inputScrollY);
            for (int i = 0; i < GRAPH_E; i++) {
                edgeFromFields[i].Update();
                edgeToFields[i].Update();
                edgeWeightFields[i].Update();
            }
        } else if (inputMode == GraphInputMode::AdjacencyMatrix) {
            LayoutMatrixFields(matrixFields, inputScrollY);
            for (int r = 0; r < GRAPH_N; r++) {
                for (int c = 0; c < GRAPH_N; c++) {
                    matrixFields[r][c].Update();
                }
            }
        } else {
            LayoutAdjListFields(adjListFields, inputScrollY);
            for (int i = 0; i < GRAPH_N; i++) {
                adjListFields[i].Update();
            }
        }

        bool anyInputFocused = false;
        if (inputMode == GraphInputMode::EdgeList) {
            for (int i = 0; i < GRAPH_E; i++) {
                anyInputFocused = anyInputFocused || edgeFromFields[i].focused || edgeToFields[i].focused || edgeWeightFields[i].focused;
            }
        } else if (inputMode == GraphInputMode::AdjacencyMatrix) {
            for (int r = 0; r < GRAPH_N; r++) {
                for (int c = 0; c < GRAPH_N; c++) {
                    anyInputFocused = anyInputFocused || matrixFields[r][c].focused;
                }
            }
        } else {
            for (int i = 0; i < GRAPH_N; i++) {
                anyInputFocused = anyInputFocused || adjListFields[i].focused;
            }
        }
        if (anyInputFocused) {
            ApplyInputToGraph(false);
        }

        bool clickedGraph = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse.x < 860.0f && mouse.y > 72.0f && mouse.y < 610.0f;
        if (clickedGraph) {
            int hitNode = -1;
            for (int i = 0; i < GRAPH_N; i++) {
                if (!nodes[i].visible) {
                    continue;
                }
                float dx = mouse.x - nodes[i].x;
                float dy = mouse.y - nodes[i].y;
                if (dx * dx + dy * dy <= (kNodeRadius + 4.0f) * (kNodeRadius + 4.0f)) {
                    hitNode = i;
                    break;
                }
            }

            if (hitNode >= 0) {
                selectionType = GraphSelectionType::Node;
                selectedIndex = hitNode;
            } else {
                int hitEdge = -1;
                for (int i = 0; i < static_cast<int>(edges.size()); i++) {
                    const auto& edge = edges[i];
                    if (!edge.visible || !nodes[edge.u].visible || !nodes[edge.v].visible) {
                        continue;
                    }
                    Vector2 a = {nodes[edge.u].x, nodes[edge.u].y};
                    Vector2 b = {nodes[edge.v].x, nodes[edge.v].y};
                    if (DistanceToSegment(mouse, a, b) < 9.0f) {
                        hitEdge = i;
                        break;
                    }
                }

                if (hitEdge >= 0) {
                    selectionType = GraphSelectionType::Edge;
                    selectedIndex = hitEdge;
                } else {
                    ClearSelection();
                }
            }
        }

        if (btnDelete.Update()) {
            DeleteSelected();
        }
        if (btnEdit.Update()) {
            OpenEditDialog();
        }
    } else {
        editField.Update();
        if (btnEditOk.Update() || IsKeyPressed(KEY_ENTER)) {
            ApplySelectedEdit();
        }
        if (btnEditCancel.Update() || IsKeyPressed(KEY_ESCAPE)) {
            editDialogOpen = false;
            editTarget = GraphEditTarget::None;
            ClearInputFocus();
        }
    }

    return Screen::MST;
}

void GraphScreen::Draw() const {
    ClearBackground(Pal::BG);

    DrawRectangleRec({0, 0, 1280, 72}, Pal::Surface);
    DrawLineEx({0, 72}, {1280, 72}, 1.0f, Pal::Border);
    DrawTextEx(fontBold, "Graph Input", {130, 20}, 28.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(
        fontRegular,
        "Left: graph preview. Right: choose input style (Edge List / Matrix / List).",
        {130, 52},
        13.0f,
        1.0f,
        Pal::TxtLight
    );
    btnBack.Draw();

    DrawLineEx({860, 72}, {860, 610}, 1.0f, Pal::Border);

    for (int i = 0; i < static_cast<int>(edges.size()); i++) {
        const GEdge& e = edges[i];
        if (!e.visible) {
            continue;
        }

        Vector2 a = {nodes[e.u].x, nodes[e.u].y};
        Vector2 b = {nodes[e.v].x, nodes[e.v].y};
        bool selected = selectionType == GraphSelectionType::Edge && selectedIndex == i;

        if (selected) {
            DrawLineEx(a, b, 4.8f, {255, 213, 79, 80});
        }
        DrawLineEx(a, b, selected ? 2.8f : 1.8f, selected ? Pal::NodeHL : Pal::EdgeColor);

        float mx = (a.x + b.x) * 0.5f + 6.0f;
        float my = (a.y + b.y) * 0.5f - 10.0f;
        char wbuf[16];
        std::snprintf(wbuf, sizeof(wbuf), "%d", e.w);
        DrawTextEx(fontBold, wbuf, {mx, my}, 14.0f, 1.0f, selected ? Pal::NodeHL : Pal::TxtMid);
    }

    for (int i = 0; i < GRAPH_N; i++) {
        const GNode& nd = nodes[i];
        if (!nd.visible) {
            continue;
        }

        bool selected = selectionType == GraphSelectionType::Node && selectedIndex == i;
        DrawCircleV({nd.x, nd.y}, kNodeRadius + 2.0f, selected ? Pal::NodeHL : Pal::NodeBorder);
        DrawCircleV({nd.x, nd.y}, kNodeRadius, Pal::NodeFill);

        Vector2 ts = MeasureTextEx(fontBold, nd.label.c_str(), 17.0f, 1.0f);
        DrawTextEx(fontBold, nd.label.c_str(), {nd.x - ts.x * 0.5f, nd.y - ts.y * 0.5f}, 17.0f, 1.0f, Pal::TxtDark);
    }

    DrawRectangleRec({872, 86, 388, 516}, Pal::Panel);
    DrawRectangleRoundedLines({872, 86, 388, 516}, 0.12f, 8, Pal::Border);

    DrawTextEx(fontBold, "Input Graph", {892, 102}, 18.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, "Choose one format, then type directly into the live table.", {892, 124}, 12.0f, 1.0f, Pal::TxtLight);

    Rectangle idx0 = {890, 138, 74, 26};
    Rectangle idx1 = {972, 138, 74, 26};
    DrawModeTab(idx0, "0", indexBase == 0);
    DrawModeTab(idx1, "1", indexBase == 1);

    DrawTextEx(fontBold, "Indexing", {1080, 145}, 13.0f, 1.0f, Pal::TxtMid);

    DrawModeTab(kTabEdge, "Edge List", inputMode == GraphInputMode::EdgeList);
    DrawModeTab(kTabMatrix, "Adjacency Matrix", inputMode == GraphInputMode::AdjacencyMatrix);
    DrawModeTab(kTabAdj, "Adjacency List", inputMode == GraphInputMode::AdjacencyList);

    DrawRectangleRounded(kInputContentArea, 0.12f, 8, Pal::Surface);
    DrawRectangleRoundedLines(kInputContentArea, 0.12f, 8, Pal::Border);

    float maxScroll = GetInputMaxScroll();
    float scrollY = inputScrollY;
    if (scrollY < 0.0f) scrollY = 0.0f;
    if (scrollY > maxScroll) scrollY = maxScroll;

    BeginScissorMode((int)kInputClip.x, (int)kInputClip.y, (int)kInputClip.width, (int)kInputClip.height);
    if (inputMode == GraphInputMode::EdgeList) {
        DrawFixedHeader("Edge List", "U, V, and weight for each edge.");
        DrawTextEx(fontBold, "STT", {kEdgeListSttX, 340}, 12.0f, 1.0f, Pal::TxtLight);
        DrawTextEx(fontBold, "U", {kEdgeListUX, 340}, 12.0f, 1.0f, Pal::TxtLight);
        DrawTextEx(fontBold, "V", {kEdgeListVX, 340}, 12.0f, 1.0f, Pal::TxtLight);
        DrawTextEx(fontBold, "W", {kEdgeListWX, 340}, 12.0f, 1.0f, Pal::TxtLight);

        for (int i = 0; i < GRAPH_E; i++) {
            char rowNo[8];
            std::snprintf(rowNo, sizeof(rowNo), "%02d", i + 1);
            DrawTextEx(fontRegular, rowNo, {kEdgeListSttX, kEdgeListRowY + i * kEdgeListRowGap - scrollY}, 12.0f, 1.0f, Pal::TxtMid);
            edgeFromFields[i].Draw();
            edgeToFields[i].Draw();
            edgeWeightFields[i].Draw();
        }
    } else if (inputMode == GraphInputMode::AdjacencyMatrix) {
        DrawFixedHeader("Adjacency Matrix", "Rows and columns use the selected index base.");

        for (int c = 0; c < GRAPH_N; c++) {
            char label[8];
            std::snprintf(label, sizeof(label), "%d", c + indexBase);
            DrawTextEx(fontBold, label, {kMatrixColX + c * kMatrixGap, 322.0f}, 12.0f, 1.0f, Pal::TxtLight);
        }

        for (int r = 0; r < GRAPH_N; r++) {
            char label[8];
            std::snprintf(label, sizeof(label), "%d", r + indexBase);
            DrawTextEx(fontBold, label, {905.0f, kMatrixRowY + r * kMatrixGap - scrollY}, 12.0f, 1.0f, Pal::TxtLight);
            for (int c = 0; c < GRAPH_N; c++) {
                matrixFields[r][c].Draw();
            }
        }
    } else {
        DrawFixedHeader("Adjacency List", "Each row: neighbor,weight pairs separated by spaces.");

        for (int i = 0; i < GRAPH_N; i++) {
            char label[8];
            std::snprintf(label, sizeof(label), "%d", i + indexBase);
            DrawTextEx(fontBold, label, {902.0f, kAdjRowY + i * kAdjRowGap - scrollY}, 13.0f, 1.0f, Pal::TxtMid);
            adjListFields[i].Draw();
        }
    }
    EndScissorMode();

    if (maxScroll > 0.0f) {
        float thumbH = kScrollTrack.height * (kInputClip.height / GetInputContentHeight());
        if (thumbH < 34.0f) thumbH = 34.0f;
        float trackRange = kScrollTrack.height - thumbH;
        float thumbY = kScrollTrack.y + trackRange * (scrollY / maxScroll);
        Rectangle thumb = {kScrollTrack.x, thumbY, kScrollTrack.width, thumbH};
        DrawRectangleRounded(kScrollTrack, 0.5f, 4, {72, 82, 106, 110});
        DrawRectangleRounded(thumb, 0.5f, 4, Pal::BtnPrimary);
    }

    DrawTextEx(fontRegular, "STT stays attached to each row while the headers remain fixed.", {892, 548}, 11.0f, 1.0f, Pal::TxtLight);

    DrawRectangleRec({0, 610, 1280, 110}, Pal::Panel);
    DrawLineEx({0, 610}, {1280, 610}, 1.0f, Pal::Border);

    btnDelete.Draw();
    btnEdit.Draw();

    if (selectionType == GraphSelectionType::Node && selectedIndex >= 0) {
        std::string line = "Selected node: " + nodes[selectedIndex].label;
        DrawTextEx(fontRegular, line.c_str(), {20, 645}, 13.0f, 1.0f, Pal::TxtMid);
    } else if (selectionType == GraphSelectionType::Edge && selectedIndex >= 0) {
        DrawTextEx(fontRegular, "Selected edge. Use Delete/Edit buttons below.", {20, 645}, 13.0f, 1.0f, Pal::TxtMid);
    } else {
        DrawTextEx(fontRegular, "Click a node or edge to select.", {20, 645}, 13.0f, 1.0f, Pal::TxtLight);
    }

    DrawTextEx(fontRegular, editDialogOpen ? "Edit dialog is open. Confirm or cancel in the popup." : "Click a node or edge, then Edit or Delete.", {20, 667}, 13.0f, 1.0f, Pal::TxtLight);

    if (msgTimer > 0.0f && !message.empty()) {
        Color c = msgColor;
        c.a = static_cast<unsigned char>((msgTimer < 0.5f ? msgTimer / 0.5f : 1.0f) * 220.0f);
        DrawTextEx(fontRegular, message.c_str(), {20, 690}, 14.2f, 1.0f, c);
    }

    if (editDialogOpen) {
        DrawRectangleRec({0, 0, 1280, 720}, {20, 28, 48, 110});
        DrawRectangleRounded({330, 240, 620, 210}, 0.08f, 10, Pal::Surface);
        DrawRectangleRoundedLines({330, 240, 620, 210}, 0.08f, 10, Pal::Border);

        DrawTextEx(fontBold, editTarget == GraphEditTarget::NodeLabel ? "Edit Node" : "Edit Edge", {360, 264}, 24.0f, 1.0f, Pal::TxtDark);
        DrawTextEx(fontRegular, editTarget == GraphEditTarget::NodeLabel ? "Enter a new node label and confirm." : "Enter a new edge weight and confirm.", {360, 296}, 13.0f, 1.0f, Pal::TxtLight);
        editField.Draw();
        btnEditOk.Draw();
        btnEditCancel.Draw();
    }
}
