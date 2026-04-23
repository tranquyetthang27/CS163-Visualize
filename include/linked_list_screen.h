#pragma once
#include "screen.h"
#include "button.h"
#include "input_field.h"
#include <functional>
#include <vector>
#include <string>

enum class LLState { Normal, Highlighted, Found, Removing };

struct LLNode {
    int   value;
    float tx, ty;   // target position
    float x,  y;   // animated position
    float alpha;    // 0..1 for fade in/out
    LLState state;
};

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

    // ── Step-by-step ──────────────────────────────────────────────
    struct StepFrame {
        std::string          desc;
        std::vector<LLState> nodeStates; // per existing node
        bool                 showPending = false;
        bool                 isFinal     = false;
    };

    bool                   stepMode  = false;
    int                    stepIdx   = 0;
    std::vector<StepFrame> stepFrames;
    std::function<void()>  stepFinalAction;
    std::function<void()>  stepCancelAction;

    // pending node shown before actual insert
    bool   hasPendingNode   = false;
    LLNode pendingNode      = {};
    int    pendingInsertIdx = 0;

    Button btnNext, btnPrev, btnStop;

    void LayoutNodes();
    void SetMsg(const char* msg, Color c = {46,160,67,255}, float dur = 2.5f);

    void ApplyFrame(int i);
    void ExitStepMode(bool execute);
    void PrepareInsertSteps(int insertIdx, int v);

public:
    LinkedListScreen();
    Screen Update();
    void   Draw() const;
};
