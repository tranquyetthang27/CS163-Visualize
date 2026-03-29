#pragma once
#include "raylib.h"

// Call once after InitAudioDevice()
void AudioInit();
void AudioUnload();
void AudioUpdate();          // call every frame to stream music

void AudioPlayClick();       // UI button click
void AudioPlayHover();       // mouse hover on button
void AudioPlaySuccess();     // successful operation
void AudioPlayBack();        // back/exit navigation

void AudioSetMusicVolume(float v);
void AudioSetSFXVolume(float v);
