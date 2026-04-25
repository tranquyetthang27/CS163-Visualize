#include "trie_screen.h"
#include "font.h"
#include "colors.h"
#include "camera.h"
#include "init_file.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cctype>

static constexpr float TRIE_TOP_Y  = 140.0f;
static constexpr float TRIE_DY     = 80.0f;
static constexpr float NODE_R      = 20.0f;

TrieScreen::TrieScreen()
    : btnInsert({20,  636, 80,  40}, "Insert",    Pal::BtnPrimary, Pal::BtnPrimHov),
      btnClear  ({110, 636, 80,  40}, "Clear",     Pal::BtnDanger,  Pal::BtnDangHov),
      input     ({200, 636, 180, 40}, "Enter word...", 10),
      btnSearch ({390, 636, 100, 40}, "Search",    Pal::BtnOrange,  Pal::BtnOrangeHov),
      btnToggleMode({500, 636, 140, 40}, "Mode: Step", Pal::BtnNeutral, Pal::BtnNeutHov), 
      btnLoad   ({650, 636, 110, 40}, "Load File", Pal::BtnNeutral, Pal::BtnNeutHov),
      btnBack   ({20,  20,  100, 36}, "< Back",    Pal::BtnNeutral, Pal::BtnNeutHov),
      btnDelete ({780, 636, 110, 40}, "Delete", Pal::BtnNeutral, Pal::BtnNeutHov),
      msgTimer(0), msgColor(Pal::BtnSuccess), root(0)
{
    camera.target = (Vector2){ 640, TRIE_TOP_Y };
    camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/3.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    pool.emplace_back('$');   
    pool[0].x = pool[0].targetX = 640; 
    pool[0].y = pool[0].targetY = TRIE_TOP_Y; 
    pool[0].alpha = 1.0f;
}


void TrieScreen::SetMsg(const char* msg, Color c, float dur) {
    message = msg; msgColor = c; msgTimer = dur;
}

int TrieScreen::UpdateLeafCount(int node) {
    if (node == -1 || pool[node].cnt <= 0) return 0;
    int cnt = 0;
    bool isLeaf = true;
    for (int i = 0; i < 26; i++) {
        int child = pool[node].children[i];
        if (child != -1 && pool[child].cnt > 0) {
            cnt += UpdateLeafCount(pool[node].children[i]);
            isLeaf = false;
        }
    }
    pool[node].leafCount = isLeaf ? 1 : cnt;
    return pool[node].leafCount;
}

void TrieScreen::LayoutSubtree(int node, float x, float y, float spread) {
    if(node == -1 || pool[node].cnt <= 0)return;
    pool[node].targetX = x; 
    pool[node].targetY = y; 
    
    float childY = y + TRIE_DY;
    int totalLeaves = pool[node].leafCount;

    float curX = x - spread / 2.0f;
    for (int i = 0; i < 26; i++) {
        int c = pool[node].children[i];
        if (c == -1) continue;
        
        float childSpread = spread * (float)pool[c].leafCount / (float)totalLeaves;
        LayoutSubtree(c, curX + childSpread / 2.0f, childY, childSpread);
        curX += childSpread;
    }
}

void TrieScreen::Layout() {
    if (pool.empty()) return;
    int leaves = UpdateLeafCount(root);
    float spread = std::max(500.0f, leaves * 70.0f);
    LayoutSubtree(root, 640.0f, TRIE_TOP_Y, spread);
}


Screen TrieScreen::Update() {
    float dt = GetFrameTime();
    
    if (!isAnimating && !isSearching && !loadQueue.empty()) {
        std::string nextWord = loadQueue.front();
        loadQueue.pop();
        StartInsert(nextWord); 
    }

    UpdateCameraZoom(camera); 
    UpdateCameraPan(camera);
    
    for (auto& nd : pool) {
        nd.alpha += (1.0f - nd.alpha) * 8.0f * dt;
        nd.x += (nd.targetX - nd.x) * 10.0f * dt;
        nd.y += (nd.targetY - nd.y) * 10.0f * dt;
    }

    if (btnToggleMode.Update()) {
    isStepByStep = !isStepByStep;
    
    if (isStepByStep) {
        btnToggleMode.label = "Mode: Step";
        btnToggleMode.baseColor = Pal::BtnNeutral; 
    } else {
        btnToggleMode.label = "Mode: Instant";
        btnToggleMode.baseColor = Pal::BtnPrimary; 
    }
}

    if (isAnimating || isSearching) {
        stepTimer += dt;
        if (stepTimer >= 0.5f) {
            stepTimer = 0.0f;
            if (currentIdx < (int)pendingWord.size()) {
                int idx = pendingWord[currentIdx] - 'a';
                if (isAnimating) {
                    if (pool[currentNode].children[idx] == -1) {
                        int newNodeIdx = (int)pool.size();
                        pool.emplace_back(pendingWord[currentIdx]);
                        pool[newNodeIdx].x = pool[currentNode].x;
                        pool[newNodeIdx].y = pool[currentNode].y;
                        pool[currentNode].children[idx] = newNodeIdx;
                        Layout();
                    }
                    currentNode = pool[currentNode].children[idx];
                    highlightPath.push_back(currentNode);
                    currentIdx++;
                } else if (isSearching) {
                    if (pool[currentNode].children[idx] != -1) {
                        currentNode = pool[currentNode].children[idx];
                        highlightPath.push_back(currentNode);
                        currentIdx++;
                    } else {
                        isSearching = false;
                        SetMsg("Not Found", Pal::BtnDanger);
                    }
                }
            } else {
                if (isAnimating) {
                    pool[currentNode].isEnd = true;
                    SetMsg("Inserted successfully!");
                } else if (isSearching) {
                    if (pool[currentNode].isEnd) SetMsg("Found!", Pal::BtnSuccess);
                    else SetMsg("Prefix exists, but word not found", Pal::BtnOrange);
                }
                isAnimating = isSearching = false;
            }
        }
    }

    if(isAnimating || isSearching)return Screen::Trie;
    
    if (msgTimer > 0) msgTimer -= dt;

    if (btnBack.Update() || IsKeyPressed(KEY_ESCAPE)) return Screen::Home;

    input.Update();

    bool clickInsert = btnInsert.Update() || (input.focused && IsKeyPressed(KEY_ENTER));
    bool clickSearch = btnSearch.Update();
    bool clickClear = btnClear.Update();
    bool clickDelete = btnDelete.Update();

    if(clickDelete && !input.IsEmpty()){
        std::string word;
        for(char c: input.text)word += (char)tolower(c);
        Delete(word);
        input.Clear();
    }
    if (clickClear) {
        pool.clear();
        pool.emplace_back('$');
        pool[0].x = pool[0].targetX = 640;
        pool[0].y = pool[0].targetY = TRIE_TOP_Y;
        pool[0].alpha = 1.0f;
        highlightPath.clear();
        input.Clear();
        isAnimating = isSearching = false;
        SetMsg("Trie cleared.", Pal::BtnNeutral);
        return Screen::Trie;
    }

    if ((clickInsert || clickSearch) && !input.IsEmpty()) {
        std::string clean;
        for (char c : input.text) if (isalpha(c)) clean += (char)tolower(c);
        
        if (!clean.empty()) {
            if (isStepByStep) {
                if (clickInsert) StartInsert(clean);
                else StartSearch(clean);
            } else {
                if (clickInsert) {
                    InstantInsert(clean); 
                    SetMsg("Inserted (Instant)");
                } else {
                    InstantSearch(clean); 
                }
            }
        }
        input.Clear();
    }

    if (btnLoad.Update()) {
        std::string fullPath = std::string(PROJECT_ROOT_PATH) + "data.txt";
        OnLoadFileTriggered(fullPath); 
    }

    return Screen::Trie;
}

void TrieScreen::StartInsert(const std::string& word) {
    pendingWord = word;
    currentIdx = 0;
    currentNode = root;
    isAnimating = true;
    isSearching = false;
    stepTimer = 0.0f;
    highlightPath.clear();
    highlightPath.push_back(root);
}

void TrieScreen::StartSearch(const std::string& word) {
    pendingWord = word;
    currentIdx = 0;
    currentNode = root;
    isSearching = true;
    isAnimating = false;
    stepTimer = 0.0f;
    highlightPath.clear();
    highlightPath.push_back(root);
}


void TrieScreen::DrawAllEdges(int node) {
    if(node == -1 || pool[node].cnt <= 0)return;
    for (int i = 0; i < 26; i++) {
        int c = pool[node].children[i];
        if (c == -1) continue;

        bool onPath = false;
        for (int p : highlightPath) if (p == node) onPath = true;
        bool childOnPath = false;
        for (int p : highlightPath) if (p == c) childOnPath = true;
        bool edgeHL = onPath && childOnPath && !highlightPath.empty();

        Color ec = edgeHL ? Color{255, 152, 0, 220} : Pal::EdgeColor;
        ec.a = (unsigned char)(pool[c].alpha * ec.a);

        DrawLineEx({pool[node].x, pool[node].y}, {pool[c].x, pool[c].y}, edgeHL ? 3.0f : 1.5f, ec);
        DrawAllEdges(c);
    }
}

void TrieScreen::DrawAllNodes(int node){
    if(node == -1 || pool[node].cnt <= 0)return;
    bool onPath = false;
    for (int p : highlightPath) if (p == node) onPath = true;
    bool isLast = !highlightPath.empty() && highlightPath.back() == node;

    Color fillC = onPath ? (isLast ? Pal::NodeFound : Pal::NodeHL) : Pal::NodeFill;
    Color bordC = onPath ? (isLast ? Color{36, 128, 54, 255} : Color{200, 160, 0, 255}) : Pal::NodeBorder;
    Color textC = (onPath && isLast) ? WHITE : Pal::TxtDark;

    unsigned char a = (unsigned char)(pool[node].alpha * 255);
    fillC.a = bordC.a = textC.a = a;

    if (pool[node].isEnd) {
        Color ringC = {63, 81, 181, (unsigned char)(a / 2)};
        DrawCircleV({pool[node].x, pool[node].y}, NODE_R + 5, ringC);
    }

    DrawCircleV({pool[node].x, pool[node].y}, NODE_R + 1.5f, bordC);
    DrawCircleV({pool[node].x, pool[node].y}, NODE_R, fillC);

    if (pool[node].ch != '$') {
        char buf[2] = { (char)std::toupper(pool[node].ch), '\0' };
        Vector2 ts = MeasureTextEx(fontBold, buf, 18.0f, 1.0f);
        DrawTextEx(fontBold, buf, {pool[node].x - ts.x / 2, pool[node].y - ts.y / 2}, 18.0f, 1.0f, textC);
    }

    for (int i = 0; i < 26; i++) {
        if (pool[node].children[i] != -1) DrawAllNodes(pool[node].children[i]);
    }
}

void TrieScreen::Draw(){
    ClearBackground(Pal::BG);

    BeginMode2D(camera);
        DrawAllEdges(root);
        DrawAllNodes(root);
    EndMode2D();

    DrawRectangleRec({0, 0, 1280, 72}, Pal::Surface);
    DrawLineEx({0, 72}, {1280, 72}, 1.0f, Pal::Border);
    DrawTextEx(fontBold, "Trie Visualization", {130, 20}, 28.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, "Insert words to build the tree and search prefixes", {130, 52}, 14.0f, 1.0f, Pal::TxtLight);
    
    DrawCircleV({820, 45}, 7, Pal::NodeFound);
    DrawTextEx(fontRegular, "= End of Word", {835, 38}, 14.0f, 1.0f, Pal::TxtMid);

    DrawRectangleRec({0, 616, 1280, 104}, Pal::Panel);
    DrawLineEx({0, 616}, {1280, 616}, 1.0f, Pal::Border);

    btnInsert.Draw();
    btnClear.Draw();
    input.Draw();
    btnSearch.Draw();
    btnToggleMode.Draw();
    btnBack.Draw();
    btnLoad.Draw();
    btnDelete.Draw();

    if (msgTimer > 0 && !message.empty()) {
        float fade = (msgTimer < 0.5f) ? (msgTimer / 0.5f) : 1.0f;
        Color c = msgColor; 
        c.a = (unsigned char)(fade * 255);
        Vector2 msgPos = { 20, 600 }; 
        DrawTextEx(fontRegular, message.c_str(), msgPos, 20.0f, 1.0f, c);
    }


    char cnt[32];
    snprintf(cnt, sizeof(cnt), "Total Nodes: %d", (int)pool.size());
    DrawTextEx(fontRegular, cnt, {1140, 646}, 15.0f, 1.0f, Pal::TxtMid);
}

void TrieScreen::InstantInsert(const std::string& word) {
    int curr = root;
    pool[curr].cnt++;
    for (char c : word) {
        int idx = tolower(c) - 'a';
        if (pool[curr].children[idx] == -1) {
            int newNodeIdx = (int)pool.size();
            pool.emplace_back(tolower(c));
            pool[newNodeIdx].alpha = 1.0f; 
            pool[newNodeIdx].x = pool[curr].x;
            pool[newNodeIdx].y = pool[curr].y;
            pool[curr].children[idx] = newNodeIdx;
        }
        curr = pool[curr].children[idx];
        pool[curr].cnt++;
    }
    pool[curr].isEnd = true;
    Layout(); 
}


bool TrieScreen::InstantSearch(const std::string& word) {
    int curr = root;
    highlightPath.clear();
    highlightPath.push_back(root);

    for (char c : word) {
        int idx = tolower(c) - 'a';
        if (pool[curr].children[idx] == -1) {
            SetMsg("Not Found", Pal::BtnDanger);
            return 0;
        }
        curr = pool[curr].children[idx];
        highlightPath.push_back(curr);
    }

    if (pool[curr].isEnd) SetMsg("Found!", Pal::BtnSuccess);
    else{
        SetMsg("Prefix exists, but word not found", Pal::BtnOrange);
        return 0;
    }
    return 1;
}

void TrieScreen::OnLoadFileTriggered(const std::string& path) {
    std::vector<std::string> wordList = InitFile::loadWords(path);
    if (wordList.empty()) {
        SetMsg("Failed to load or file empty", Pal::BtnDanger);
        return;
    }

    if (isStepByStep) {
        for (const std::string& w : wordList) loadQueue.push(w);
        SetMsg("Processing file step-by-step...", Pal::BtnOrange);
    } else {
        for (const std::string& w : wordList) InstantInsert(w);
        Layout();
        SetMsg("File loaded successfully (Instant)!");
    }
}

void TrieScreen::Delete(const std::string& word){
    if(!TrieScreen::InstantSearch(word))return;
    int cur = root;
    for(char c: word){
        int idx = tolower(c) - 'a';
        int nextnode = pool[cur].children[idx];
        if(nextnode == -1)break;
        pool[nextnode].cnt--;
        if(pool[nextnode].cnt <= 0){
            pool[cur].children[idx] = -1;
            SetMsg("Deleted successfully!");
            Layout();
        }
        cur = nextnode;
    }
    if(cur != -1)pool[cur].isEnd = false;
    highlightPath.clear();
    SetMsg("Deleted successfully!");
    Layout();
}