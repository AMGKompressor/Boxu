# Building Boxu on Windows (for submission)

You can keep developing on **Mac**; before **FWT / Gold**, build on **Windows** and drop **`codename.exe`** + **`codename_debug.exe`** into `game/` per the brief.

## What you need on Windows

- **Visual Studio 2022** with **Desktop development with C++**
- **CMake** (optional if you open the generated solution only)

Your project already includes **`Boxu/lib/`** with **SDL2 / SDL2_image / GLEW** (**Win32 x86** `.lib` files) — same layout as Studio 5.

## Configure (Win32)

In **Command Prompt** or **PowerShell** (adjust paths):

```bat
cd path\to\GameProject23196422\Boxu\source
cmake -G "Visual Studio 17 2022" -A Win32 -B ..\temp\build-win
```

`-A Win32` matches the course **32-bit** expectation.

## Build

```bat
cmake --build ..\temp\build-win --config Release
cmake --build ..\temp\build-win --config Debug
```

Executables will be under something like:

- `Boxu\temp\build-win\Release\gpframework.exe`
- `Boxu\temp\build-win\Debug\gpframework.exe`

## Run (important)

1. Copy **`SDL2.dll`**, **`SDL2_image.dll`**, **`glew32.dll`** next to the `.exe` **or** run with cwd = **`Boxu\game`** where those DLLs already live.
2. **Working directory** should be **`Boxu\game`** so `shaders\` and `textures\` load (same as Mac).

Example from `Boxu\game`:

```bat
..\temp\build-win\Release\gpframework.exe
```

## Hand-in names

The brief asks for **`codename.exe`** (Release) and **`codename_debug.exe`** (Debug). You can:

- **Rename** `gpframework.exe` → `boxu.exe` (or your codename), **or**
- Set CMake `set_target_properties(gpframework PROPERTIES OUTPUT_NAME boxu)` and build `boxu.exe` / `boxu_debug.exe` (needs per-config naming for `_debug` — we can add that when you pick final exe names).

Put the two exes + all assets + DLLs inside the submission **`game/`** folder.

## Mac + Windows together

- **Same source** in `Boxu/source/`; commit from Mac.
- On Windows: **pull** from GitHub, run CMake + build, test, then zip for Canvas.
