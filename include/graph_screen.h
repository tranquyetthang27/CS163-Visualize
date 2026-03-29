#pragma once
#include "screen.h"
#include "button.h"
#include <vector>
#include <string>

struct GNode {
    float x, y;
    const char* label;
};

struct GEdge {
    int   u, v, w;
    bool  inMST;
    bool  skipped;   // would create cycle
    bool  highlighted;
};

class GraphScreen {
    static constexpr int GRAPH_N = 7;
    static constexpr int GRAPH_E = 11;

    GNode nodes[GRAPH_N];
    GEdge edges[GRAPH_E];

    // Kruskal union-find
    int   parent[GRAPH_N];
    int   ufRank[GRAPH_N];
    int   sortedEdges[GRAPH_E];   // indices into edges[], sorted by weight
    int   stepIdx;                // next edge to process
    int   mstCost;
    bool  mstDone;

    Button btnStep, btnReset, btnBack;

    std::string message;
    float       msgTimer;
    Color       msgColor;

    void ResetKruskal();
    int  Find(int x);
    bool Union(int x, int y);
    void SetMsg(const char* msg, Color c = {46,160,67,255}, float dur = 3.0f);

public:
    GraphScreen();
    Screen Update();
    void   Draw() const;
};
