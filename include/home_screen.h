#pragma once
#include "screen.h"
#include "card.h"
#include "button.h"

class HomeScreen {
    Card   cards[4];
    Button btnBack;
public:
    HomeScreen();
    Screen Update();
    void   Draw() const;
};
