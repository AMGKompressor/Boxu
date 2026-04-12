# Studio5Resources smoke test

This folder is **not** part of Shadow Box. It is a **tiny program** that only checks:

- Headers and link setup for **SDL2**, **SDL2_image**, and **GLEW** from `../lib/` (same layout as `Studio5Resources`).

There is **no** `Game`, `Renderer`, or gameplay code here.

## Run (after building)

From the **repository root** (cwd does not matter for this exe):

```bash
./build/studio5_smoketest
```

On Windows, run `build-win\\Debug\\studio5_smoketest.exe` (or `Release`) from Explorer or a terminal.

You should see a **green** window (different colour from Shadow Box’s cyan). **Esc** or close to quit.
