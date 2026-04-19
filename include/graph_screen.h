#pragma once
#include "screen.h"
#include "button.h"
#include "input_field.h"
#include <string>

struct GNode {
    float x, y;
    const char* label;
};

struct GEdge {
    int u, v, w;
    bool inMST;
    bool skipped;   // would create cycle
    bool highlighted;
};

enum class GraphInputMode {
    EdgeList,
    AdjacencyMatrix,
    AdjacencyList
};

class GraphScreen {
    static constexpr int GRAPH_N = 7;
    static constexpr int GRAPH_E = 11;

    GNode nodes[GRAPH_N];
    GEdge edges[GRAPH_E];

    GraphInputMode inputMode;

    Button btnBack;
    Button btnDelete;
    Button btnEdit;

    InputField edgeFromFields[GRAPH_E];
    InputField edgeToFields[GRAPH_E];
    InputField edgeWeightFields[GRAPH_E];
    InputField matrixFields[GRAPH_N][GRAPH_N];
    InputField adjListFields[GRAPH_N];

    std::string message;
    float msgTimer;
    Color msgColor;

    void ClearInputFocus();
    void SetInputMode(GraphInputMode mode);
    void SetMsg(const char* msg, 
                Color c = {46, 160, 67, 255}, 
                float dur = 3.0f);

public:
    GraphScreen();
    Screen Update();
    void Draw() const;
};
