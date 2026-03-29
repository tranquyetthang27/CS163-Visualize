#pragma once
#include "screen.h"
#include "button.h"

class AboutScreen {
    Button btnBack;
public:
    AboutScreen();
    Screen Update();
    void   Draw() const;
};
