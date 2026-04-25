#pragma once
#include "screen.h"
#include "button.h"
#include "input_field.h"
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

enum class LLState { Normal, Highlighted, Found, Removing, Predecessor, Successor };

struct LLNode {
    int   value;
    float tx, ty;
    float x,  y;
    float alpha;
    LLState state;
};

enum class StepOp { None, Insert, Delete, Search, Update };

class LinkedListScreen {
    std::vector<LLNode> nodes;

    InputField input;
    Button btnInsert, btnInsHead, btnInsTail, btnInsIdx;
    Button btnDel,    btnDelHead, btnDelTail, btnDelIdx;
    Button btnSearch, btnUpdate, btnBack, btnLoadFile;
    bool   insertMenuOpen;
    bool   deleteMenuOpen;

    std::string message;
    float       msgTimer;
    Color       msgColor;

    bool   showCode   = false;
    Button btnShowCode;

    float  scrollX = 0.0f;
    Button btnScrollLeft, btnScrollRight;

    // Mode: Step-by-step vs Instant (same pattern as HeapScreen / TrieScreen)
    bool   isStepByStep = true;
    Button btnMode;

    // Step-by-step animation state
    StepOp stepOp     = StepOp::None;
    bool   stepActive = false;
    int    stepPhase  = 0;
    float  stepTimer  = 0.0f;
    int    stepIdx    = -1;
    int    stepVal    = 0;
    int    stepNewVal = 0;

    void LayoutNodes();
    void SetMsg(const char* msg, Color c = {46,160,67,255}, float dur = 2.5f);
    void StartInsertStep(int idx, int v);
    void StartDeleteStep(int idx);
    void StartSearchStep(int v);
    void StartUpdateStep(int idx, int newVal);
    void AdvanceStep();
    void InstantInsert(int idx, int v);
    void InstantDelete(int idx);
    void OnLoadFileTriggered(const std::string& path);

    // File dialog runs on background thread to avoid blocking game loop
    std::thread           dialogThread;
    std::atomic<bool>     dialogPending { false };
    std::mutex            dialogMutex;
    std::string           dialogResult;

public:
    LinkedListScreen();
    void   Reset();
    Screen Update();
    void   Draw() const;
};
