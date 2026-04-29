Place WAV files here so CMake copies them next to `gpframework` under `sounds/`:

  thud.wav              — wall hit (optional if you only care about footsteps)
  walkTemp1.wav …       — footstep clips (try .WAV if needed)
  walkTemp4.wav

The game loads these using SDL_GetBasePath() so they resolve next to the
executable even when you run the app from another working directory.

After adding files, rebuild so POST_BUILD copies them into temp/build-*/sounds/.
