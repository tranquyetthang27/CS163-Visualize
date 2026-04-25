#pragma once
#include "screen.h"
#include "button.h"
#include "input_field.h"
#include <vector>
#include <string>

enum class HeapAnimStep { Idle, SwapUp, SwapDown };

struct HeapNodeVis {
    float x, y;       // animated position
    float tx, ty;     // target position
    float alpha;
    bool highlighted;
};

class HeapScreen {
    std::vector<int> heap;   // max-heap array
    std::vector<HeapNodeVis> vis;

    InputField input;
    Button btnInsert, btnDelMax, btnBack, btnMode;

    std::string message;
    float       msgTimer;
    Color       msgColor;

    // Animation queue: pairs of indices to highlight swap
    int   animA, animB, animC;   // -1 = none
    int   idCurrent;

    float stepTimer;

    bool  doingInsert;
    bool  doingDelete;
    bool  isHighlight;

    bool isStepByStep;

    static constexpr int MAX_SIZE = 15;

    void ComputePositions();
    void SetMsg(const char* msg, Color c = {46,160,67,255}, float dur = 2.5f);
    void GetNodePos(int i, float& x, float& y) const;

    void InstantInsert();
    void InstantDel();

public:
    HeapScreen();
    Screen Update();
    void   Draw() const;
};
