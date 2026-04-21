#pragma once
#include "screen.h"
#include "button.h"
#include "input_field.h"
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

    void LayoutNodes();
    void SetMsg(const char* msg, Color c = {46,160,67,255}, float dur = 2.5f);

public:
    LinkedListScreen();
    Screen Update();
    void   Draw() const;
};
