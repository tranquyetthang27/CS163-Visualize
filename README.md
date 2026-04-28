# Data Structure Visualization

## Introduction

**Data Structure Visualization** is an interactive learning tool designed to help students better understand fundamental data structures through graphical visualization. The application provides a user-friendly interface with audio support, making learning engaging and efficient.

### Team Members

| Student ID | Full Name |
|------------|-----------|
| 25125034 | Trần Quyết Thắng |
| 25125053 | Trần Quốc Huy |
| 25125058 | Nguyễn Sơn Lâm |
| 25125059 | Trần Thành Lợi |

## Features

- 📊 **Data Structure Visualization:**
  - Linked List
  - Trie (Prefix Tree)
  - Heap
  - Graph - Minimum Spanning Tree (MST)

- 🎨 **User Interface:**
  - Main menu with multiple options
  - Settings screen
  - About screen
  - Resizable window support

- 🔊 **Audio Management:** Integrated audio system to enhance user experience

- ⚙️ **Customization:** Application settings for personalized experience

## System Requirements

- **C++:** C++17 or later
- **CMake:** Version 3.25 or higher
- **Compiler:** g++ or MSVC (full support on Windows, Linux, macOS)
- **Library:** raylib (included in the project)

## Installation

### Step 1: Clone or Download the Project
```bash
cd d:\CS163-Visualize
```

### Step 2: Create Build Directory
```bash
mkdir build
cd build
```

### Step 3: Run CMake
```bash
cmake ..
```

### Step 4: Compile
On Windows:
```bash
cmake --build .
```

On Linux/macOS:
```bash
make
```

## Usage

### Running the Application
After compilation, execute:
```bash
./MyGame.exe          # Windows
./MyGame              # Linux/macOS
```

### Main Interface
- **Main Menu:** Select a data structure to explore
- **Settings:** Customize application settings
- **About:** View application information
- **Home:** Return to main screen

### Interaction
- Use mouse to interact with the interface
- Press ESC or buttons to return to previous menu

## Project Structure

```
.
├── src/                           # Main source code
│   ├── main.cpp
│   ├── about_screen.cpp
│   ├── app_settings.cpp
│   ├── audio_manager.cpp
│   ├── button.cpp
│   ├── camera.cpp
│   ├── card.cpp
│   ├── font.cpp
│   ├── graph_screen.cpp
│   ├── heap_screen.cpp
│   ├── home_screen.cpp
│   ├── init_file.cpp
│   ├── input_field.cpp
│   ├── linked_list_screen.cpp
│   ├── main_menu.cpp
│   ├── settings_screen.cpp
│   └── trie_screen.cpp
├── include/                       # Header files
│   ├── about_screen.h
│   ├── app_settings.h
│   ├── audio_manager.h
│   ├── button.h
│   ├── camera.h
│   ├── card.h
│   ├── colors.h
│   ├── font.h
│   ├── graph_screen.h
│   ├── heap_screen.h
│   ├── home_screen.h
│   ├── init_file.h
│   ├── input_field.h
│   ├── linked_list_screen.h
│   ├── main_menu.h
│   ├── screen.h
│   ├── settings_screen.h
│   └── trie_screen.h
├── assets/                        # Resources (audio, images, etc.)
│   └── music/
├── fonts/                         # Font files
├── raylib/                        # raylib library
├── build/                         # Build directory (created after CMake)
├── CMakeLists.txt                 # CMake configuration file
├── data.txt                       # Configuration data
└── README.md                      # This guide
```

## Technologies Used

- **C++17:** Primary programming language
- **raylib:** Graphics and audio library
- **CMake:** Build automation tool
- **Visual Studio Code:** Recommended code editor

## Development Guide

### Adding a New Data Structure
1. Create a header file in `include/`
2. Create corresponding implementation file in `src/`
3. Inherit from the `Screen` class
4. Add to `main.cpp`

### Rebuild Project
```bash
cd build
cmake ..
cmake --build .
```

## Notes

- Font files and assets are automatically copied to the executable directory during build
- Application runs at 1280x720 pixels by default and supports resizing
- Audio device must be initialized to use audio features

## AI Usage Declaration

This project used AI assistance in a limited and supervised way to support development. The team treated AI as a reference tool, not as an automatic source of final answers or implementation.

AI was mainly used for:

- Quickly checking raylib-related functions and usage patterns
- Understanding the overall project structure and file responsibilities
- Reviewing implementation ideas for the visualized data structure screens
- Helping the team verify whether a proposed approach matched the current codebase

AI was not used to make final project decisions. All core code, UI behavior, screen flow, and data structure visualization logic were reviewed, adjusted, and approved by the team before being kept in the project.

## License

This project uses the raylib library. Please see LICENSE in the raylib/ directory.

## Contact & Support

If you have any questions or feedback about the project, please contact the team members.