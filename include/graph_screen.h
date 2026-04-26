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

struct MSTStep {
    int edgeIdx;
    bool added;
    int cumulativeWeight;
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
    EdgeWeight,
    AddEdge,
    AddNodeLabel
};

class GraphScreen {
    static constexpr int MAX_GRAPH_N = 26;
    static constexpr int MAX_GRAPH_E = 50;

    GNode nodes[MAX_GRAPH_N];
    std::vector<GEdge> edges;
    int nodeCount;

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
    Button btnChange;
    Button btnAddNode;
    Button btnAddEdge;
    Button btnEditOk;
    Button btnEditCancel;
    Button btnKruskal;
    Button btnPrim;
    Button btnLoadFile;
    std::vector<MSTStep> mstSteps;
    int mstCurrentStep;
    bool mstActive;
    float mstStepTimer;

    InputField edgeFromFields[MAX_GRAPH_E];
    InputField edgeToFields[MAX_GRAPH_E];
    InputField edgeWeightFields[MAX_GRAPH_E];
    InputField matrixFields[MAX_GRAPH_N][MAX_GRAPH_N];
    InputField adjListFields[MAX_GRAPH_N];
    InputField editField;
    InputField editFromField;
    InputField editToField;
    InputField filePathField;

    std::string message;
    float msgTimer;
    Color msgColor;

    void ClearInputFocus();
    void ClearSelection();
    void SetInputMode(GraphInputMode mode);
    void SetIndexBase(int base);
    void SyncFieldsFromGraph(bool clearFocus = true);
    bool ApplyInputToGraph(bool showMessage);
    void OpenChangeDialog();
    void OpenAddNodeDialog();
    void OpenAddEdgeDialog();
    void ApplySelectedEdit();
    void DeleteSelected();
    void AddNodeAtRandom(const std::string& label);
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
