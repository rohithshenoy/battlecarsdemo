# AGENTS.md

## Cursor Cloud specific instructions

### What this repo is
A single product: **SpaceInvaders**, a native C++17 / OpenGL 3.3 desktop game
(GLFW for windowing, glad for the GL loader) built with **CMake** + `FetchContent`.
There is no web/JS component, no backend, no database, and no test suite. Source
lives in `src/` (`main.cpp`, `Game.cpp`, `Shader.cpp`, `src/shaders/*.glsl`).
CLI feature: `--spaceship-color=r,g,b` (each component `0.0`–`1.0`).

### Build/run needs network
`cmake` configure clones GLFW 3.3.9 and glad v0.1.36 via `FetchContent`, and the
glad generator (`python3 -m glad`) downloads `gl.xml` from GitHub at build time.
Network access is required for the first configure/build.

### The stock build commands do NOT work as-is on this toolchain
`cmake -S . -B build && cmake --build build` fails. Three independent issues:
1. glad v0.1.36 (Dav1dde generator) produces `glad.c` **at build time** inside the
   build tree, but `CMakeLists.txt` references `${glad_SOURCE_DIR}/src/glad.c`,
   which must already exist **at configure time**. So the loader must be
   pre-generated into the fetched `glad-src` before configuring.
2. `project(SpaceInvaders LANGUAGES CXX)` never enables the C language, but the
   `glad_lib` target compiles a C file → configure/generate fails with
   `CMAKE_C_COMPILE_OBJECT not set`. C must be enabled.
3. `Game.cpp` calls GL 3.0+ functions (`glBindVertexArray`, `glDrawArrays`) but
   only includes `<GLFW/glfw3.h>`, not `<glad/glad.h>`. On the default `c++`
   (clang selecting the GCC 14 stdlib) + Mesa headers, those symbols are
   undeclared unless `GL_GLEXT_PROTOTYPES` is defined.

These are latent source/build-config issues (not env). Do not modify tracked
source for environment setup; instead use the build-tree/flag-only recipe below,
which produces a working binary without editing repo code. Also note: the
committed `build/` directory is a stale/failed configure — build into a fresh dir.

### Working build recipe (no source edits)
```bash
BUILD=/tmp/si-build
rm -rf "$BUILD"
# Pass 1: fetch GLFW + glad sources (expected to fail at the missing glad.c).
cmake -S . -B "$BUILD" || true
# Pre-generate the glad loader into the fetched source tree.
python3 -m glad --generator=c --profile=core --api="gl=3.3" --spec=gl \
  --out-path="$BUILD/_deps/glad-src"
# Enable C for the glad_lib target without editing CMakeLists.txt.
echo 'enable_language(C)' > /tmp/inject_c.cmake
# Pass 2: real configure (C enabled + GL prototypes for Game.cpp).
cmake -S . -B "$BUILD" \
  -DCMAKE_PROJECT_SpaceInvaders_INCLUDE=/tmp/inject_c.cmake \
  -DCMAKE_CXX_FLAGS="-DGL_GLEXT_PROTOTYPES"
cmake --build "$BUILD" -j"$(nproc)"   # -> $BUILD/SpaceInvaders
```

### Running headless (no physical display)
Use Xvfb + Mesa software rendering:
```bash
export DISPLAY=:99
Xvfb :99 -screen 0 800x600x24 >/tmp/xvfb.log 2>&1 &
LIBGL_ALWAYS_SOFTWARE=1 /tmp/si-build/SpaceInvaders --spaceship-color=1.0,0.0,0.0
```
The game loop ends on its own (enemies reach the bottom) or on ESC; there is no
keyboard input under Xvfb, so the player bar stays put. Capture frames/video with
`ffmpeg -f x11grab -video_size 800x600 -i :99 ...`.

### Lint / test
No linter config and no tests exist. "Lint" is just the compiler warnings enabled
in `CMakeLists.txt` (`-Wall -Wextra -Wpedantic`); a clean `cmake --build` covers it.
