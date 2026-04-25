#include "card.h"
#include "font.h"
#include "colors.h"
#include <cmath>

Card::Card(Rectangle r, Screen t,
           const char* ttl, const char* dsc, Color acc)
    : rect(r), target(t), title(ttl), desc(dsc),
      accent(acc), hoverAnim(0.0f), animTime(0.0f) {}

bool Card::Update() {
    bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
    hoverAnim += ((hovered ? 1.0f : 0.0f) - hoverAnim) * 10.0f * GetFrameTime();
    if (hoverAnim > 0.01f)
        animTime += GetFrameTime();
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void Card::Draw() const {
    // Lift effect: shift card up slightly on hover
    float lift = hoverAnim * 5.0f;
    Rectangle r = {rect.x, rect.y - lift, rect.width, rect.height};

    // Shadow
    Rectangle shadow = {r.x + 4, r.y + 6 + lift * 0.5f, r.width, r.height};
    DrawRectangleRounded(shadow, 0.08f, 8,
        {0, 0, 0, (unsigned char)(20 + (int)(20 * hoverAnim))});

    // Card background
    DrawRectangleRounded(r, 0.08f, 8, Pal::Surface);

    // Colored header 
    float headerH = r.height * 0.60f;
    Rectangle header = {r.x, r.y, r.width, headerH};

    // Clip header with rounded top corners
    DrawRectangleRounded(r, 0.08f, 8, accent);   // full card in accent color
    // White lower portion
    Rectangle foot = {r.x, r.y + headerH, r.width, r.height - headerH};
    DrawRectangleRec(foot, Pal::Surface);
    // Thin border
    Rectangle border = {r.x - 1, r.y - 1, r.width + 2, r.height + 2};
    DrawRectangleRounded(border, 0.08f, 8, Pal::Border);
    DrawRectangleRounded(r, 0.08f, 8, accent);
    DrawRectangleRec(foot, Pal::Surface);

    // Illustration inside header
    DrawIllustration(header);

    // Title
    float titleY = r.y + headerH + 14;
    DrawTextEx(fontBold, title,
        {r.x + 16, titleY}, 17.0f, 1.0f, Pal::TxtDark);

    // Description
    DrawTextEx(fontRegular, desc,
        {r.x + 16, titleY + 24}, 13.5f, 1.0f, Pal::TxtMid);

    // Hover arrow indicator (bottom-right)
    if (hoverAnim > 0.05f) {
        float ax = r.x + r.width - 28;
        float ay = r.y + headerH + (r.height - headerH) / 2;
        Color arrowC = {accent.r, accent.g, accent.b, (unsigned char)(200 * hoverAnim)};
        DrawTextEx(fontBold, ">", {ax, ay - 10}, 20.0f, 1.0f, arrowC);
    }
}

//Illustration helpers

static void DrawNode(float x, float y, float r, Color fill, Color border) {
    DrawCircleV({x, y}, r + 1.5f, border);
    DrawCircleV({x, y}, r,        fill);
}

static void DrawArrowH(float x1, float x2, float y, Color c) {
    DrawLineEx({x1, y}, {x2 - 6, y}, 1.5f, c);
    // arrowhead
    DrawTriangle({x2, y}, {x2 - 7, y - 4}, {x2 - 7, y + 4}, c);
}

static void DrawEdge(float x1, float y1, float x2, float y2, Color c) {
    DrawLineEx({x1, y1}, {x2, y2}, 1.5f, c);
}

void Card::DrawIllustration(Rectangle area) const {
    constexpr Color W  = {255, 255, 255, 210};
    constexpr Color WD = {255, 255, 255, 120};
    float cx = area.x + area.width  / 2.0f;
    float cy = area.y + area.height / 2.0f;
    float t = animTime;
    float a = hoverAnim;

    // Helper lerp two colors by factor 0..1
    auto lerpColor = [](Color c0, Color c1, float f) -> Color {
        return {
            (unsigned char)(c0.r + (c1.r - c0.r) * f),
            (unsigned char)(c0.g + (c1.g - c0.g) * f),
            (unsigned char)(c0.b + (c1.b - c0.b) * f),
            (unsigned char)(c0.a + (c1.a - c0.a) * f),
        };
    };
    // Accent at full opacity for color mixing
    Color acc = {accent.r, accent.g, accent.b, 255};
    // Brightened accent blend accent toward white for the highlight
    Color accBright = lerpColor(acc, {255,255,255,255}, 0.45f);

    if (target == Screen::LinkedList) {
        constexpr float DW  = 30.0f;
        constexpr float PW  = 30.0f;
        constexpr float NW  = DW + PW;
        constexpr float NH  = 34.0f;
        constexpr float ARR = 30.0f;
        constexpr float STP = NW + ARR;
        constexpr int   NN  = 4;

        float totalW = NN * STP - ARR;
        float startX = cx - totalW * 0.5f;
        float baseY  = cy - NH * 0.5f;

        // hover
        unsigned char bordA  = (unsigned char)(160 + 95 * a);
        Color cBord  = {255, 255, 255, bordA};
        Color cDiv   = {255, 255, 255, bordA};

        //light
        for (int i = 0; i < NN - 1; i++) {
            float wave     = sinf(t * 4.5f - i * 1.1f);
            float hoverAlp = wave * 0.5f + 0.5f;
            unsigned char aa = (unsigned char)(160.0f + 95.0f * (0.3f + 0.7f * hoverAlp * a));
            float thick    = 1.8f + 0.8f * hoverAlp * a;

            Color cArrow = {255, 255, 255, aa};
            float ay  = baseY + NH * 0.5f;
            float ax1 = startX + i * STP + NW;
            float ax2 = ax1 + ARR;
            DrawLineEx({ax1, ay}, {ax2 - 7, ay}, thick, cArrow);
            DrawTriangle({ax2, ay}, {ax2 - 8, ay - 5}, {ax2 - 8, ay + 5}, cArrow);
        }

        for (int i = 0; i < NN; i++) {
            float wave    = sinf(t * 3.2f - i * 1.0f);
            float bounceY = wave * 5.0f * a;
            float x = startX + i * STP;
            float y = baseY + bounceY;

            unsigned char fa = (unsigned char)(200 + 30 * a);
            DrawRectangleRec({x, y, DW, NH}, {255, 255, 255, fa});
            DrawRectangleLinesEx({x,      y, DW, NH}, 1.8f, cBord);
            DrawRectangleLinesEx({x + DW, y, PW, NH}, 1.8f, cBord);
        }

    } else if (target == Screen::Trie) {
        float nr = 5.5f;
        struct N { float x, y; };
        float dy = 44, dx = 58;
        N base[] = {
            {cx,           cy - dy},
            {cx - dx,      cy},
            {cx + dx,      cy},
            {cx - dx - 26, cy + dy},
            {cx - dx + 18, cy + dy},
            {cx + dx - 18, cy + dy},
            {cx + dx + 26, cy + dy},
        };
        N nodes[7];
        float wave[7];
        for (int i = 0; i < 7; i++) {
            wave[i] = (sinf(t * 2.0f + i * 0.8f) + 1.0f) * 0.5f;
            nodes[i].x = base[i].x + cosf(t * 1.6f + i * 1.3f) * 3.5f * a;
            nodes[i].y = base[i].y + (wave[i] * 2.0f - 1.0f) * 4.0f * a;
        }
        int edges[][2] = {{0,1},{0,2},{1,3},{1,4},{2,5},{2,6}};
        for (auto& e : edges)
            DrawEdge(nodes[e[0]].x, nodes[e[0]].y,
                     nodes[e[1]].x, nodes[e[1]].y, WD);
        for (int i = 0; i < 7; i++) {
            float cf = wave[i] * a;
            Color fill   = lerpColor({255,255,255,180}, accBright, cf);
            Color border = lerpColor({255,255,255,80},  {accBright.r,accBright.g,accBright.b,160}, cf);
            DrawNode(nodes[i].x, nodes[i].y, nr, fill, border);
        }

    } else if (target == Screen::Heap) {
        float nr = 6.0f;
        float dy = 42, dx = 55;
        struct N { float x, y; };
        N base[] = {
            {cx,             cy - dy},
            {cx - dx,        cy},
            {cx + dx,        cy},
            {cx - dx - 27,   cy + dy},
            {cx - dx + 20,   cy + dy},
            {cx + dx - 20,   cy + dy},
            {cx + dx + 27,   cy + dy},
        };
        N nodes[7];
        float wave[7];
        for (int i = 0; i < 7; i++) {
            wave[i] = (sinf(t * 1.9f + i * 0.7f) + 1.0f) * 0.5f;
            nodes[i].x = base[i].x + cosf(t * 1.4f + i * 1.1f) * 3.5f * a;
            nodes[i].y = base[i].y + (wave[i] * 2.0f - 1.0f) * 4.5f * a;
        }
        int edges[][2] = {{0,1},{0,2},{1,3},{1,4},{2,5},{2,6}};
        for (auto& e : edges)
            DrawEdge(nodes[e[0]].x, nodes[e[0]].y,
                     nodes[e[1]].x, nodes[e[1]].y, WD);
        for (int i = 0; i < 7; i++) {
            float cf = wave[i] * a;
            Color fill   = lerpColor({255,255,255,180}, accBright, cf);
            Color border = lerpColor({255,255,255,80},  {accBright.r,accBright.g,accBright.b,160}, cf);
            DrawNode(nodes[i].x, nodes[i].y, nr, fill, border);
        }

    } else if (target == Screen::MST) {
        struct N { float x, y; };
        N base[] = {
            {cx - 70, cy - 30},
            {cx + 20, cy - 50},
            {cx + 80, cy - 10},
            {cx + 50, cy + 45},
            {cx - 20, cy + 48},
            {cx - 75, cy + 20},
        };
        N nodes[6];
        float wave[6];
        for (int i = 0; i < 6; i++) {
            wave[i] = (sinf(t * 1.9f + i * 1.1f) + 1.0f) * 0.5f;
            nodes[i].x = base[i].x + cosf(t * 1.3f + i * 1.5f) * 5.0f * a;
            nodes[i].y = base[i].y + sinf(t * 1.5f + i * 1.1f) * 4.5f * a;
        }
        int allEdges[][2] = {{0,1},{0,5},{1,2},{1,4},{2,3},{2,4},{3,4},{4,5}};
        for (auto& e : allEdges)
            DrawEdge(nodes[e[0]].x, nodes[e[0]].y,
                     nodes[e[1]].x, nodes[e[1]].y, WD);
        int mstEdges[][2] = {{0,1},{1,2},{2,3},{3,4},{4,5}};
        float pulse = 0.75f + 0.25f * sinf(t * 3.5f);
        Color mstC = {255, 180, 50, (unsigned char)(220 * (1.0f - a + a * pulse))};
        for (auto& e : mstEdges)
            DrawLineEx({nodes[e[0]].x, nodes[e[0]].y},
                       {nodes[e[1]].x, nodes[e[1]].y}, 2.5f, mstC);
        float nr = 5.5f;
        for (int i = 0; i < 6; i++) {
            float cf = wave[i] * a;
            Color fill   = lerpColor({255,255,255,180}, accBright, cf);
            Color border = lerpColor({255,255,255,80},  {accBright.r,accBright.g,accBright.b,160}, cf);
            DrawNode(nodes[i].x, nodes[i].y, nr, fill, border);
        }
    }
}
