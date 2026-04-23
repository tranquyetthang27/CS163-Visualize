#pragma once
#include "screen.h"
#include "button.h"
#include "input_field.h"
#include <vector>
#include <string>

enum class LLState { Normal, Highlighted, Found, Removing, Predecessor };

struct LLNode {
    int   value;
    float tx, ty;
    float x,  y;
    float alpha;
    LLState state;
};

enum class StepOp { None, Insert, Delete };

class LinkedListScreen {
    std::vector<LLNode> nodes;

    InputField input;
    Button btnInsert, btnInsHead, btnInsTail, btnInsIdx;
    Button btnDel,    btnDelHead, btnDelTail, btnDelIdx;
    Button btnSearch, btnUpdate, btnBack;
    bool   insertMenuOpen;
    bool   deleteMenuOpen;

    std::string message;
    float       msgTimer;
    Color       msgColor;

    // Step-by-step (insert & delete)
    StepOp stepOp     = StepOp::None;
    bool   stepActive = false;
    int    stepPhase  = 0;
    float  stepTimer  = 0.0f;
    int    stepIdx    = -1;   // inserted index OR delete target index

    void LayoutNodes();
    void SetMsg(const char* msg, Color c = {46,160,67,255}, float dur = 2.5f);
    void StartInsertStep(int idx, int v);
    void StartDeleteStep(int idx);
    void AdvanceStep();

public:
    LinkedListScreen();
    Screen Update();
    void   Draw() const;
};
