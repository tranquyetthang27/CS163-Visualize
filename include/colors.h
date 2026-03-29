#pragma once
#include "raylib.h"

namespace Pal {
    constexpr Color BG           = {240, 244, 248, 255};
    constexpr Color Surface      = {255, 255, 255, 255};
    constexpr Color Border       = {210, 220, 230, 255};

    // Card accent colors
    constexpr Color Indigo       = {63,  81,  181, 255};
    constexpr Color IndigoDark   = {48,  63,  159, 255};
    constexpr Color Teal         = {0,   172, 193, 255};
    constexpr Color TealDark     = {0,   131, 143, 255};
    constexpr Color Amber        = {251, 192, 45,  255};
    constexpr Color AmberDark    = {245, 167, 0,   255};
    constexpr Color Coral        = {229, 57,  53,  255};
    constexpr Color CoralDark    = {198, 40,  40,  255};

    // Text
    constexpr Color TxtDark      = {28,  35,  51,  255};
    constexpr Color TxtMid       = {80,  95,  115, 255};
    constexpr Color TxtLight     = {148, 163, 183, 255};

    // Buttons
    constexpr Color BtnPrimary   = {63,  81,  181, 255};
    constexpr Color BtnPrimHov   = {48,  63,  159, 255};
    constexpr Color BtnDanger    = {229, 57,  53,  255};
    constexpr Color BtnDangHov   = {198, 40,  40,  255};
    constexpr Color BtnSuccess   = {46,  160, 67,  255};
    constexpr Color BtnSuccHov   = {36,  128, 54,  255};
    constexpr Color BtnNeutral   = {148, 163, 183, 255};
    constexpr Color BtnNeutHov   = {120, 138, 160, 255};
    constexpr Color BtnOrange    = {245, 124, 0,   255};
    constexpr Color BtnOrangeHov = {230, 100, 0,   255};

    // Node states
    constexpr Color NodeFill     = {255, 255, 255, 255};
    constexpr Color NodeBorder   = {63,  81,  181, 255};
    constexpr Color NodeHL       = {255, 213, 79,  255};
    constexpr Color NodeFound    = {76,  175, 80,  255};
    constexpr Color NodeRemove   = {229, 57,  53,  255};

    // Edges
    constexpr Color EdgeColor    = {190, 205, 220, 255};
    constexpr Color MSTEdge      = {255, 152, 0,   255};

    // Input
    constexpr Color InputBorder  = {180, 195, 210, 255};
    constexpr Color InputFocus   = {63,  81,  181, 255};

    // Panel
    constexpr Color Panel        = {250, 252, 255, 255};
    constexpr Color PanelDark    = {235, 240, 246, 255};
}
