# Architecture

Git Gud Gym has three moving parts: the **hub app** you run, a **game module** that lives inside the game, and the
**catalog** that lists which modules exist.

```diagram
architecture
```

## The hub app

`GitGudGym.exe` is a native Windows app written in C++20. Its UI is built with Dear ImGui on Direct3D 11, and the
in-game notifications and HUD are drawn by a separate overlay using GDI+.

The app:

- finds the game on your PC and launches it offline,
- refuses to attach while Easy Anti-Cheat is running,
- loads the game's module into the game process, and unloads it when the app closes,
- shows live HP bars, session stats and the activity feed, plays sounds and draws the overlay,
- holds your toggles (player can't die, boss can't die, falls, and so on).

## The game module

The module is a DLL loaded inside the game. It reads and writes character HP every 2 ms.

- **Immortality.** For Elden Ring, the module sets the game's per-character "no death" flag on you and the boss, so
  HP stops at 1. The module sees the 1 HP, counts a death or defeat, and refills HP to full.
- **Falls.** The module remembers the last spot where you stood level for half a second. If you drop more than the
  fall distance below it, or land hard enough to die below it, you're put back there.
- **Boss detection.** The boss is the enemy with the highest max HP within 80 m. Between attempts it keeps re-checking,
  so it switches to the arena's boss as you walk in. During a fight it stays locked.
- **Safety.** Reads are fault-tolerant, so a bad pointer is skipped instead of crashing the game. Unloading the module
  clears every flag it set.

Game data is found by pattern-scanning the game's exe instead of using fixed addresses, so other patches may work.
Offsets can also be overridden in the module's ini file without rebuilding.

## Shared memory

The app and the module talk through a named block of shared memory. The app writes settings and commands (next
boss, new session, probe, unload); the module writes status (HP, counters, the current attempt) and a stream of
events (deaths, falls, defeats) that drive the activity feed, notifications and sounds. Toggles can be flipped from
either side, which is how the in-game hotkeys and the app's switches stay in sync.

## The catalog

The catalog is a JSON file listing every module, available or planned, with versions, tested patches and download
links. It's generated when the site is deployed, from each game's `module.json` and the project's GitHub releases,
and published as [`catalog.json`]({{CATALOG_URL}}) next to this site.

Both the app's game library and this site's [Games](/games) page read it. Installed modules are kept in
`%LOCALAPPDATA%\GitGudGym\modules\<game>\`, and each game's stats and log in
`%LOCALAPPDATA%\GitGudGym\data\<game>\`.

```json
{
  "schema": 1,
  "app": { "version": "…", "releaseUrl": "…", "download": { "name": "…", "url": "…", "size": 0, "sha256": "…" } },
  "modules": [
    {
      "id": "eldenring",
      "name": "Elden Ring",
      "status": "available",
      "version": "…",
      "game": { "exe": "eldenring.exe", "steamAppId": 1245620, "testedPatches": ["1.12.0"] },
      "files": [{ "name": "…", "url": "…", "size": 0, "sha256": "…" }]
    }
  ]
}
```

Want to add a module? Start with [Adding a game](/docs/adding-a-game).
