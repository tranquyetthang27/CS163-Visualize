#pragma once
#include "screen.h"
#include "button.h"
#include "input_field.h"
#include <string>
#include <vector>
#include <queue>
struct TrieNode {
    int children[128];
    int cnt;
    int endCount;
    float x, y, alpha;
    float targetX, targetY;
    int leafCount;
    char ch;

    TrieNode(char c) : ch(c), cnt(0), endCount(0), x(640), y(140), alpha(0), targetX(640), targetY(140), leafCount(0) {
        for (int i = 0; i < 128; i++) children[i] = -1;
    }
};

class TrieScreen {
    std::vector<TrieNode> pool;
    int root;

    Camera2D camera;

    InputField input;
    Button btnInsert, btnSearch, btnClear, btnBack, btnLoad, btnDelete;
    std::string message;
    float msgTimer;
    Color msgColor;

    std::vector<int> highlightPath;

    std::string pendingWord;
    int currentIdx;
    int currentNode;
    float stepTimer;
    bool isAnimating = false;
    bool isSearching = false;
    bool isDeletingStep = false;
    void Layout();
    void LayoutSubtree(int node, float x, float y, float spread);
    int UpdateLeafCount(int node);
    void SetMsg(const char* msg, Color c = {46, 160, 67, 255}, float dur = 2.5f);
public:
    TrieScreen();
    Screen Update();
    void Draw();

private:
    std::queue<std::string> loadQueue;
    bool isStepByStep = true; 
    Button btnToggleMode;

    void DrawAllEdges(int node);
    void DrawAllNodes(int node);

    void InstantInsert(const std::string& word);
    bool InstantSearch(const std::string& word);
    void OnLoadFileTriggered(const std::string& path);

    void StartInsert(const std::string& word);
    void StartSearch(const std::string& word);

    void Delete(const std::string& word);
    bool IsValidChild(int parent, int charIdx);

    Button btnToggleCode;
    bool showPseudoCode = false;
    int lastCodeOp = 0; // 0=insert, 1=search, 2=delete
    int GetPseudoCodeLine() const;
    void DrawCodePanel() const;
};
