# Boxu / COMP710 – step log

Use this as a running checklist. Update after each milestone (and match your Git commit messages).

| Step | Date | What we did | How to verify | Git commit (short) |
|------|------|-------------|---------------|--------------------|
| 0 | | Repo layout: `Boxu/` = game, `docs/`, `scripts/` | Folders exist | |
| 1 | | Imported teacher `GPFrameworkCodeBase` into `Boxu/source/` + shaders in `Boxu/game/shaders/` | `ls Boxu/source/*.cpp` | |
| 2 | | Copied `Studio5Resources` SDKs → `Boxu/lib/`, DLLs → `Boxu/game/` | `ls Boxu/lib` | |
| 3 | | macOS: fixed `__int64` → `std::int64_t` in `game.h` | Builds on Mac | |
| 4 | | Player sprite + WASD (float position, `SDL_PumpEvents`) | Run from `Boxu/game/`, move smoothly | |
| 5 | | CMake Win32 path + `GL/glew.h` shim (Windows submission) | Build on Windows PC (see `WINDOWS_BUILD.md`) | |
| 6 | | Docs: Git beginner guide, GitHub log script | Read `GIT_BEGINNER.md` | |
| 7 | | Renamed parent folder `comp710 game` → `comp710Game` (no space) | Project path updated in `GIT_BEGINNER.md` | |
| 8 | | World map (2560×1920), camera follows player, clamped to map edges; diagonal WASD normalized | Run from `Boxu/game/`, move to edges — view scrolls, stops at map border | |
| 9 | | Mouse-aim facing: world-space cursor, shortest-path turn capped (~220°/s), `+90°` sprite alignment constant | Run from `Boxu/game/` — WASD + move mouse; sprite eases toward aim, not instant snap | |
| 10 | | Brown-box placeholder player, explicit hitbox half extents, debug hitbox overlay toggle on `H` | Run from `Boxu/game/` — player appears brown; press `H` to show/hide red hitbox | |
| | | *Next: walls from data file + collision* | | |
| | | *Next: rename exe to hand-in `codename.exe` (optional)* | | |

## Run commands (Mac)

```bash
cd "…/GameProject23196422/Boxu/source"
cmake -S . -B ../temp/build-boxu -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build ../temp/build-boxu

cd ../game
../temp/build-boxu/gpframework
```

Working directory must be **`Boxu/game/`** (textures + shaders paths).

If CMake says the cache was created under a **different path** (e.g. after renaming `comp710 game` → `comp710Game`), delete the old build tree and reconfigure: `rm -rf Boxu/temp/build-boxu`, then run the `cmake -S` / `cmake --build` lines again.

## Submission reminder

FWT zip expects **`codename.exe`** and **`codename_debug.exe`** in `game/` — before zip, copy/rename your built Windows binaries to match the brief (or set CMake output name to your codename).
