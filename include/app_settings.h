#pragma once
#include "raylib.h"

struct AppSettings {
    float musicVolume  = 0.4f;
    float sfxVolume    = 0.7f;
    int   bgColorIdx   = 0;
    int   musicTrackIdx = 1;   // 0 = no music, 1 = music1, 2 = music2

    static const Color  bgColors[];
    static const char*  bgColorNames[];
    static const int    bgColorCount;

    Color GetBG() const { return bgColors[bgColorIdx]; }
};

extern AppSettings gSettings;
