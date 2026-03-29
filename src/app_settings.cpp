#include "app_settings.h"

const Color AppSettings::bgColors[] = {
    {240, 244, 248, 255},   // Default (light blue-gray)
    {245, 245, 245, 255},   // White
    {20,  28,  48,  255},   // Dark navy
    {250, 245, 235, 255},   // Warm cream
    {232, 245, 233, 255},   // Mint
    {237, 231, 246, 255},   // Lavender
};
const char* AppSettings::bgColorNames[] = {
    "Default", "White", "Dark", "Warm", "Mint", "Lavender"
};
const int AppSettings::bgColorCount = 6;

AppSettings gSettings;
