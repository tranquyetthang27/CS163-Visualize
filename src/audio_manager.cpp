#include "audio_manager.h"
#include "app_settings.h"
#include <cmath>
#include <cstdio>

#ifndef PI
#define PI 3.14159265358979f
#endif

static Sound sndClick;
static Sound sndHover;
static Sound sndSuccess;
static Sound sndBack;
static Music bgMusic;
static bool  musicLoaded = false;

// Generate a short sine-wave sound
static Sound MakeSound(float freqHz, float durSec, float amp, float freqEnd = -1.0f) {
    int rate  = 44100;
    int count = (int)(rate * durSec);

    Wave w;
    w.frameCount = (unsigned int)count;
    w.sampleRate = (unsigned int)rate;
    w.sampleSize = 16;
    w.channels   = 1;
    w.data       = MemAlloc(sizeof(short) * count);

    short* d = (short*)w.data;
    if (freqEnd < 0) freqEnd = freqHz;

    for (int i = 0; i < count; i++) {
        float t    = (float)i / rate;
        float frac = (float)i / count;
        float freq = freqHz + (freqEnd - freqHz) * frac;
        float env  = (1.0f - frac) * (1.0f - frac);   // quadratic decay
        d[i] = (short)(sinf(2.0f * PI * freq * t) * env * amp * 32767.0f);
    }

    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);
    return s;
}

void AudioInit() {
    // Click: short high tone
    sndClick   = MakeSound(900.0f,  0.07f, 0.5f);
    // Hover: very soft low tone
    sndHover   = MakeSound(600.0f,  0.04f, 0.25f);
    // Success: rising two-tone "ding"
    sndSuccess = MakeSound(660.0f,  0.18f, 0.55f, 990.0f);
    // Back: falling tone
    sndBack    = MakeSound(700.0f,  0.10f, 0.40f, 440.0f);

    // Load background music based on saved track selection
    AudioSetMusicTrack(gSettings.musicTrackIdx);
}

void AudioUnload() {
    UnloadSound(sndClick);
    UnloadSound(sndHover);
    UnloadSound(sndSuccess);
    UnloadSound(sndBack);
    if (musicLoaded) UnloadMusicStream(bgMusic);
}

void AudioUpdate() {
    if (musicLoaded) {
        UpdateMusicStream(bgMusic);
        // If stream stopped unexpectedly (e.g. buffer underrun), restart it
        if (!IsMusicStreamPlaying(bgMusic)) {
            PlayMusicStream(bgMusic);
        }
    }
}

static void PlaySFX(Sound& s) {
    SetSoundVolume(s, gSettings.sfxVolume);
    PlaySound(s);
}

void AudioPlayClick()   { PlaySFX(sndClick);   }
void AudioPlayHover()   { PlaySFX(sndHover);   }
void AudioPlaySuccess() { PlaySFX(sndSuccess); }
void AudioPlayBack()    { PlaySFX(sndBack);    }

void AudioSetMusicVolume(float v) {
    gSettings.musicVolume = v;
    if (musicLoaded) SetMusicVolume(bgMusic, v);
}

void AudioSetSFXVolume(float v) {
    gSettings.sfxVolume = v;
}

void AudioSetMusicTrack(int idx) {
    gSettings.musicTrackIdx = idx;
    if (musicLoaded) {
        StopMusicStream(bgMusic);
        UnloadMusicStream(bgMusic);
        musicLoaded = false;
    }
    if (idx == 0) return;  // no music

    const char* relPaths[] = {
        nullptr,
        "assets/music/music1.mp3",
        "assets/music/music2.mp3",
    };
    
    if (idx >= 1 && idx <= 2) {
        // Build absolute path from executable directory so it works
        // regardless of the current working directory.
        const char* appDir = GetApplicationDirectory();
        char absPath[512];
        snprintf(absPath, sizeof(absPath), "%s%s", appDir, relPaths[idx]);

        if (FileExists(absPath)) {
            bgMusic     = LoadMusicStream(absPath);
            musicLoaded = true;
            SetMusicVolume(bgMusic, gSettings.musicVolume);
            PlayMusicStream(bgMusic);
        }
    }
}