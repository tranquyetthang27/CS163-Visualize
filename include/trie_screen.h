#pragma once
#include "screen.h"
#include "button.h"
#include "input_field.h"
#include <string>
#include <vector>
#include <array>

struct TrieNode {
    int  children[26];
    bool isEnd;
    float x, y, alpha;
    char  ch;

    TrieNode(char c = '$') : isEnd(false), x(0), y(0), alpha(0), ch(c) {
        for (int i = 0; i < 26; i++) children[i] = -1;
    }
};

class TrieScreen {
    std::vector<TrieNode> pool;
    int root;

    InputField input;
    Button btnInsert, btnSearch, btnClear, btnBack;

    std::string message;
    float       msgTimer;
    Color       msgColor;

    std::vector<int> highlightPath;  // nodes on search/insert path

    void Layout();
    void LayoutSubtree(int node, float x, float y, float spread);
    int  CountLeaves(int node) const;
    void SetMsg(const char* msg, Color c = {46,160,67,255}, float dur = 2.5f);

public:
    TrieScreen();
    Screen Update();
    void   Draw() const;

private:
    void DrawNode(int idx) const;
    void DrawEdges(int parent, int child) const;
    void DrawAllEdges(int node) const;
    void DrawAllNodes(int node) const;
    void Insert(const std::string& word);
    bool Search(const std::string& word);
};
