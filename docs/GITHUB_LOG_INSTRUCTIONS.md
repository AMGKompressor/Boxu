# GitHub log (`GHL – Student ID.txt`) – how to create it

## Why

The COMP710 brief asks for a text file with **all GitHub commits and associated messages**. This proves incremental development.

## Prerequisite: Git + GitHub

1. In your project root (the folder that will become the repo root – e.g. `GameProject23196422` or your `Boxu` root if you use only that):

   ```bash
   git init
   ```

2. Add a `.gitignore` (build folders, OS junk) – already in repo root if present.

3. Create a **private** (or public) repo on GitHub, then:

   ```bash
   git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO.git
   git branch -M main
   git add .
   git commit -m "Initial commit: Boxu / GP framework baseline"
   git push -u origin main
   ```

4. After that, **commit often** with clear messages, e.g. `Add player WASD movement`, `Add guard patrol state`, not `fix`.

## Generate the log file

From the **repository root** (same folder as `.git`):

```bash
chmod +x scripts/export-github-log.sh
./scripts/export-github-log.sh YOUR_STUDENT_ID
```

Or manually:

```bash
mkdir -p docs
git log --reverse --pretty=format:"%h | %ad | %s%n%b%n" --date=iso > "docs/GHL – YOUR_STUDENT_ID.txt"
```

- `%h` – short hash  
- `%ad` – date  
- `%s` – subject  
- `%b` – body (extra lines if you use multi-line commits)

## Filenames for the zip

Match Canvas **exactly** (spaces, dashes). Typical:

- `docs/GHL – 23196422.txt` (replace with your ID)

## FWT vs Gold

- **FWT:** Include the log that covers work **up to FWT** (or full log to date – usually fine).
- **Gold:** Refresh the log so it includes **everything** through final commit before zip.

## Tip

Before each milestone zip:

```bash
git pull
./scripts/export-github-log.sh YOUR_STUDENT_ID
```

Then copy `docs/GHL – …` into your submission zip under `docs/`.
