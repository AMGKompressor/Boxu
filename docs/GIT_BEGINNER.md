# Git for beginners (this project)

Goal: **save versions of your work**, push to **GitHub**, and generate **`GHL – Student ID.txt`** for COMP710.

## 1. Install Git (Mac)

Git is usually already installed. Check:

```bash
git --version
```

If missing: `xcode-select --install` (Command Line Tools).

## 2. Tell Git who you are (once per machine)

```bash
git config --global user.name "Your Name"
git config --global user.email "you@example.com"
```

Use the **same email** you will use on GitHub (GitHub account settings).

## 3. Create the repo in your project folder

Pick **one** folder as the repo root. Recommended: the folder that contains **`Boxu/`**, **`docs/`**, **`scripts/`**:

```bash
cd "/Users/bardiakhalifeh/Downloads/comp710Game/GameProject23196422"
git init
```

## 4. First save (“commit”)

```bash
git status          # see what changed
git add .
git commit -m "Initial commit: Boxu GP framework and docs"
```

If Git says “nothing to commit”, you may need to add files or fix `.gitignore`.

## 5. Put it on GitHub

1. On [github.com](https://github.com): **New repository** (empty, no README if you already have files locally).
2. Copy the HTTPS URL, e.g. `https://github.com/you/boxu-comp710.git`
3. In Terminal:

```bash
cd "/Users/bardiakhalifeh/Downloads/comp710Game/GameProject23196422"
git remote add origin https://github.com/YOU/YOUR_REPO.git
git branch -M main
git push -u origin main
```

You may need to log in (browser or token). If GitHub asks for a **Personal Access Token**, use that instead of a password.

## 6. Daily workflow (the part you repeat)

After you finish a small feature or fix:

```bash
cd "/Users/bardiakhalifeh/Downloads/comp710Game/GameProject23196422"
git status
git add .
git commit -m "Short message: what you changed"
git push
```

**Good messages:** `Add player movement with WASD`, `Fix Windows CMake for Win32 libs`  
**Bad messages:** `asdf`, `update`, `fix`

## 7. Generate the GitHub log file (for FWT / Gold)

After you have commits:

```bash
cd "/Users/bardiakhalifeh/Downloads/comp710Game/GameProject23196422"
./scripts/export-github-log.sh YOUR_STUDENT_ID
```

This creates `docs/GHL – YOUR_STUDENT_ID.txt`. Put that file inside your submission zip under **`docs/`**.

## 8. If something goes wrong

- **`git status`** – always run this first.  
- **Undo changes to a file (before commit):**  
  `git checkout -- path/to/file`  
- **See history:**  
  `git log --oneline`  

For anything scary (lost commits), stop and ask before running random commands from the internet.

## 9. Optional: private repo

For coursework, a **private** GitHub repo is fine; you still submit the **zip + GHL** to Canvas.
