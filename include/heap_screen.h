#pragma once
#include <queue>
#include <string>
#include <vector>
#include <stack>
#include "button.h"
#include "init_file.h"
#include "input_field.h"
#include "screen.h"



enum class HeapAnimStep { 
    Idle, 
    SwapUp, 
    SwapDown 
};

struct HeapNodeVis {
    float x, y;         // Animated position
    float tx, ty;       // Target position
    float alpha;
    bool highlighted;
};


class HeapScreen {
private:
    static constexpr int MAX_SIZE = 15;
    std::vector<int> heap;            // Max-heap array
    std::vector<HeapNodeVis> vis;
    std::queue<int> loadQueue;
    InputField input;
    Button btnInsert;
    Button btnDelMax;
    Button btnBack;
    Button btnMode;
    Button btnLoad;
    std::string message;
    float msgTimer;
    Color msgColor;

    //Animation queue: pairs of indices to highlight swap
    int animA;
    int animB;
    int animC;                        // -1 = none
    int idCurrent;
    float stepTimer;

    //Logic and Control Flags
    bool doingInsert;
    bool doingDelete;
    bool isHighlight;
    bool isStepByStep;

    //Internal Helpers
    void ComputePositions();
    void SetMsg(const char* msg, Color c = {46, 160, 67, 255}, float dur = 2.5f);
    void GetNodePos(int i, float& x, float& y) const;
    void InstantInsert();
    void InstantDel();
    int  GetPseudoCodeLine() const;
    void DrawCodePanel() const;

public:
    //Core API
    HeapScreen();

    Screen Update();
    void Draw() const;

private:
    Button btnToggleCode;
    bool   showPseudoCode;
    bool   codeShowInsert;
};