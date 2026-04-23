#pragma once
#include "screen.h"
#include "button.h"
#include "input_field.h"
#include <string>
#include <vector>

struct GNode {
    float x, y;
    std::string label;
    bool visible;
};

struct GEdge {
    int u, v, w;
    bool visible;
    bool inMST;
    bool skipped;   // would create cycle
    bool highlighted;
};

enum class GraphInputMode {
    EdgeList,
    AdjacencyMatrix,
    AdjacencyList
};
enum class GraphSelectionType {
    None,
    Node,
    Edge
};
enum class GraphEditTarget {
    None,
    NodeLabel,
    EdgeWeight
};

class GraphScreen {
    static constexpr int GRAPH_N = 7;
    static constexpr int GRAPH_E = 11;

    GNode nodes[GRAPH_N];
    std::vector<GEdge> edges;

    GraphInputMode inputMode;
    int indexBase;

    GraphSelectionType selectionType;
    int selectedIndex;

    GraphEditTarget editTarget;
    bool editDialogOpen;

    int draggingNodeIndex;
    Vector2 draggingOffset;

    float inputScrollY;
    bool inputScrollDragging;
    float inputScrollDragOffset;

    Button btnBack;
    Button btnDelete;
    Button btnEdit;
    Button btnEditOk;
    Button btnEditCancel;
    Button btnKruskal;
    Button btnPrim;

    InputField edgeFromFields[GRAPH_E];
    InputField edgeToFields[GRAPH_E];
    InputField edgeWeightFields[GRAPH_E];
    InputField matrixFields[GRAPH_N][GRAPH_N];
    InputField adjListFields[GRAPH_N];
    InputField editField;

    std::string message;
    float msgTimer;
    Color msgColor;

    void ClearInputFocus();
    void ClearSelection();
    void SetInputMode(GraphInputMode mode);
    void SetIndexBase(int base);
    void SyncFieldsFromGraph(bool clearFocus = true);
    bool ApplyInputToGraph(bool showMessage);
    void OpenEditDialog();
    void ApplySelectedEdit();
    void DeleteSelected();
    float GetInputContentHeight() const;
    float GetInputMaxScroll() const;
    void ClampInputScroll();
    void SetMsg(const char* msg, 
                Color c = {46, 160, 67, 255}, 
                float dur = 3.0f);
    void RunKruskal();
    void RunPrim();

public:
    GraphScreen();
    Screen Update();
    void Draw() const;
};
