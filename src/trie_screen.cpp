#include "trie_screen.h"
#include "font.h"
#include "colors.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cctype>

static constexpr float TRIE_TOP_Y  = 140.0f;
static constexpr float TRIE_DY     = 80.0f;
static constexpr float NODE_R      = 20.0f;

TrieScreen::TrieScreen()
    : input({340, 636, 200, 40}, "Enter word...", 10),
      btnInsert({20,  636, 120, 40}, "Insert",  Pal::BtnPrimary, Pal::BtnPrimHov),
      btnSearch ({550, 636, 120, 40}, "Search",  Pal::BtnOrange,  Pal::BtnOrangeHov),
      btnClear  ({155, 636, 100, 40}, "Clear",   Pal::BtnDanger,  Pal::BtnDangHov),
      btnBack   ({20,  20,  100, 36}, "< Back",  Pal::BtnNeutral, Pal::BtnNeutHov),
      msgTimer(0), msgColor(Pal::BtnSuccess), root(0)
{
    pool.emplace_back('$');   // root node
    pool[0].x = 640; pool[0].y = TRIE_TOP_Y; pool[0].alpha = 1.0f;
}

void TrieScreen::SetMsg(const char* msg, Color c, float dur) {
    message = msg; msgColor = c; msgTimer = dur;
}

int TrieScreen::CountLeaves(int node) const {
    int cnt = 0;
    for (int i = 0; i < 26; i++)
        if (pool[node].children[i] != -1)
            cnt += CountLeaves(pool[node].children[i]);
    return cnt == 0 ? 1 : cnt;
}

void TrieScreen::LayoutSubtree(int node, float x, float y, float spread) {
    pool[node].x = x;
    pool[node].y = y;

    float childY = y + TRIE_DY;
    int childCount = 0;
    for (int i = 0; i < 26; i++)
        if (pool[node].children[i] != -1) childCount++;
    if (childCount == 0) return;

    // Distribute children by leaf count
    int totalLeaves = 0;
    for (int i = 0; i < 26; i++)
        if (pool[node].children[i] != -1)
            totalLeaves += CountLeaves(pool[node].children[i]);

    float curX = x - spread / 2.0f;
    for (int i = 0; i < 26; i++) {
        int c = pool[node].children[i];
        if (c == -1) continue;
        int leaves = CountLeaves(c);
        float childSpread = spread * leaves / totalLeaves;
        LayoutSubtree(c, curX + childSpread / 2.0f, childY, childSpread);
        curX += childSpread;
    }
}

void TrieScreen::Layout() {
    // Root fixed at center top; spread based on leaf count
    int leaves = CountLeaves(root);
    float spread = std::max(500.0f, leaves * 70.0f);
    LayoutSubtree(root, 640.0f, TRIE_TOP_Y, spread);
    // Fade in new nodes
    for (auto& nd : pool)
        if (nd.alpha < 0.01f) nd.alpha = 0.01f;
}

void TrieScreen::Insert(const std::string& word) {
    highlightPath.clear();
    int cur = root;
    highlightPath.push_back(cur);
    for (char ch : word) {
        int idx = ch - 'a';
        if (pool[cur].children[idx] == -1) {
            pool[cur].children[idx] = (int)pool.size();
            pool.emplace_back(ch);
        }
        cur = pool[cur].children[idx];
        highlightPath.push_back(cur);
    }
    pool[cur].isEnd = true;
    Layout();
}

bool TrieScreen::Search(const std::string& word) {
    highlightPath.clear();
    int cur = root;
    highlightPath.push_back(cur);
    for (char ch : word) {
        int idx = ch - 'a';
        if (pool[cur].children[idx] == -1) return false;
        cur = pool[cur].children[idx];
        highlightPath.push_back(cur);
    }
    return pool[cur].isEnd;
}

Screen TrieScreen::Update() {
    float dt = GetFrameTime();

    // Animate alpha
    for (auto& nd : pool)
        nd.alpha += (1.0f - nd.alpha) * 8.0f * dt;

    if (msgTimer > 0) msgTimer -= dt;

    if (btnBack.Update() || IsKeyPressed(KEY_ESCAPE)) return Screen::Home;

    input.Update();
    bool doInsert = btnInsert.Update() || (input.focused && IsKeyPressed(KEY_ENTER));
    bool doSearch = btnSearch.Update();
    bool doClear  = btnClear.Update();

    if (doClear) {
        pool.clear();
        pool.emplace_back('$');
        pool[0].x = 640; pool[0].y = TRIE_TOP_Y; pool[0].alpha = 1.0f;
        highlightPath.clear();
        input.Clear();
        SetMsg("Trie cleared.", Pal::BtnNeutral);
        return Screen::Trie;
    }

    if (doInsert && !input.IsEmpty()) {
        std::string word = input.text;
        for (char& c : word) c = (char)std::tolower(c);
        // only alpha
        std::string clean;
        for (char c : word) if (std::isalpha(c)) clean += c;
        if (clean.empty()) { SetMsg("Letters only!", Pal::BtnDanger); }
        else if (clean.size() > 10) { SetMsg("Max 10 characters!", Pal::BtnDanger); }
        else {
            Insert(clean);
            char buf[64]; snprintf(buf, sizeof(buf), "Inserted \"%s\".", clean.c_str());
            SetMsg(buf);
            input.Clear();
        }
    } else if (doSearch && !input.IsEmpty()) {
        std::string word = input.text;
        for (char& c : word) c = (char)std::tolower(c);
        std::string clean;
        for (char c : word) if (std::isalpha(c)) clean += c;
        bool found = Search(clean);
        char buf[64]; snprintf(buf, sizeof(buf), "\"%s\" %s in trie.", clean.c_str(), found ? "FOUND" : "NOT FOUND");
        SetMsg(buf, found ? Pal::BtnSuccess : Pal::BtnDanger);
    }

    return Screen::Trie;
}

void TrieScreen::DrawAllEdges(int node) const {
    bool onPath = false;
    for (int p : highlightPath) if (p == node) { onPath = true; break; }

    for (int i = 0; i < 26; i++) {
        int c = pool[node].children[i];
        if (c == -1) continue;

        bool childOnPath = false;
        for (int p : highlightPath) if (p == c) { childOnPath = true; break; }
        bool edgeHL = onPath && childOnPath;

        Color ec = edgeHL ? Color{255,152,0,220} : Pal::EdgeColor;
        DrawLineEx({pool[node].x, pool[node].y},
                   {pool[c].x,    pool[c].y}, edgeHL ? 2.5f : 1.5f, ec);
        DrawAllEdges(c);
    }
}

void TrieScreen::DrawAllNodes(int node) const {
    bool onPath = false;
    for (int p : highlightPath) if (p == node) { onPath = true; break; }
    bool isLast = !highlightPath.empty() && highlightPath.back() == node;

    Color fillC  = onPath ? (isLast ? Pal::NodeFound : Pal::NodeHL) : Pal::NodeFill;
    Color bordC  = onPath ? (isLast ? Color{36,128,54,255} : Color{200,160,0,255}) : Pal::NodeBorder;
    Color textC  = (onPath && isLast) ? WHITE : Pal::TxtDark;

    unsigned char a = (unsigned char)(pool[node].alpha * 220);
    fillC.a = bordC.a = textC.a = a;

    // End-of-word ring
    if (pool[node].isEnd) {
        DrawCircleV({pool[node].x, pool[node].y}, NODE_R + 4,
                    {63, 81, 181, (unsigned char)(a/2)});
    }

    DrawCircleV({pool[node].x, pool[node].y}, NODE_R + 1.5f, bordC);
    DrawCircleV({pool[node].x, pool[node].y}, NODE_R,        fillC);

    // Character label
    char buf[3];
    if (pool[node].ch == '$') buf[0] = '\0';
    else { buf[0] = (char)std::toupper(pool[node].ch); buf[1] = '\0'; }
    if (buf[0]) {
        Vector2 ts = MeasureTextEx(fontBold, buf, 16.0f, 1.0f);
        DrawTextEx(fontBold, buf,
            {pool[node].x - ts.x/2, pool[node].y - ts.y/2},
            16.0f, 1.0f, textC);
    }

    for (int i = 0; i < 26; i++) {
        int c = pool[node].children[i];
        if (c != -1) DrawAllNodes(c);
    }
}

void TrieScreen::Draw() const {
    ClearBackground(Pal::BG);

    // Header
    DrawRectangleRec({0, 0, 1280, 72}, Pal::Surface);
    DrawLineEx({0, 72}, {1280, 72}, 1.0f, Pal::Border);
    DrawTextEx(fontBold,    "Trie",   {130, 20}, 28.0f, 1.0f, Pal::TxtDark);
    DrawTextEx(fontRegular, "Insert words and search prefixes",
               {130, 52}, 13.5f, 1.0f, Pal::TxtLight);
    btnBack.Draw();

    // Legend
    DrawCircleV({820, 50}, 7, Pal::NodeFound);
    DrawTextEx(fontRegular, "= end of word", {832, 43}, 13.0f, 1.0f, Pal::TxtMid);

    // Trie drawing area
    DrawAllEdges(root);
    DrawAllNodes(root);

    // Bottom panel
    DrawRectangleRec({0, 616, 1280, 104}, Pal::Panel);
    DrawLineEx({0, 616}, {1280, 616}, 1.0f, Pal::Border);

    btnInsert.Draw();
    btnClear.Draw();
    input.Draw();
    btnSearch.Draw();

    // Message
    if (msgTimer > 0 && !message.empty()) {
        float alpha = msgTimer < 0.5f ? msgTimer / 0.5f : 1.0f;
        Color c = msgColor;
        c.a = (unsigned char)(alpha * 220);
        DrawTextEx(fontRegular, message.c_str(), {690, 646}, 16.0f, 1.0f, c);
    }

    char cnt[32]; snprintf(cnt, sizeof(cnt), "Nodes: %d", (int)pool.size() - 1);
    DrawTextEx(fontRegular, cnt, {1180, 646}, 14.0f, 1.0f, Pal::TxtLight);
}
