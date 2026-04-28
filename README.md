# GhostHunter

Ghost Hunter is a simple FPS game project developed by C++ and OpenGL. Players of the game will act as ghost hunters, using a vacuum cleaner to capture ghosts in a haunted house.

## How to play

1. Player and Ghosts can not move in 5 seconds after game start.
2. Player can move with key 'w', 's', 'a', 'd' and control the view with mouse.
3. Player shoot with vacuum when he/she left click.
4. vacuum works if and only if the aiming ghost is in 5 meters.
5. if the ghost touched the player, game over, player lose.

## Build & Run

The project builds on **Windows**, **macOS** and **Linux** from a single
CMake configuration. The required runtime is OpenGL 3.3 Core, GLFW and
Assimp; `glad`, `glm`, `stb_image` are vendored inside `include/`.

### Quick start (any platform)

```sh
cmake -S . -B build
cmake --build build --config Release
```

The executable lands in `build/bin/GhostHunter/` and a copy of `res/` is
staged at `build/bin/res/` so the game can be launched directly. At
startup the program `chdir`s to its own directory, so it works regardless
of where you invoke it from.

### Platform notes

#### Windows

By default uses the prebuilt `glfw3.lib` / `assimp-vc143-mt.lib`
shipped under `lib/`. Tested with Visual Studio 2022 (toolset v143):

```sh
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

To opt out of the vendored binaries (e.g. for MinGW / clang-cl) add
`-DGHOSTHUNTER_USE_VENDORED_DEPS=OFF`; CMake will then either find an
installed GLFW/Assimp or fetch them via `FetchContent`.

#### macOS

```sh
brew install glfw assimp
cmake -S . -B build
cmake --build build --config Release
```

If Homebrew packages are not available, leave them out and CMake will
fetch GLFW and Assimp from source automatically.

#### Linux

```sh
sudo apt install libglfw3-dev libassimp-dev xorg-dev
cmake -S . -B build
cmake --build build
```

Without the system packages the same `cmake` command will pull GLFW
and Assimp from source via `FetchContent`.

### CMake options

| Option | Default | Notes |
| --- | --- | --- |
| `GHOSTHUNTER_USE_VENDORED_DEPS` | `ON` on Windows, `OFF` elsewhere | Use the prebuilt MSVC libs in `lib/`. |
| `GHOSTHUNTER_FETCH_DEPS` | `ON` | Fall back to `FetchContent` for any missing dependency. |

## Pictures

<img src="./screenshoots/screenshoot1.png" alt="GhostHunter1" style="zoom:53%;" />

<img src="./screenshoots/screenshoot2.png" alt="image-20231208104149685" style="zoom: 53%;" />

Lose the game

<img src="./screenshoots/lose.gif" alt="GhostHunter1" style="zoom: 500%;" />

Win the game

<img src="./screenshoots/win.gif" alt="GhostHunter1" style="zoom: 500%;" />



## Reference

[LearnOpenGL](https://github.com/JoeyDeVries/LearnOpenGL)

[Assimp](https://github.com/assimp/assimp)

[stb](https://github.com/nothings/stb)
