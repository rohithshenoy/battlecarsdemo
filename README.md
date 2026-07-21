# battlecarsdemo

A simple OpenGL Space Invaders-style game built with C++17, GLFW, and GLAD. It demonstrates Cursor Cloud Agents capabilities with a playable desktop window.

## Prerequisites

| Requirement | Minimum version | Notes |
|-------------|-----------------|-------|
| CMake | 3.16 | Used to configure and generate build files |
| C++ compiler | C++17 support | GCC 8+, Clang 7+, or MSVC 2017+ |
| Python 3 | 3.6+ | Required at configure time to generate OpenGL bindings via GLAD |
| OpenGL | 3.3 Core | A GPU with a working OpenGL 3.3 driver |
| Git | any recent version | CMake downloads GLFW and GLAD via `FetchContent` |

GLFW and GLAD are fetched automatically during the CMake configure step. You do not need to install them separately.

### Linux (Debian / Ubuntu)

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  git \
  python3 \
  libgl1-mesa-dev \
  libx11-dev \
  libxrandr-dev \
  libxinerama-dev \
  libxcursor-dev \
  libxi-dev
```

### macOS

Install Xcode Command Line Tools, then use Homebrew:

```bash
xcode-select --install
brew install cmake python3
```

### Windows

Install the following:

- [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload
- [CMake](https://cmake.org/download/)
- [Python 3](https://www.python.org/downloads/) (make sure `python` is on your `PATH`)
- [Git for Windows](https://git-scm.com/download/win)

## Building

### 1. Clone the repository

```bash
git clone <repository-url>
cd battlecarsdemo
```

### 2. Configure with CMake

Create an out-of-source build directory and run CMake from there:

```bash
cmake -S . -B build
```

This step:

1. Downloads [GLFW 3.3.9](https://github.com/glfw/glfw) and [GLAD v0.1.36](https://github.com/Dav1dde/glad)
2. Runs the GLAD generator to produce OpenGL 3.3 Core bindings (requires network access on first configure)
3. Generates platform-specific build files in `build/`

#### Optional CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | platform default | Set to `Debug` or `Release` on single-config generators (Make, Ninja) |

Example release build on Linux/macOS:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

Example with Visual Studio generator:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
```

### 3. Compile

```bash
cmake --build build
```

On Windows with the Visual Studio generator, you can also open the generated solution in `build/` and build from the IDE.

### 4. Locate the executable

After a successful build, the game binary is at:

| Platform | Path |
|----------|------|
| Linux / macOS | `build/SpaceInvaders` |
| Windows | `build/Release/SpaceInvaders.exe` (or `build/Debug/SpaceInvaders.exe` depending on configuration) |

## Running

From the repository root:

```bash
./build/SpaceInvaders
```

On Windows:

```powershell
.\build\Release\SpaceInvaders.exe
```

The game opens an 800×600 window. You need a graphical desktop session with OpenGL support (it will not run headless).

### Command-line options

You can set the spaceship color at launch with `--spaceship-color=r,g,b`, where each component is between `0.0` and `1.0`:

```bash
./build/SpaceInvaders --spaceship-color=1.0,0.0,0.0
```

An alternate form is also accepted:

```bash
./build/SpaceInvaders --spaceship-color 0.2,0.9,0.3
```

If no color is specified, the default green (`0.2, 0.9, 0.3`) is used.

Choose a procedural space background with `--background=<theme>`:

| Theme | Aliases | Look |
|-------|---------|------|
| `classic` | `black` | Solid black (default) |
| `nebula` | `stars`, `space` | Deep space with stars and colorful nebulae |
| `neon` | `synthwave`, `80s`, `retro` | 1980s neon sky with sun bands and a perspective grid |
| `aurora` | | Curtains of green/purple northern lights over stars |
| `deep-space` | `deepspace`, `deep` | Dark blue void with sparse stars and mist |

```bash
./build/SpaceInvaders --background=nebula
./build/SpaceInvaders --background neon --spaceship-color=1.0,0.2,0.8
```

## Controls

| Key | Action |
|-----|--------|
| Left / A | Move left |
| Right / D | Move right |
| Space (hold) | Fire |
| Shift + Space (hold) | Turbo fire — rapid shots, faster bullets (orange) |
| Escape | Quit |

## Project structure

```
battlecarsdemo/
├── CMakeLists.txt          # Build configuration and dependency fetching
├── src/
│   ├── main.cpp            # Entry point, window setup, game loop
│   ├── Game.cpp / Game.h   # Game logic, input, rendering
│   ├── Shader.cpp / Shader.h
│   └── shaders/
│       ├── vertex.glsl
│       └── fragment.glsl
└── build/                  # Generated build output (created by you)
```

## Troubleshooting

### `Failed to initialize GLFW` or window creation errors

- Confirm you are running in a graphical environment (not SSH without X11 forwarding).
- On Linux, ensure the X11 development libraries listed above are installed.

### CMake cannot find Python

GLAD generates OpenGL loader code during the configure step and needs Python 3:

```bash
python3 --version   # Linux / macOS
python --version    # Windows
```

Install Python 3 if it is missing, then re-run `cmake -S . -B build`.

### GLAD generation fails (network error)

The first configure downloads the Khronos OpenGL specification from GitHub. If you are offline or behind a restrictive firewall, configure will fail until network access is available.

### Stale build directory after moving the project

If you moved or copied the repository to a new path, delete the old build directory and reconfigure:

```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

### `Cannot find source file ... glad.c`

This usually means the build directory is in a bad state. Remove `build/` and run a fresh configure as shown above.

## Cleaning

To remove all build artifacts:

```bash
rm -rf build
```

On Windows (PowerShell):

```powershell
Remove-Item -Recurse -Force build
```
