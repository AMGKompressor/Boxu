# COMP710 – what documentation goes where

Use this so your **portfolio (Word)** and **official submissions** stay separate.

## First-Walk-and-Talk (FWT) zip

Per the brief, inside `FWT - Student ID.zip` you need (among other items):

| Item | Notes |
|------|--------|
| `Coversheet - Student ID.pdf` | From Canvas |
| `docs/Pitch - Student ID.pdf` | Game pitch (1 page, non-technical) |
| `docs/GDD - Student ID.pdf` | Game Design Document |
| `docs/TDD - Student ID.pdf` | Technical Design Document |
| `docs/GHL – Student ID.txt` | **GitHub log** – all commits + messages (see below) |
| `game/` | Release + Debug exes, all assets |
| `source/` | `.sln` + source files |
| `lib/` | Third-party if needed |
| `test/` | Test / cheat notes |

Your **personal portfolio Word doc** is **not** a substitute for these PDFs unless a lecturer says otherwise. Keep the portfolio for employers; hand in the **required PDFs** for marks.

## Gold submission zip

Same structure plus **readme.pdf** and **Post-mortem** in `docs/`, and an updated **GHL** for Gold (often one log covering the whole project, or follow whatever Canvas says exactly).

## GitHub log (`GHL`)

Markers want a **text file** listing commits and messages so they can see steady progress. Generate it from Git **after every meaningful commit** (or right before zip submission).

See: `docs/GITHUB_LOG_INSTRUCTIONS.md`, `docs/GIT_BEGINNER.md`, and `scripts/export-github-log.sh`.

**Windows hand-in:** `docs/WINDOWS_BUILD.md`  
**Step-by-step dev log:** `docs/STEP_LOG.md`

## Next steps (together, step by step)

1. **Git** – init repo, `.gitignore`, remote, first commit (if not done).
2. **GHL** – run export script → save as `docs/GHL – YOURID.txt`.
3. **Pitch** – one page in Word → export **PDF** with correct filename.
4. **GDD** – sections from brief, then PDF.
5. **TDD** – diagrams + 10 acceptance questions, then PDF.

We’ll tackle one item per step when you say which one is next.
