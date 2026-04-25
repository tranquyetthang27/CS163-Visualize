#pragma once

// --- Standard Libraries ---
#include <queue>
#include <string>
#include <vector>

// --- Project Headers ---
#include "button.h"
#include "init_file.h"
#include "input_field.h"
#include "screen.h"

// ============================================================================

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

// ============================================================================

class HeapScreen {
private:
    // --- Constants ---
    static constexpr int MAX_SIZE = 15;

    // --- Data Structures ---
    std::vector<int> heap;            // Max-heap array
    std::vector<HeapNodeVis> vis;
    std::queue<int> loadQueue;

    // --- UI Components ---
    InputField input;
    Button btnInsert;
    Button btnDelMax;
    Button btnBack;
    Button btnMode;
    Button btnLoad;

    // --- Feedback & Messages ---
    std::string message;
    float msgTimer;
    Color msgColor;

    // --- Animation State ---
    // Animation queue: pairs of indices to highlight swap
    int animA;
    int animB;
    int animC;                        // -1 = none
    int idCurrent;
    float stepTimer;

    // --- Logic & Control Flags ---
    bool doingInsert;
    bool doingDelete;
    bool isHighlight;
    bool isStepByStep;

    // --- Internal Helpers ---
    void ComputePositions();
    void SetMsg(const char* msg, Color c = {46, 160, 67, 255}, float dur = 2.5f);
    void GetNodePos(int i, float& x, float& y) const;
    void InstantInsert();
    void InstantDel();

public:
    // --- Core API ---
    HeapScreen();
    
    Screen Update();
    void Draw() const;
};