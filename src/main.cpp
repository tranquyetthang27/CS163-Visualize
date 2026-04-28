#include "raylib.h"
#include "font.h"
#include "screen.h"
#include "app_settings.h"
#include "audio_manager.h"
#include "main_menu.h"
#include "settings_screen.h"
#include "about_screen.h"
#include "home_screen.h"
#include "linked_list_screen.h"
#include "trie_screen.h"
#include "heap_screen.h"
#include "graph_screen.h"

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Data Structure Visualization");
    SetTargetFPS(120);
    InitAudioDevice();

    LoadFonts();
    AudioInit();

    Screen current = Screen::MainMenu;

    MainMenu         mainMenu;
    SettingsScreen   settingsScreen;
    AboutScreen      aboutScreen;
    HomeScreen       homeScreen;
    LinkedListScreen llScreen;
    TrieScreen       trieScreen;
    HeapScreen       heapScreen;
    GraphScreen      mstScreen;

    while (!WindowShouldClose()) {
        AudioUpdate();

        // Update 
        Screen next = current;
        switch (current) {
            case Screen::MainMenu:   next = mainMenu.Update();        break;
            case Screen::Settings:   next = settingsScreen.Update();  break;
            case Screen::About:      next = aboutScreen.Update();     break;
            case Screen::Home:       next = homeScreen.Update();      break;
            case Screen::LinkedList: next = llScreen.Update();        break;
            case Screen::Trie:       next = trieScreen.Update();      break;
            case Screen::Heap:       next = heapScreen.Update();      break;
            case Screen::MST:        next = mstScreen.Update();       break;
        }

        if (current == Screen::MainMenu) {
        }

        // Re-construct screens on transition
        if (next != current) {
            switch (next) {
                case Screen::Settings:   settingsScreen = SettingsScreen();   break;
                case Screen::About:      aboutScreen    = AboutScreen();      break;
                case Screen::Home:       homeScreen     = HomeScreen();       break;
                // Data structure screens preserve their state when navigating back
                default: break;
            }
            current = next;
        }

        // Draw
        BeginDrawing();
        switch (current) {
            case Screen::MainMenu:   mainMenu.Draw();        break;
            case Screen::Settings:   settingsScreen.Draw();  break;
            case Screen::About:      aboutScreen.Draw();     break;
            case Screen::Home:       homeScreen.Draw();      break;
            case Screen::LinkedList: llScreen.Draw();        break;
            case Screen::Trie:       trieScreen.Draw();      break;
            case Screen::Heap:       heapScreen.Draw();      break;
            case Screen::MST:        mstScreen.Draw();       break;
        }
        EndDrawing();
    }

    AudioUnload();
    UnloadFonts();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
