# Architecture

```
          GitHub releases ──► catalog.json (GitHub Pages)
                                    │
                                    ▼
┌──────────────────────────── GitGudGym.exe (hub) ─────────────────────────────┐
│  Library: install / update / remove modules (SHA-256 verified)               │
│  Game page: launch the game, attach the module, live stats, toggles          │
│  Overlay: toasts + mini HUD over the game window (GDI+ layered windows)      │
└───────────────┬──────────────────────────────────────────────▲───────────────┘
                │ LoadLibrary (injected)                       │
                ▼                                              │ shared memory
┌────────── game process ─────────────────────────────┐        │ "Local\GitGudGym.State.v1"
│  gitgudgym-<id>.dll                                  │        │
│   ├─ src/core   trainer loop, stats, link to app ────┼────────┘
│   └─ src/games/<id>  GameAdapter for this game       │
└──────────────────────────────────────────────────────┘
```

## Pieces

### Hub app: `src/app/`
- **Library** (`modules.cpp`, `catalog.cpp`, `http.cpp`): reads the catalog over HTTPS (WinHTTP)
  and caches it in `%LOCALAPPDATA%\GitGudGym\catalog.json`. It downloads each module file, checks
  it against the catalog's SHA-256, and swaps the module folder in atomically.
- **Attaching** (`app.cpp`, `injector.cpp`): watches for the active game's exe and waits for it to
  finish starting. It refuses while any of the module's anti-cheat processes run, then loads the
  module DLL.
- **Finding games** (`locator.cpp`): searches Steam libraries (registry + `libraryfolders.vdf`),
  Steam's uninstall entry, and common folders. It also offers a file picker.
- **UI** (`ui.cpp`): Dear ImGui on D3D11 in a borderless window.

### Shared core: `src/core/`
Compiled into every module. `trainer.cpp` runs on its own thread inside the game:
- polls HP every 2 ms through the `GameAdapter`;
- keeps the player and the tracked boss from dying (HP stops at 1), counts deaths and defeats, and
  refills HP;
- tracks "solid ground" and respawns the player after a fatal fall;
- records attempts (`stats.cpp`) to CSV in `%LOCALAPPDATA%\GitGudGym\data\<id>\stats`;
- publishes status and events to the app through shared memory, and handles hotkeys and app
  commands.

### Game modules: `src/games/<id>/`
A module is a `GameAdapter` implementation (finding the player, the loaded characters, reading and
writing HP and position, the "no death" flag) plus a `module.json`. See
[ADDING_A_GAME.md](ADDING_A_GAME.md).

### The contract: `src/common/shared_state.h`
A fixed-layout struct in a named file mapping:

| Part | Written by | What |
|---|---|---|
| `settings` | both | Gameplay toggles. Hotkeys in game and switches in the app write the same fields. |
| `cmd` / `cmdSeq` / `cmdAck` | app | Commands: next boss, new session, probe, unload. |
| `status` | module | Snapshot of HP, boss, attempt and session totals, ~30×/s. |
| `events[]` | module | Ring buffer of deaths, defeats and warnings, used for toasts, sounds and the feed. |

Changing the layout means bumping the mapping name (`.v1` → `.v2`) so old and new builds never
read each other's memory.

## Files on disk

```
%LOCALAPPDATA%\GitGudGym\
  settings.ini            app prefs, gameplay toggles, per-game install paths
  catalog.json            last fetched game library
  modules\<id>\           installed modules (dll, module.json, ini)
  data\<id>\stats\        attempts.csv, sessions.csv
  data\<id>\gitgudgym.log module log (F8 probe output goes here)
```

## Releases and the catalog
`release.yml` builds and publishes a tag (`app-v*` or `<id>-v*`), then triggers `pages.yml`.
`pages.yml` runs `scripts/build-catalog.mjs`, which matches each `src/games/*/module.json` with its
latest release, records file sizes and SHA-256s, adds `catalog/planned.json`, and deploys the
website with the fresh `catalog.json`.
