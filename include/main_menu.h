#pragma once
#include "screen.h"
#include "button.h"

class MainMenu {
    Button btnStart, btnSettings, btnAbout, btnExit;

    // Background decoration: animated nodes
    struct BgNode { float x, y, vx, vy, r, pulse; };
    static constexpr int BG_NODE_COUNT = 18;
    BgNode bgNodes[BG_NODE_COUNT];

    float time;

public:
    MainMenu();
    Screen Update();
    void   Draw() const;
};
