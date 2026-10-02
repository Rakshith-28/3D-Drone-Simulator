# 3D Drone Navigation Simulator

An interactive Computer Graphics mini-project written in C++ and OpenGL. The
simulator includes a hierarchical drone model, a lit 3D environment,
multiple camera modes, collision detection, animated propellers, waypoint
navigation, moving traffic and clouds, swaying trees, shadows, navigation
lights, atmospheric fog, and day/night modes.

## Build

Requirements: CMake 3.20+, a C++17 compiler, Git, and an OpenGL-capable system.
FreeGLUT is downloaded automatically during configuration.

On this Windows machine, portable CMake is installed in `.tools` and the
MSYS2 UCRT64 compiler is available on PATH. Build and run from PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

To launch the already built executable directly:

```powershell
.\build\drone_simulator.exe
```

```powershell
cmake -S . -B build
cmake --build build --config Release
./build/Release/drone_simulator.exe
```

For a single-configuration generator, the executable may instead be at
`build/drone_simulator.exe`.

## Controls

| Key | Action |
|---|---|
| `W` / `S` | Move forward / backward |
| `A` / `D` | Strafe left / right |
| `R` / `F` | Ascend / descend |
| `Q` / `E` | Rotate left / right |
| Touchpad/mouse drag | Hold left click and drag horizontally to rotate |
| `+` / `-` | Increase / decrease speed |
| Arrow keys | Orbit the camera |
| `C` | Cycle chase, cockpit, and overview cameras |
| `P` | Toggle automatic waypoint navigation |
| `G` | Toggle the ground grid |
| `N` | Toggle day/night lighting |
| `V` | Toggle atmospheric fog |
| `H` | Toggle help overlay |
| `Esc` | Exit |

The drone turns red when a requested movement would collide with an obstacle.
The cyan line and markers show the automatic route.
