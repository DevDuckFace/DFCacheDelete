# DFCacheDelete 🦆

**DFCacheDelete** is a modern, high-performance C++ utility designed to find and delete cache folders recursively on Windows. It features a clean, dark-themed UI and ensures safety with explicit deletion handling.

![DFCacheDelete Screenshot](screenshot.png)

## Features

- **Recursive Scanning**: Efficiently finds all folders named "cache" (case-insensitive).
- **Smart Filtering**: Configurable minimum size (default: 50 MB) to ignore small, insignificant folders.
- **Favorites System**: Star your frequently accessed cache locations to keep them pinned.
- **Dark Mode**: A beautiful, custom-styled Qt user interface.
- **Safety First**: No automatic deletions. You select what to delete, and every action is confirmed.
- **Symlink Protection**: Automatically ignores symbolic links to prevent accidental system damage.

## Tech Stack

- **Language**: C++20
- **Framework**: Qt 6 Widgets
- **Build System**: CMake

## Build Instructions

1.  Ensure you have **CMake**, **Qt 6**, and a C++ compiler (MSVC or MinGW) installed.
2.  Clone the repository:
    ```bash
    git clone https://github.com/yourusername/DFCacheDelete.git
    cd DFCacheDelete
    ```
3.  Create a build directory:
    ```bash
    mkdir build
    cd build
    ```
4.  Configure and build:
    ```bash
    cmake ..
    cmake --build . --config Release
    ```
5.  Run:
    ```bash
    ./DFCacheDelete.exe
    ```

## Installation

You can download the latest installer from the [Releases](https://github.com/yourusername/DFCacheDelete/releases) page (if available) or build the installer yourself using the provided Inno Setup script (`installer.iss`).

## License

This project is open-source. Feel free to modify and distribute.
