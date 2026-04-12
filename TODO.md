# Shadow Box – project checklist

## Pre-production (COMP710 design phase)

- [ ] Game Pitch (1 page, non-technical)
- [ ] GDD (rules, controls, mock-ups, asset list, cheats, algorithms)
- [ ] TDD (UML, flow/state diagrams, debug plan, coding standards, file formats, 10 acceptance-test questions)
- [ ] GitHub repo + regular commits

## Phase 0 – GP base (current)

- [x] Folder layout: `docs/`, `assets/`, `lib/`, `game/`, `test/`, `temp/`, `source/`
- [x] `lib/` populated from **Studio5Resources** (SDL2 / SDL2_image / GLEW) + DLLs in `game/` + sprites in `assets/`
- [x] CMake + macOS build (links Homebrew; **includes** from `lib/` like the course)
- [x] `LogManager`, `Game`, `Renderer`, `main` – runnable from `game/` cwd
- [x] `game/shaders/` placeholders + VS Code launch cwd = `game/`
- [ ] Full sprite/shader pipeline (next)

## Production – vertical slice (stealth MVP)

- [ ] WASD screen-relative movement + walls
- [ ] One enemy: patrol + vision cone + detection
- [ ] Collect + extract; lose on catch; **O** restart
- [ ] Fullscreen default, splash, instructions
- [ ] Audio; data files for patrols/stats
- [ ] Xbox 360 controller mapping
- [ ] Enemy types + hearing + 5s “still” rule

## Milestones & Windows handoff

- [ ] Windows: `.sln`, Win32 Release/Debug, zip layout per brief
- [ ] FWT + Gold deliverables, GitHub log, readme/post-mortem
