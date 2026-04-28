#include "graph_screen.h"
#include "init_file.h"
#include "colors.h"
#include "font.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <numeric>
#include <sstream>
#include <unordered_map>

namespace {
constexpr float kNodeRadius = 22.0f;
constexpr int kMaxGraphN = 26;
constexpr int kMaxGraphE = 50;

constexpr Rectangle kInputPanel       = {872, 86, 388, 516};
constexpr Rectangle kInputContentArea = {884, 218, 360, 350};
constexpr Rectangle kInputClip        = {892, 252, 334, 300};
constexpr Rectangle kScrollTrack      = {1236, 226, 8, 326};

constexpr float kEdgeListSttX    = 896.0f;
constexpr float kEdgeListUX      = 922.0f;
constexpr float kEdgeListVX      = 986.0f;
constexpr float kEdgeListWX      = 1050.0f;
constexpr float kEdgeListWeightW = 150.0f;
constexpr float kEdgeListHeaderY = 253.0f;
constexpr float kEdgeListRowY    = 275.0f;
constexpr float kEdgeListRowGap  = 22.0f;

constexpr float kMatrixColX    = 934.0f;
constexpr float kMatrixRowY    = 275.0f;
constexpr float kMatrixGap     = 30.0f;
constexpr float kMatrixHeaderY = 253.0f;

constexpr float kAdjRowY   = 257.0f;
constexpr float kAdjRowGap = 26.0f;

constexpr Rectangle kTabEdge   = {884, 94, 360, 34};
constexpr Rectangle kTabMatrix = {884, 136, 360, 34};
constexpr Rectangle kTabAdj    = {884, 178, 360, 34};
constexpr Rectangle kCodePanel = {876, 86, 372, 166};
constexpr float kMstStepDuration = 0.80f;

constexpr const char* kKruskalPseudo[] = {
    "sort(edges.begin(), edges.end());",
    "for (edge e : edges) {",
    "    if (find(e.u) != find(e.v)) {",
    "        mst.push_back(e); union_set(e.u, e.v);",
    "    } else mark_skipped(e);",
    "}"
};

constexpr const char* kPrimPseudo[] = {
    "visited[start] = true;",
    "while (mst.size() < n - 1) {",
    "    edge e = min_crossing_edge(visited);",
    "    mst.push_back(e); visited[newNode] = true;",
    "}"
};

// --- Parsers & Helpers ---
bool ParseIntStrict(const std::string& text, int* value) {
    if (text.empty()) return false;
    char* end = nullptr;
    long parsed = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0') return false;
    *value = static_cast<int>(parsed);
    return true;
}

bool ParseNodeRef(const std::string& text, const GNode* nodes, int n, int* outIdx) {
    if (ParseIntStrict(text, outIdx)) {
        return true;
    }
    for (int i = 0; i < n; i++) {
        if (nodes[i].visible && nodes[i].label == text) {
            *outIdx = i;
            return true;
        }
    }
    return false;
}

std::string FormatVisitOrder(const std::vector<int>& order, const GNode* nodes, int nodeCount) {
    std::string text;
    for (int idx : order) {
        if (idx < 0 || idx >= nodeCount || !nodes[idx].visible) continue;
        if (!text.empty()) text += " -> ";
        text += nodes[idx].label;
    }
    return text.empty() ? "-" : text;
}

void DrawModeTab(Rectangle rect, const char* label, bool active) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);

    Color fill = active ? Pal::BtnPrimary : Pal::Surface;
    if (hovered && !active) fill = Pal::PanelDark;
    else if (hovered && active) fill = Pal::BtnPrimHov;

    DrawRectangleRounded(rect, 0.22f, 8, fill);
    DrawRectangleRoundedLines(rect, 0.22f, 8, active ? Pal::BtnPrimary : Pal::Border);

    Vector2 ts = MeasureTextEx(fontBold, label, 15.0f, 1.0f);
    DrawTextEx(fontBold, label, 
        {rect.x + rect.width * 0.5f - ts.x * 0.5f, rect.y + rect.height * 0.5f - ts.y * 0.5f}, 
        15.0f, 1.0f, active ? WHITE : Pal::TxtDark);

}

void LayoutEdgeListFields(InputField* from, InputField* to, InputField* w, float scrollY, int count) {
    for (int i = 0; i < count; i++) {
        float y = kEdgeListRowY + i * kEdgeListRowGap - scrollY;
        from[i].rect = {kEdgeListUX, y, 56.0f, 20.0f};
        to[i].rect   = {kEdgeListVX, y, 56.0f, 20.0f};
        w[i].rect    = {kEdgeListWX, y, kEdgeListWeightW, 20.0f};
    }
}

void LayoutMatrixFields(InputField fields[kMaxGraphN][kMaxGraphN], float scrollY, int n) {
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            float x = kMatrixColX + c * kMatrixGap;
            float y = kMatrixRowY + r * kMatrixGap - scrollY;
            fields[r][c].rect = {x, y, 26.0f, 20.0f};
        }
    }
}

void LayoutAdjListFields(InputField* fields, float scrollY, int n) {
    for (int i = 0; i < n; i++) {
        float y = kAdjRowY + i * kAdjRowGap - scrollY;
        fields[i].rect = {930.0f, y, 270.0f, 20.0f};
    }
}

float DistanceToSegment(Vector2 point, Vector2 a, Vector2 b) {
    Vector2 ab = {b.x - a.x, b.y - a.y};
    Vector2 ap = {point.x - a.x, point.y - a.y};
    float ab2 = ab.x * ab.x + ab.y * ab.y;
    
    if (ab2 <= 0.0001f) {
        return std::sqrt(std::pow(point.x - a.x, 2) + std::pow(point.y - a.y, 2));
    }

    float t = std::clamp((ap.x * ab.x + ap.y * ab.y) / ab2, 0.0f, 1.0f);
    Vector2 c = {a.x + ab.x * t, a.y + ab.y * t};
    return std::sqrt(std::pow(point.x - c.x, 2) + std::pow(point.y - c.y, 2));
}

}
// --- Class Implementation ---

GraphScreen::GraphScreen()
    : inputMode(GraphInputMode::EdgeList), nodeCount(7),
      selectionType(GraphSelectionType::None), selectedIndex(-1),
      editTarget(GraphEditTarget::None), editDialogOpen(false),
      draggingNodeIndex(-1), draggingOffset({0.0f, 0.0f}),
      inputScrollY(0.0f), inputScrollDragging(false), inputScrollDragOffset(0.0f),
      btnBack({20, 20, 100, 36}, "< Back", Pal::BtnNeutral, Pal::BtnNeutHov),
            btnDelete({200, 642, 130, 40}, "Delete", Pal::BtnDanger, Pal::BtnDangHov),
    btnTableOk({992, 572, 136, 26}, "OK", Pal::BtnSuccess, Pal::BtnSuccHov),
            btnAddNode({350, 642, 130, 40}, "Add Node", Pal::BtnNeutral, Pal::BtnNeutHov),
            btnKruskal({500, 642, 130, 40}, "Kruskal", Pal::BtnSuccess, Pal::BtnSuccHov),
            btnPrim({650, 642, 130, 40}, "Prim", Pal::BtnPrimary, Pal::BtnPrimHov),
            btnLoadFile({800, 642, 130, 40}, "Load File", Pal::BtnNeutral, Pal::BtnNeutHov),
            btnToggleCode({1120, 18, 140, 36}, "Show Code", Pal::BtnNeutral, Pal::BtnNeutHov),
      mstCurrentStep(0), mstActive(false), mstStepTimer(0.0f),
            showPseudoCode(false), mstAlgoType(MSTAlgorithmType::None),
      btnEditOk({360, 470, 272, 40}, "OK", Pal::BtnSuccess, Pal::BtnSuccHov),
      btnEditCancel({648, 470, 272, 40}, "Cancel", Pal::BtnNeutral, Pal::BtnNeutHov),
      editField({360, 328, 560, 38}, "value", 24),
      msgTimer(0.0f), msgColor(Pal::TxtMid) 
{
    nodes[0] = {200, 200, "A", true};
    nodes[1] = {450, 140, "B", true};
    nodes[2] = {700, 200, "C", true};
    nodes[3] = {800, 400, "D", true};
    nodes[4] = {560, 480, "E", true};
    nodes[5] = {300, 450, "F", true};
    nodes[6] = {130, 370, "G", true};
    edges = {
        {0, 1, 7, true}, {0, 5, 9, true}, {0, 6, 14, true},
        {1, 2, 8, true}, {1, 5, 10, true}, {2, 3, 7, true},
        {2, 4, 2, true}, {3, 4, 6, true}, {4, 5, 11, true},
        {5, 6, 2, true}, {3, 6, 9, true}
    };

    // Initialize UI Fields
    for (int i = 0; i < MAX_GRAPH_E; i++) {
        float y = 337.0f + i * 25.0f;
        edgeFromFields[i]   = InputField({904.0f, y, 46.0f, 22.0f}, "u", 3);
        edgeToFields[i]     = InputField({960.0f, y, 46.0f, 22.0f}, "v", 3);
        edgeWeightFields[i] = InputField({1016.0f, y, 88.0f, 22.0f}, "w", 5);
    }

    for (int r = 0; r < MAX_GRAPH_N; r++) {
        float yAdj = 338.0f + r * 36.0f;
        adjListFields[r] = InputField({952.0f, yAdj, 240.0f, 24.0f}, "neighbor,weight", 64);
        for (int c = 0; c < MAX_GRAPH_N; c++) {
            float x = 929.0f + c * 33.0f;
            float y = 334.0f + r * 33.0f;
            matrixFields[r][c] = InputField({x, y, 28.0f, 22.0f}, "0", 4);
        }
    }

    SyncFieldsFromGraph();
    SetMsg("Choose an input format on the right.", Pal::TxtMid, 6.0f);
}

void GraphScreen::ClearInputFocus() {
    for (int i = 0; i < MAX_GRAPH_E; i++) {
        edgeFromFields[i].focused = edgeToFields[i].focused = edgeWeightFields[i].focused = false;
    }
    for (int r = 0; r < MAX_GRAPH_N; r++) {
        adjListFields[r].focused = false;
        for (int c = 0; c < MAX_GRAPH_N; c++) matrixFields[r][c].focused = false;
    }
    editField.focused = false;
}

void GraphScreen::ClearSelection() {
    selectionType = GraphSelectionType::None;
    selectedIndex = draggingNodeIndex = -1;
}

void GraphScreen::SetInputMode(GraphInputMode mode) {
    if (inputMode == mode) return;
    inputMode = mode;
    inputScrollY = 0.0f;
    ClearInputFocus();
    SyncFieldsFromGraph();
}

void GraphScreen::SyncFieldsFromGraph(bool clearFocus) {
    if (clearFocus) ClearInputFocus();

    int visibleEdgeCount = 0;
    for (int i = 0; i < static_cast<int>(edges.size()); i++) {
        edgeFromFields[i].Clear(); edgeToFields[i].Clear(); edgeWeightFields[i].Clear();
        if (edges[i].visible) {
            edgeFromFields[visibleEdgeCount].text   = nodes[edges[i].u].label;
            edgeToFields[visibleEdgeCount].text     = nodes[edges[i].v].label;
            edgeWeightFields[visibleEdgeCount].text = std::to_string(edges[i].w);
            visibleEdgeCount++;
        }
    }
    for (int i = visibleEdgeCount; i < MAX_GRAPH_E; i++) {
        edgeFromFields[i].Clear(); edgeToFields[i].Clear(); edgeWeightFields[i].Clear();
    }

    for (int r = 0; r < nodeCount; r++) {
        for (int c = 0; c < nodeCount; c++) matrixFields[r][c].text = "0";
    }

    std::vector<std::string> rows(nodeCount);
    for (const auto& edge : edges) {
        if (!edge.visible || edge.u >= nodeCount || edge.v >= nodeCount) continue;
        matrixFields[edge.u][edge.v].text = matrixFields[edge.v][edge.u].text = std::to_string(edge.w);
        if (!rows[edge.u].empty()) rows[edge.u] += ' ';
        rows[edge.u] += nodes[edge.v].label + "," + std::to_string(edge.w);
        if (!rows[edge.v].empty()) rows[edge.v] += ' ';
        rows[edge.v] += nodes[edge.u].label + "," + std::to_string(edge.w);
    }
    for (int i = 0; i < nodeCount; i++) adjListFields[i].text = rows[i];
}

bool GraphScreen::ApplyInputToGraph(bool showMessage) {
    std::vector<GEdge> nextEdges;
    std::unordered_map<int, std::string> renameMap;
    auto addEdge = [&](int u, int v, int w) {
        if (u >= 0 && v >= 0 && u < nodeCount && v < nodeCount && u != v) {
            nextEdges.push_back({u, v, w, true});
        }
    };

    switch (inputMode) {
        case GraphInputMode::EdgeList:
        {
            std::vector<int> visibleEdgeIndices;
            visibleEdgeIndices.reserve(edges.size());
            for (int i = 0; i < static_cast<int>(edges.size()) && static_cast<int>(visibleEdgeIndices.size()) < MAX_GRAPH_E; i++) {
                if (edges[i].visible) visibleEdgeIndices.push_back(i);
            }

            for (int i = 0; i < MAX_GRAPH_E; i++) {
                if (edgeFromFields[i].text.empty() && edgeToFields[i].text.empty() && edgeWeightFields[i].text.empty()) continue;

                int edgeIdx = (i < static_cast<int>(visibleEdgeIndices.size())) ? visibleEdgeIndices[i] : -1;
                int oldU = (edgeIdx >= 0) ? edges[edgeIdx].u : -1;
                int oldV = (edgeIdx >= 0) ? edges[edgeIdx].v : -1;
                bool hasUnknownNodeRef = false;

                auto resolveEndpoint = [&](const std::string& text, int oldNodeIdx, int* outIdx) {
                    if (ParseNodeRef(text, nodes, nodeCount, outIdx)) return true;
                    if (oldNodeIdx < 0 || oldNodeIdx >= nodeCount || text.empty()) {
                        hasUnknownNodeRef = true;
                        return false;
                    }

                    auto it = renameMap.find(oldNodeIdx);
                    if (it == renameMap.end()) renameMap[oldNodeIdx] = text;
                    else if (it->second != text) return false;

                    *outIdx = oldNodeIdx;
                    return true;
                };

                int u = 0, v = 0, w = 0;
                if (!resolveEndpoint(edgeFromFields[i].text, oldU, &u) ||
                    !resolveEndpoint(edgeToFields[i].text, oldV, &v) ||
                    !ParseIntStrict(edgeWeightFields[i].text, &w)) {
                    if (showMessage) {
                        if (hasUnknownNodeRef) SetMsg("Unknown node in edge list. Add node first, then press OK.", Pal::BtnDanger, 2.8f);
                        else SetMsg("Edge: invalid row. U/V must be node label/index or full rename, W must be a number.", Pal::BtnDanger, 2.8f);
                    }
                    return false;
                }
                addEdge(u, v, w);
            }

            for (const auto& rename : renameMap) {
                int oldNodeIdx = rename.first;
                const std::string& newLabel = rename.second;

                if (newLabel.empty()) {
                    if (showMessage) SetMsg("Node label cannot be empty.", Pal::BtnDanger, 2.5f);
                    return false;
                }

                for (int row = 0; row < MAX_GRAPH_E && row < static_cast<int>(visibleEdgeIndices.size()); row++) {
                    if (edgeFromFields[row].text.empty() && edgeToFields[row].text.empty() && edgeWeightFields[row].text.empty()) continue;
                    int edgeIdx = visibleEdgeIndices[row];

                    if (edges[edgeIdx].u == oldNodeIdx && edgeFromFields[row].text != newLabel) {
                        if (showMessage) SetMsg("Rename node: update all occurrences in the table, then press OK.", Pal::BtnDanger, 3.0f);
                        return false;
                    }
                    if (edges[edgeIdx].v == oldNodeIdx && edgeToFields[row].text != newLabel) {
                        if (showMessage) SetMsg("Rename node: update all occurrences in the table, then press OK.", Pal::BtnDanger, 3.0f);
                        return false;
                    }
                }
            }

            std::unordered_map<std::string, int> seenLabels;
            for (int i = 0; i < nodeCount; i++) {
                std::string label = nodes[i].label;
                auto it = renameMap.find(i);
                if (it != renameMap.end()) label = it->second;

                if (label.empty() || seenLabels.count(label)) {
                    if (showMessage) SetMsg("Node labels must be non-empty and unique.", Pal::BtnDanger, 2.8f);
                    return false;
                }
                seenLabels[label] = i;
            }

            break;
        }

        case GraphInputMode::AdjacencyMatrix:
            for (int r = 0; r < nodeCount; r++) {
                for (int c = r + 1; c < nodeCount; c++) {
                    int w = 0;
                    if (!ParseIntStrict(matrixFields[r][c].text, &w)) {
                        if (showMessage) SetMsg("Matrix values must be numbers.", Pal::BtnDanger, 2.5f);
                        return false;
                    }
                    if (w != 0) addEdge(r, c, w);
                }
            }
            break;

        case GraphInputMode::AdjacencyList:
            for (int r = 0; r < nodeCount; r++) {
                std::istringstream row(adjListFields[r].text);
                std::string token;
                while (row >> token) {
                    size_t sep = token.find_first_of(",:/-");
                    if (sep == std::string::npos || sep == 0 || sep + 1 >= token.size()) {
                        if (showMessage) SetMsg("Adjacency List: use label,weight (e.g. B,7).", Pal::BtnDanger, 2.5f);
                        return false;
                    }
                    int neighbor = 0, weight = 0;
                    if (!ParseNodeRef(token.substr(0, sep), nodes, nodeCount, &neighbor) ||
                        !ParseIntStrict(token.substr(sep + 1), &weight)) {
                        if (showMessage) SetMsg("Adjacency List: use label,weight (e.g. B,7).", Pal::BtnDanger, 2.5f);
                        return false;
                    }
                    if (neighbor > r) addEdge(r, neighbor, weight);
                }
            }
            break;
    }

    for (const auto& rename : renameMap) {
        if (rename.first >= 0 && rename.first < nodeCount) nodes[rename.first].label = rename.second;
    }

    edges = std::move(nextEdges);
    
    // Clean up selection if out of bounds
    if (selectionType == GraphSelectionType::Edge && selectedIndex >= static_cast<int>(edges.size())) ClearSelection();
    if (selectionType == GraphSelectionType::Node && (selectedIndex < 0 || selectedIndex >= nodeCount || !nodes[selectedIndex].visible)) ClearSelection();
    
    if (showMessage) SetMsg("Graph updated from input.", Pal::BtnSuccess, 2.0f);
    return true;
}

// --- Dialog Handlers ---
void GraphScreen::OpenAddNodeDialog() {
    if (nodeCount >= MAX_GRAPH_N) {
        SetMsg("Maximum 26 nodes reached.", Pal::BtnDanger, 2.5f);
        return;
    }
    editDialogOpen = true;
    editTarget = GraphEditTarget::AddNodeLabel;
    ClearInputFocus();
    editField.Clear();
    editField.focused = true;
}

void GraphScreen::ApplySelectedEdit() {
    if (!editDialogOpen) return;

    if (editTarget == GraphEditTarget::AddNodeLabel) {
        if (editField.text.empty()) { SetMsg("Node label cannot be empty.", Pal::BtnDanger, 2.5f); return; }
        AddNodeAtRandom(editField.text);
    }

    editDialogOpen = false;
    editTarget = GraphEditTarget::None;
    ClearInputFocus();
    SyncFieldsFromGraph(false);
}

void GraphScreen::DeleteSelected() {
    if (selectionType == GraphSelectionType::None || selectedIndex < 0) {
        SetMsg("Click a node or edge first.", Pal::BtnDanger, 2.5f);
        return;
    }
    if (selectionType == GraphSelectionType::Node) {
        nodes[selectedIndex].visible = false;
        for (auto& edge : edges) {
            if (edge.u == selectedIndex || edge.v == selectedIndex) edge.visible = false;
        }
        SetMsg("Node removed from preview.", Pal::BtnSuccess, 2.0f);
    } else {
        edges[selectedIndex].visible = false;
        SetMsg("Edge removed from preview.", Pal::BtnSuccess, 2.0f);
    }
    ClearSelection();
    SyncFieldsFromGraph(false);
}

// --- Scrolling & UI Helpers ---
float GraphScreen::GetInputContentHeight() const {
    switch (inputMode) {
        case GraphInputMode::EdgeList: return (kEdgeListRowY - kInputClip.y) + MAX_GRAPH_E * kEdgeListRowGap + 8.0f;
        case GraphInputMode::AdjacencyMatrix: return (kMatrixRowY - kInputClip.y) + std::min(nodeCount, 9) * kMatrixGap + 8.0f;
        case GraphInputMode::AdjacencyList: return (kAdjRowY - kInputClip.y) + nodeCount * kAdjRowGap + 8.0f;
    }
    return 400.0f;
}

float GraphScreen::GetInputMaxScroll() const {
    return std::max(0.0f, GetInputContentHeight() - kInputClip.height);
}

void GraphScreen::ClampInputScroll() {
    inputScrollY = std::clamp(inputScrollY, 0.0f, GetInputMaxScroll());
}

void GraphScreen::SetMsg(const char* msg, Color c, float dur) {
    message = msg; msgColor = c; msgTimer = dur;
}

void GraphScreen::AddNodeAtRandom(const std::string& label) {
    if (nodeCount >= MAX_GRAPH_N) { SetMsg("Maximum 26 nodes reached.", Pal::BtnDanger, 2.5f); return; }
    
    float minX = kNodeRadius + 8.0f, maxX = 860.0f - kNodeRadius - 8.0f;
    float minY = 72.0f + kNodeRadius + 8.0f, maxY = 610.0f - kNodeRadius - 8.0f;
    auto rand01 = []() { return static_cast<float>(GetRandomValue(0, 10000)) / 10000.0f; };

    float x = 0, y = 0, minDist2 = std::pow(kNodeRadius * 2.0f + 10.0f, 2);
    for (int attempt = 0; attempt < 32; ++attempt) {
        x = minX + rand01() * (maxX - minX);
        y = minY + rand01() * (maxY - minY);
        bool overlap = false;
        for (int i = 0; i < nodeCount; i++) {
            if (nodes[i].visible && (std::pow(nodes[i].x - x, 2) + std::pow(nodes[i].y - y, 2) < minDist2)) {
                overlap = true; break;
            }
        }
        if (!overlap) break;
    }

    nodes[nodeCount++] = {x, y, label, true};
    SyncFieldsFromGraph(false);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "Node %s added.", label.c_str());
    SetMsg(buf, Pal::BtnSuccess, 2.0f);
}

// UPDATE LOOP
// ============================================================================
Screen GraphScreen::Update() {
    float dt = GetFrameTime();
    UpdateMessages(dt);

    if (btnBack.Update() || IsKeyPressed(KEY_ESCAPE)) return Screen::Home;
    btnToggleCode.label = showPseudoCode ? "Hide Code" : "Show Code";
    if (btnToggleCode.Update()) showPseudoCode = !showPseudoCode;
    if (!editDialogOpen) {
        UpdateTabsAndScroll();
        UpdateInputFields();
        HandleGraphInteraction(GetMousePosition());
        HandleButtons();
    } else {
        HandleDialogUpdate();
    }
    UpdateMSTAnimation(dt);
    return Screen::MST;
}

void GraphScreen::UpdateMessages(float dt) {
    if (msgTimer > 0.0f) msgTimer = std::max(0.0f, msgTimer - dt);
}

void GraphScreen::UpdateTabsAndScroll() {
    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, kTabEdge) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) SetInputMode(GraphInputMode::EdgeList);
    if (CheckCollisionPointRec(mouse, kTabMatrix) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) SetInputMode(GraphInputMode::AdjacencyMatrix);
    if (CheckCollisionPointRec(mouse, kTabAdj) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) SetInputMode(GraphInputMode::AdjacencyList);

    if (CheckCollisionPointRec(mouse, kInputContentArea) && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || GetMouseWheelMove() != 0.0f)) {
        inputScrollY -= GetMouseWheelMove() * 32.0f;
    }
    float maxScroll = GetInputMaxScroll();
    if (maxScroll > 0.0f) {
        float thumbH = std::max(34.0f, kScrollTrack.height * (kInputClip.height / GetInputContentHeight()));
        float trackRange = kScrollTrack.height - thumbH;
        Rectangle thumb = {kScrollTrack.x, kScrollTrack.y + trackRange * (inputScrollY / maxScroll), kScrollTrack.width, thumbH};

        if (!inputScrollDragging && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, thumb)) {
            inputScrollDragging = true;
            inputScrollDragOffset = mouse.y - thumb.y;
        }
        if (inputScrollDragging) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                float nextThumbY = std::clamp(mouse.y - inputScrollDragOffset, kScrollTrack.y, kScrollTrack.y + trackRange);
                inputScrollY = trackRange > 0.0f ? ((nextThumbY - kScrollTrack.y) / trackRange) * maxScroll : 0.0f;
            } else {
                inputScrollDragging = false;
            }
        }
    } else {
        inputScrollDragging = false; inputScrollY = 0.0f;
    }
    ClampInputScroll();
}

void GraphScreen::UpdateInputFields() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        bool clickedInsideClip = CheckCollisionPointRec(mouse, kInputClip);
        bool focusedOne = false;

        ClearInputFocus();

        auto tryFocus = [&](InputField& field) {
            if (focusedOne) return;
            if (clickedInsideClip && CheckCollisionPointRec(mouse, field.rect)) {
                field.focused = true;
                focusedOne = true;
            }
        };
        if (inputMode == GraphInputMode::EdgeList) {
            for (int i = 0; i < MAX_GRAPH_E; i++) {
                tryFocus(edgeFromFields[i]);
                tryFocus(edgeToFields[i]);
                tryFocus(edgeWeightFields[i]);
            }
        } else if (inputMode == GraphInputMode::AdjacencyMatrix) {
            int matrixDisplayN = std::min(nodeCount, 9);
            for (int r = 0; r < matrixDisplayN; r++) {
                for (int c = 0; c < matrixDisplayN; c++) {
                    tryFocus(matrixFields[r][c]);
                }
            }
        } else {
            for (int i = 0; i < nodeCount; i++) tryFocus(adjListFields[i]);
        }
    }

    int matrixDisplayN = std::min(nodeCount, 9);
    if (inputMode == GraphInputMode::EdgeList) {
        LayoutEdgeListFields(edgeFromFields, edgeToFields, edgeWeightFields, inputScrollY, MAX_GRAPH_E);
        for (int i = 0; i < MAX_GRAPH_E; i++) {
            edgeFromFields[i].Update(); edgeToFields[i].Update(); edgeWeightFields[i].Update();
        }
    } else if (inputMode == GraphInputMode::AdjacencyMatrix) {
        LayoutMatrixFields(matrixFields, inputScrollY, matrixDisplayN);
        for (int r = 0; r < matrixDisplayN; r++) {
            for (int c = 0; c < matrixDisplayN; c++) {
                matrixFields[r][c].Update();
            }
        }
    } else {
        LayoutAdjListFields(adjListFields, inputScrollY, nodeCount);
        for (int i = 0; i < nodeCount; i++) {
            adjListFields[i].Update();
        }
    }
}

void GraphScreen::HandleGraphInteraction(Vector2 mouse) {
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) draggingNodeIndex = -1;

    bool insideGraph = mouse.x < 860.0f && mouse.y > 72.0f && mouse.y < 610.0f;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && insideGraph) {
        int hitNode = -1;
        for (int i = 0; i < nodeCount; i++) {
            if (nodes[i].visible && std::pow(mouse.x - nodes[i].x, 2) + std::pow(mouse.y - nodes[i].y, 2) <= std::pow(kNodeRadius + 4.0f, 2)) {
                hitNode = i; break;
            }
        }
        if (hitNode >= 0) {
            selectionType = GraphSelectionType::Node; selectedIndex = draggingNodeIndex = hitNode;
            draggingOffset = {nodes[hitNode].x - mouse.x, nodes[hitNode].y - mouse.y};
        } else {
            int hitEdge = -1;
            for (int i = 0; i < static_cast<int>(edges.size()); i++) {
                if (edges[i].visible && nodes[edges[i].u].visible && nodes[edges[i].v].visible) {
                    if (DistanceToSegment(mouse, {nodes[edges[i].u].x, nodes[edges[i].u].y}, {nodes[edges[i].v].x, nodes[edges[i].v].y}) < 9.0f) {
                        hitEdge = i; break;
                    }
                }
            }
            selectionType = hitEdge >= 0 ? GraphSelectionType::Edge : GraphSelectionType::None;
            selectedIndex = hitEdge; draggingNodeIndex = -1;
        }
    }
    if (draggingNodeIndex >= 0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && draggingNodeIndex < nodeCount) {
        nodes[draggingNodeIndex].x = std::clamp(mouse.x + draggingOffset.x, kNodeRadius + 8.0f, 860.0f - kNodeRadius - 8.0f);
        nodes[draggingNodeIndex].y = std::clamp(mouse.y + draggingOffset.y, 72.0f + kNodeRadius + 8.0f, 610.0f - kNodeRadius - 8.0f);
    }
}

void GraphScreen::HandleButtons() {
    if (btnTableOk.Update()) {
        if (ApplyInputToGraph(true)) SyncFieldsFromGraph(false);
    }

    if (btnDelete.Update()) DeleteSelected();
    if (btnAddNode.Update()) OpenAddNodeDialog();
    
    if (btnKruskal.Update()) { mstSteps.clear(); mstVisitOrder.clear(); mstActive = false; RunKruskal(); }
    if (btnPrim.Update())    { mstSteps.clear(); mstVisitOrder.clear(); mstActive = false; RunPrim(); }
    if (btnLoadFile.Update()) LoadGraphFromFile();
}

void GraphScreen::HandleDialogUpdate() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        editField.focused     = CheckCollisionPointRec(mouse, editField.rect);
    }

    if (editField.focused) editField.UpdateFocused();

    if (btnEditOk.Update() || IsKeyPressed(KEY_ENTER)) ApplySelectedEdit();
    if (btnEditCancel.Update() || IsKeyPressed(KEY_ESCAPE)) {
        editDialogOpen = false; editTarget = GraphEditTarget::None; ClearInputFocus();
    }
}

void GraphScreen::UpdateMSTAnimation(float dt) {
    if (mstActive && mstCurrentStep < (int)mstSteps.size()) {
        mstStepTimer -= dt;
        if (mstStepTimer <= 0.0f) {
            mstCurrentStep++;
            if (mstCurrentStep < (int)mstSteps.size()) {
                mstStepTimer = kMstStepDuration;
            }
        }
    }
}

void GraphScreen::LoadGraphFromFile() {
    std::vector<std::string> lines = InitFile::loadLines("data.txt");
    if (lines.empty()) { SetMsg("Failed to open file or empty.", Pal::BtnDanger, 2.5f); return; }

    struct LoadedEdge { std::string u, v; int w; };
    std::vector<std::string> labels;
    std::unordered_map<std::string, int> labelToIndex;
    std::vector<LoadedEdge> loadedEdges;

    int expectedNodeCount = -1, expectedEdgeCount = -1, parsedNodeCount = 0, parsedEdgeCount = 0, stage = 0;

    auto ensureNode = [&](const std::string& label) {
        if (labelToIndex.count(label)) return labelToIndex[label];
        int idx = labels.size();
        labels.push_back(label);
        return labelToIndex[label] = idx;
    };

    for (const auto& line : lines) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
        if (tokens.empty()) continue;

        if (stage == 0) {
            if (tokens.size() != 2 || !ParseIntStrict(tokens[0], &expectedNodeCount) || !ParseIntStrict(tokens[1], &expectedEdgeCount) || expectedNodeCount <= 0 || expectedEdgeCount < 0) {
                SetMsg("Invalid header: nodeCount edgeCount", Pal::BtnDanger, 2.5f); return;
            }
            stage = 1; continue;
        }
        if (stage == 1) {
            if (tokens.size() != 1) { SetMsg("Vertex names must be one per line.", Pal::BtnDanger, 2.5f); return; }
            ensureNode(tokens[0]);
            if (++parsedNodeCount >= expectedNodeCount) stage = 2;
            continue;
        }
        if (stage == 2) {
            int weight;
            if (tokens.size() != 3 || !ParseIntStrict(tokens[2], &weight)) { SetMsg("Edge format: from to weight", Pal::BtnDanger, 2.5f); return; }
            ensureNode(tokens[0]); ensureNode(tokens[1]);
            loadedEdges.push_back({tokens[0], tokens[1], weight});
            parsedEdgeCount++;
        }
    }

    if (expectedNodeCount <= 0 || parsedNodeCount != expectedNodeCount || parsedEdgeCount != expectedEdgeCount) {
        SetMsg("File data mismatch.", Pal::BtnDanger, 2.5f); return;
    }

    nodeCount = std::min(static_cast<int>(labels.size()), MAX_GRAPH_N);
    for (int i = 0; i < MAX_GRAPH_N; i++) {
        if (i < nodeCount) { nodes[i].label = labels[i]; nodes[i].visible = true; }
        else nodes[i].visible = false;
    }

    edges.clear();
    for (const auto& edge : loadedEdges) {
        if (!labelToIndex.count(edge.u) || !labelToIndex.count(edge.v)) continue;
        int u = labelToIndex[edge.u], v = labelToIndex[edge.v];
        if (u >= 0 && v >= 0 && u < nodeCount && v < nodeCount && u != v) edges.push_back({u, v, edge.w, true});
    }

    const float cX = 430.0f, cY = 342.0f, rX = 260.0f, rY = 180.0f, startAng = -1.5707963f;
    for (int i = 0; i < nodeCount; i++) {
        float angle = startAng + (2.0f * PI * i) / std::max(nodeCount, 1);
        nodes[i].x = cX + std::cos(angle) * rX;
        nodes[i].y = cY + std::sin(angle) * rY;
    }

    mstSteps.clear(); mstActive = false; mstCurrentStep = 0; mstStepTimer = 0.0f;
    ClearSelection(); ClearInputFocus(); SyncFieldsFromGraph(false);

    char buf[128];
    std::snprintf(buf, sizeof(buf), "Loaded %d nodes and %d edges.", nodeCount, (int)edges.size());
    SetMsg(buf, Pal::BtnSuccess, 3.0f);
}

// ============================================================================
// DRAW LOOP
// ============================================================================
void GraphScreen::Draw() const {
    ClearBackground(Pal::BG);
    DrawHeader();
    DrawGraphView();
    DrawInputPanel();
    DrawPseudoCodePanel();
    DrawBottomArea();
    if (editDialogOpen) DrawEditDialog();
}

void GraphScreen::DrawHeader() const {
    DrawRectangleRec({0, 0, 1280, 72}, Pal::Surface);
    DrawLineEx({0, 72}, {1280, 72}, 1.0f, Pal::Border);
    DrawTextEx(fontBold, "Minimum Spanning Tree", {130, 20}, 28.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, "Left: graph preview. Right: choose input style and load a graph file.", {130, 52}, 13.0f, 1.0f, Pal::TxtLight);
    btnBack.Draw();
    btnToggleCode.Draw();
    DrawLineEx({860, 72}, {860, 610}, 1.0f, Pal::Border);
}

void GraphScreen::DrawGraphView() const {
    std::vector<int> edgeState(edges.size(), 0); // 0: untouched, 1: added, -1: skipped
    int currentEdge = -1;
    if (mstActive) {
        for (int s = 0; s < mstCurrentStep && s < (int)mstSteps.size(); s++) {
            const MSTStep& step = mstSteps[s];
            if (step.edgeIdx >= 0 && step.edgeIdx < (int)edges.size() && step.marksDecision) {
                edgeState[step.edgeIdx] = step.added ? 1 : -1;
            }
        }
        if (mstCurrentStep >= 0 && mstCurrentStep < (int)mstSteps.size()) {
            currentEdge = mstSteps[mstCurrentStep].edgeIdx;
        }
    }

    // Draw Edges
    for (int i = 0; i < static_cast<int>(edges.size()); i++) {
        const GEdge& e = edges[i];
        if (!e.visible) continue;

        Vector2 a = {nodes[e.u].x, nodes[e.u].y}, b = {nodes[e.v].x, nodes[e.v].y};
        bool selected = (selectionType == GraphSelectionType::Edge && selectedIndex == i);
        Color lineColor = Pal::EdgeColor;
        float lineW = 1.8f;
        bool isNext = false;

        if (mstActive) {
            if (edgeState[i] == 1) {
                lineColor = Color{46, 180, 90, 255};
                lineW = 3.6f;
            } else if (edgeState[i] == -1) {
                lineColor = Color{160, 60, 60, 180};
                lineW = 1.4f;
            } else if (i == currentEdge) {
                lineColor = {255, 200, 50, 255}; lineW = 3.0f; isNext = true;
            } else {
                lineColor = {100, 110, 140, 120}; lineW = 1.4f;
            }
        }

        if (selected) DrawLineEx(a, b, lineW + 3.6f, {255, 213, 79, 80});
        else if (isNext) DrawLineEx(a, b, lineW + 3.0f, {255, 200, 50, 55});
        
        DrawLineEx(a, b, lineW, selected ? Pal::NodeHL : lineColor);

        char wbuf[16]; std::snprintf(wbuf, sizeof(wbuf), "%d", e.w);
        Color labelColor = selected ? Pal::NodeHL : (mstActive ? lineColor : Pal::TxtMid);
        DrawTextEx(fontBold, wbuf, {(a.x + b.x) * 0.5f + 6.0f, (a.y + b.y) * 0.5f - 10.0f}, 14.0f, 1.0f, labelColor);
    }

    // Draw Nodes
    for (int i = 0; i < nodeCount; i++) {
        if (!nodes[i].visible) continue;
        bool selected = (selectionType == GraphSelectionType::Node && selectedIndex == i);
        DrawCircleV({nodes[i].x, nodes[i].y}, kNodeRadius + 2.0f, selected ? Pal::NodeHL : Pal::NodeBorder);
        DrawCircleV({nodes[i].x, nodes[i].y}, kNodeRadius, Pal::NodeFill);
        Vector2 ts = MeasureTextEx(fontBold, nodes[i].label.c_str(), 17.0f, 1.0f);
        DrawTextEx(fontBold, nodes[i].label.c_str(), {nodes[i].x - ts.x * 0.5f, nodes[i].y - ts.y * 0.5f}, 17.0f, 1.0f, Pal::TxtDark);
    }

    // Draw MST Box
    if (mstActive) {
        int currentSum = (mstCurrentStep > 0) ? mstSteps[mstCurrentStep - 1].cumulativeWeight : 0;
        char sumBuf[32]; std::snprintf(sumBuf, sizeof(sumBuf), "S = %d", currentSum);
        DrawRectangleRounded({12, 542, 270, 58}, 0.18f, 6, Pal::Surface);
        DrawRectangleRoundedLines({12, 542, 270, 58}, 0.18f, 6, Pal::Border);
        DrawTextEx(fontRegular, "MST Weight", {20, 548}, 11.0f, 1.0f, Pal::TxtLight);
        DrawTextEx(fontBold, sumBuf, {20, 564}, 19.0f, 1.0f, {46, 180, 90, 255});
        DrawTextEx(fontRegular, FormatVisitOrder(mstVisitOrder, nodes, nodeCount).c_str(), {88, 566}, 12.0f, 1.0f, Pal::TxtDark);
    }

}

int GraphScreen::GetPseudoCodeHighlightLine() const {
    if (mstAlgoType == MSTAlgorithmType::None) return -1;
    if (mstSteps.empty()) return 0;
    if (!mstActive) return mstSteps[0].pseudoLine;

    int idx = std::clamp(mstCurrentStep, 0, (int)mstSteps.size() - 1);
    return mstSteps[idx].pseudoLine;
}

void GraphScreen::DrawPseudoCodePanel() const {
    if (!showPseudoCode || mstAlgoType == MSTAlgorithmType::None) return;
    DrawRectangleRounded(kCodePanel, 0.06f, 8, Color{34, 43, 67, 255});
    DrawRectangleRoundedLines(kCodePanel, 0.06f, 8, Color{78, 93, 124, 255});
    const bool isKruskal = (mstAlgoType == MSTAlgorithmType::Kruskal);
    const char* title = isKruskal ? "Kruskal" : "Prim";
    DrawTextEx(fontBold, title, {kCodePanel.x + 12.0f, kCodePanel.y + 10.0f}, 14.0f, 1.0f, Color{197, 209, 228, 255});
    const int highlightLine = GetPseudoCodeHighlightLine();
    const int lineCount = isKruskal ? 6 : 5;
    const float baseY = kCodePanel.y + 36.0f;
    const float lineGap = 24.0f;

    for (int i = 0; i < lineCount; i++) {
        const char* lineText = isKruskal ? kKruskalPseudo[i] : kPrimPseudo[i];
        const bool active = (i == highlightLine);
        if (active) {
            DrawRectangleRounded(
                {kCodePanel.x + 8.0f, baseY + i * lineGap - 3.0f, kCodePanel.width - 16.0f, 22.0f},
                0.12f, 6, Color{47, 88, 67, 235});
        }
        DrawTextEx(fontRegular, lineText, {kCodePanel.x + 16.0f, baseY + i * lineGap}, 13.0f, 1.0f,
            active ? Color{184, 250, 202, 255} : Color{166, 178, 203, 255});
    }
}

void GraphScreen::DrawInputPanel() const {
    DrawRectangleRec(kInputPanel, Pal::Panel);
    DrawRectangleRoundedLines(kInputPanel, 0.12f, 8, Pal::Border);

    DrawModeTab(kTabEdge, "Edge List", inputMode == GraphInputMode::EdgeList);
    DrawModeTab(kTabMatrix, "Adjacency Matrix", inputMode == GraphInputMode::AdjacencyMatrix);
    DrawModeTab(kTabAdj, "Adjacency List", inputMode == GraphInputMode::AdjacencyList);

    DrawRectangleRounded(kInputContentArea, 0.12f, 8, Pal::Surface);
    DrawRectangleRoundedLines(kInputContentArea, 0.12f, 8, Pal::Border);

    float scrollY = std::clamp(inputScrollY, 0.0f, GetInputMaxScroll());
    BeginScissorMode((int)kInputClip.x, (int)kInputClip.y, (int)kInputClip.width, (int)kInputClip.height);

    if (inputMode == GraphInputMode::EdgeList) {
        DrawTextEx(fontBold, "No.", {kEdgeListSttX, kEdgeListHeaderY}, 11.0f, 1.0f, Pal::TxtLight);
        DrawTextEx(fontBold, "U", {kEdgeListUX, kEdgeListHeaderY}, 11.0f, 1.0f, Pal::TxtLight);
        DrawTextEx(fontBold, "V", {kEdgeListVX, kEdgeListHeaderY}, 11.0f, 1.0f, Pal::TxtLight);
        DrawTextEx(fontBold, "W", {kEdgeListWX, kEdgeListHeaderY}, 11.0f, 1.0f, Pal::TxtLight);
        for (int i = 0; i < MAX_GRAPH_E; i++) {
            char rowNo[8]; std::snprintf(rowNo, sizeof(rowNo), "%02d", i + 1);
            DrawTextEx(fontRegular, rowNo, {kEdgeListSttX, kEdgeListRowY + i * kEdgeListRowGap - scrollY + 2.0f}, 11.0f, 1.0f, Pal::TxtMid);
            edgeFromFields[i].Draw(); edgeToFields[i].Draw(); edgeWeightFields[i].Draw();
        }
    } else if (inputMode == GraphInputMode::AdjacencyMatrix) {
        int limit = std::min(nodeCount, 9);
        for (int c = 0; c < limit; c++) DrawTextEx(fontBold, nodes[c].label.c_str(), {kMatrixColX + c * kMatrixGap + 7.0f, kMatrixHeaderY}, 11.0f, 1.0f, Pal::TxtLight);
        for (int r = 0; r < limit; r++) {
            DrawTextEx(fontBold, nodes[r].label.c_str(), {910.0f, kMatrixRowY + r * kMatrixGap - scrollY + 3.0f}, 11.0f, 1.0f, Pal::TxtLight);
            for (int c = 0; c < limit; c++) matrixFields[r][c].Draw();
        }
    } else {
        for (int i = 0; i < nodeCount; i++) {
            DrawTextEx(fontBold, nodes[i].label.c_str(), {902.0f, kAdjRowY + i * kAdjRowGap - scrollY + 2.0f}, 11.0f, 1.0f, Pal::TxtMid);
            adjListFields[i].Draw();
        }
    }
    EndScissorMode();

    if (GetInputMaxScroll() > 0.0f) {
        float thumbH = std::max(34.0f, kScrollTrack.height * (kInputClip.height / GetInputContentHeight()));
        Rectangle thumb = {kScrollTrack.x, kScrollTrack.y + (kScrollTrack.height - thumbH) * (scrollY / GetInputMaxScroll()), kScrollTrack.width, thumbH};
        DrawRectangleRounded(kScrollTrack, 0.5f, 4, {72, 82, 106, 110});
        DrawRectangleRounded(thumb, 0.5f, 4, Pal::BtnPrimary);
    }
    btnTableOk.Draw();
}

void GraphScreen::DrawBottomArea() const {
    DrawRectangleRec({0, 610, 1280, 110}, Pal::Panel);
    DrawLineEx({0, 610}, {1280, 610}, 1.0f, Pal::Border);

    btnDelete.Draw(); btnAddNode.Draw();
    btnKruskal.Draw(); btnPrim.Draw(); btnLoadFile.Draw();

    if (mstActive) {
        char stepBuf[48]; std::snprintf(stepBuf, sizeof(stepBuf), "Step %d / %d", mstCurrentStep, (int)mstSteps.size());
        DrawTextEx(fontRegular, stepBuf, {20, 694}, 13.0f, 1.0f, Pal::TxtMid);
    }
    if (msgTimer > 0.0f && !message.empty()) {
        Color c = msgColor;
        c.a = static_cast<unsigned char>((msgTimer < 0.5f ? msgTimer / 0.5f : 1.0f) * 220.0f);
        DrawTextEx(fontRegular, message.c_str(), {20, 702}, 14.2f, 1.0f, c);
    }
}

void GraphScreen::DrawEditDialog() const {
    DrawRectangleRec({0, 0, 1280, 720}, {20, 28, 48, 110});
    DrawRectangleRounded({300, 200, 680, 330}, 0.08f, 10, Pal::Surface);
    DrawRectangleRoundedLines({300, 200, 680, 330}, 0.08f, 10, Pal::Border);

    const char* dlgTitle = "Add Node";
    const char* dlgHint  = "Enter a label for the new node.";
    
    DrawTextEx(fontBold, dlgTitle, {360, 258}, 22.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, dlgHint, {360, 284}, 13.0f, 1.0f, Pal::TxtLight);
    DrawLineEx({360, 448}, {920, 448}, 1.0f, Pal::Border);

    editField.Draw();
    btnEditOk.Draw(); btnEditCancel.Draw();
}

// ALGORITHMS
// ============================================================================
void GraphScreen::RunKruskal() {
    mstVisitOrder.clear();

    std::vector<int> order;
    for (int i = 0; i < (int)edges.size(); i++) if (edges[i].visible) order.push_back(i);
    std::sort(order.begin(), order.end(), [&](int a, int b) { return edges[a].w < edges[b].w; });

    std::vector<int> parent(nodeCount);
    std::iota(parent.begin(), parent.end(), 0);
    std::function<int(int)> find = [&](int x) -> int { return parent[x] == x ? x : parent[x] = find(parent[x]); };

    mstSteps.clear();
    int cumWeight = 0;
    mstSteps.push_back({-1, false, cumWeight, 0, false});
    for (int idx : order) {
        const GEdge& e = edges[idx];
        if (!nodes[e.u].visible || !nodes[e.v].visible) continue;
        mstSteps.push_back({idx, false, cumWeight, 1, false});
        int pu = find(e.u), pv = find(e.v);
        mstSteps.push_back({idx, false, cumWeight, 2, false});
        bool added = (pu != pv);
        if (added) {
            parent[pu] = pv;
            cumWeight += e.w;
            if (std::find(mstVisitOrder.begin(), mstVisitOrder.end(), e.u) == mstVisitOrder.end()) mstVisitOrder.push_back(e.u);
            if (std::find(mstVisitOrder.begin(), mstVisitOrder.end(), e.v) == mstVisitOrder.end()) mstVisitOrder.push_back(e.v);
            mstSteps.push_back({idx, true, cumWeight, 3, true});
        } else {
            mstSteps.push_back({idx, false, cumWeight, 4, true});
        }
    }
    mstSteps.push_back({-1, false, cumWeight, 5, false});

    mstCurrentStep = 0; mstActive = true; mstStepTimer = kMstStepDuration;
    mstAlgoType = MSTAlgorithmType::Kruskal;
    SetMsg("Kruskal: running step by step...", Pal::TxtMid, 3.0f);
}

void GraphScreen::RunPrim() {
    mstVisitOrder.clear();

    int start = -1, visibleCount = 0;
    for (int i = 0; i < nodeCount; i++) {
        if (nodes[i].visible) { if (start == -1) start = i; visibleCount++; }
    }
    if (start == -1) { SetMsg("No visible nodes.", {200, 60, 60, 255}, 3.0f); return; }

    mstAlgoType = MSTAlgorithmType::Prim;

    std::vector<bool> inTree(nodeCount, false);
    inTree[start] = true;
    mstVisitOrder.push_back(start);
    mstSteps.clear();

    int cumWeight = 0, added = 0;
    mstSteps.push_back({-1, false, cumWeight, 0, false});
    while (added < visibleCount - 1) {
        mstSteps.push_back({-1, false, cumWeight, 1, false});
        int bestIdx = -1, bestW = INT_MAX;
        for (int i = 0; i < (int)edges.size(); i++) {
            const GEdge& e = edges[i];
            if (!e.visible || !nodes[e.u].visible || !nodes[e.v].visible || inTree[e.u] == inTree[e.v]) continue;
            if (e.w < bestW) { bestW = e.w; bestIdx = i; }
        }
        if (bestIdx >= 0) {
            mstSteps.push_back({bestIdx, false, cumWeight, 2, false});
        }
        if (bestIdx == -1) break;
        int u = edges[bestIdx].u, v = edges[bestIdx].v;
        int nextNode = inTree[u] ? v : u;
        inTree[u] = inTree[v] = true;
        if (nextNode >= 0 && nextNode < nodeCount) mstVisitOrder.push_back(nextNode);

        cumWeight += edges[bestIdx].w;
        mstSteps.push_back({bestIdx, true, cumWeight, 3, true});
        added++;
    }
    mstSteps.push_back({-1, false, cumWeight, 4, false});

    mstCurrentStep = 0; mstActive = true; mstStepTimer = kMstStepDuration;
    SetMsg("Prim: running step by step...", Pal::TxtMid, 3.0f);
}