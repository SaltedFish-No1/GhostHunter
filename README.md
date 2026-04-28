# GhostHunter

Ghost Hunter is a simple FPS game project developed by C++ and OpenGL . Players of the game will act as ghost hunters, using a vacuum cleaner to capture ghosts in a haunted house. 

## How to play

1. Player and Ghosts can not move in 5 seconds after game start.
2. Player can move with key 'w', 's', 'a', 'd' and control the view with mouse.
3. Player shoot with vacuum when he/she left click.
4. vacuum works if and only if the aiming ghost is in 5 meters.
5. if the ghost touched the player, game over, player lose.

## How to build&run

### Prebuilt (Windows)

Run `./Excutable/GhostHunter/GhostHunter.exe`.

### Visual Studio 2022 (Windows)

Open `GhostHunter.sln` and build.

### CMake

```sh
cmake -S . -B build
cmake --build build --config Release
```

The executable lands in `build/bin/GhostHunter/` and resources are staged
under `build/bin/res/` so the exe can be launched directly.

- **Windows**: uses the vendored `include/` and `lib/` shipped in the repo;
  no external dependencies required.
- **Linux / macOS**: install `glfw` and `assimp` via your package manager
  (e.g. `apt install libglfw3-dev libassimp-dev`, `brew install glfw assimp`)
  before configuring.

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
