# Architecture

```
  module-<game> releases ──► registry (CI) ──► catalog.json (git-gud-gym.github.io/registry)
                                                       │
                                                       ▼
┌──────────────────────────── GitGudGym.exe (launcher) ────────────────────────┐
│  Library: install / update / remove modules (SHA-256 verified)               │
│  Game page: launch the game, attach the module, live stats, toggles          │
│  Overlay: toasts + mini HUD over the game window (GDI+ layered windows)      │
└───────────────┬──────────────────────────────────────────────▲───────────────┘
                │ LoadLibrary (injected)                       │
                ▼                                              │ shared memory
┌────────── game process ─────────────────────────────┐        │ "Local\GitGudGym.State.v1"
│  gitgudgym-<game>.dll  (built from module-<game>)   │        │
│   ├─ SDK core: trainer loop, stats, link to app ────┼────────┘
│   └─ the game's GameAdapter                         │
└──────────────────────────────────────────────────────┘
```

## The repositories

| Repo | Role |
|---|---|
| [launcher](https://github.com/Git-Gud-Gym/launcher) | This app. Builds against the SDK's contract headers only. |
| [sdk](https://github.com/Git-Gud-Gym/sdk) | Trainer core, `GameAdapter`, the shared-memory contract, `ggg_add_module()` |
| `module-<game>` | One per game: an adapter + `module.json`, released on its own |
| [registry](https://github.com/Git-Gud-Gym/registry) | Reviewed list of module repos, turned into `catalog.json` by CI |
| [git-gud-gym.github.io](https://github.com/Git-Gud-Gym/git-gud-gym.github.io) | Website and docs; reads the same catalog |

## Inside the launcher

- **Library** (`modules.cpp`, `catalog.cpp`, `http.cpp`): reads the catalog over HTTPS (WinHTTP)
  and caches it in `%LOCALAPPDATA%\GitGudGym\catalog.json`. It downloads each module file, checks
  it against the catalog's SHA-256, and swaps the module folder in atomically.
- **Attaching** (`app.cpp`, `injector.cpp`): watches for the active game's exe (from its
  `module.json`) and waits for the game to finish starting. It refuses while any of the module's
  anti-cheat processes run, then loads the module DLL.
- **Finding games** (`locator.cpp`): searches Steam libraries (registry + `libraryfolders.vdf`),
  Steam's uninstall entry, and common folders. It also offers a file picker.
- **UI** (`ui.cpp`): Dear ImGui on D3D11 in a borderless window.

## The contract (from the SDK)

A fixed-layout struct in a named file mapping:

| Part | Written by | What |
|---|---|---|
| `settings` | both | Gameplay toggles. Hotkeys in game and switches in the app write the same fields. |
| `cmd` / `cmdSeq` / `cmdAck` | launcher | Commands: next boss, new session, probe, unload. |
| `status` | module | Snapshot of HP, boss, attempt and session totals, ~30×/s. |
| `events[]` | module | Ring buffer of deaths, defeats and warnings, used for toasts, sounds and the feed. |

Changing the layout is a major SDK release with a new mapping name (`.v1` → `.v2`), so an old
launcher and a new module never read each other's memory.

## Files on disk

```
%LOCALAPPDATA%\GitGudGym\
  settings.ini            app prefs, gameplay toggles, per-game install paths
  catalog.json            last fetched game library
  modules\<game>\         installed modules (dll, module.json, ini)
  data\<game>\stats\      attempts.csv, sessions.csv
  data\<game>\gitgudgym.log module log (F8 probe output goes here)
```
