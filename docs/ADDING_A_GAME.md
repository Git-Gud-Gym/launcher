# Adding a game

A game module is one folder under `src/games/<id>/`. The shared core does everything else: the
trainer loop, stats, falls, boss tracking, the link to the app, hotkeys and the overlay. Your job
is to teach it how to read and write the game's memory.

Use `src/games/eldenring/` as the reference implementation.

## 1. Research the game

You'll need, for the game version you're targeting:

| What | Why |
|---|---|
| A pattern to find the **player** character | Player HP and position |
| A way to list **loaded characters** | Finding the boss |
| **HP / max HP** offsets on a character | Deaths, defeats, damage stats |
| **Position** (x, y, z with y up) | Boss distance, fall detection, respawning |
| A **"can't die"** mechanism (a per-character flag, or a debug flag) | Keeping HP at 1 instead of 0 |

Cheat Engine tables and the game's modding community are the usual starting points. Prefer
**byte-pattern scans** over hardcoded addresses so the module survives small patches. Put tunable
offsets in the module's `.ini`, so users can fix a broken offset without a rebuild.

## 2. Create the module

```
src/games/<id>/
  module.json            identity + what the app needs to know about the game
  gitgudgym-<id>.ini     default offsets and tuning ([general], [boss], [offsets])
  <id>_game.h/.cpp       class <Id>Game : public GameAdapter
  dllmain.cpp            creates the adapter and calls RunTrainer(self, game, L"<id>")
  module.rc              version resource (copy from eldenring and change the two #defines)
```

`<id>` is lowercase letters and digits only, e.g. `sekiro` or `darksouls3`.

### module.json

```json
{
  "schema": 1,
  "id": "sekiro",
  "name": "Sekiro: Shadows Die Twice",
  "version": "0.1.0",
  "tagline": "One line shown on the library card.",
  "dll": "gitgudgym-sekiro.dll",
  "files": ["gitgudgym-sekiro.dll", "gitgudgym-sekiro.ini", "module.json"],
  "features": ["Shown on the website's games page"],
  "game": {
    "exe": "sekiro.exe",
    "steamAppId": 814380,
    "steamFolder": "Sekiro",
    "exeSubdir": "",
    "antiCheatProcesses": [],
    "versions": [{ "exe": "1.6.0.0", "patch": "1.06", "tested": true }]
  }
}
```

- `steamFolder` / `exeSubdir`: where the exe sits inside `steamapps\common\`. Used for auto-detect.
- `antiCheatProcesses`: the app refuses to attach while any of these run. Required for every game
  that has online anti-cheat.
- `versions`: exe file versions you've verified. Others show as "untested" in the app.

### The adapter

Implement every pure virtual in [`src/core/game_adapter.h`](../src/core/game_adapter.h):
- **Reading memory:** use `mem::Read` / `mem::ReadPtr` / `mem::SafeCopy` from `src/core/memory.h`.
  They never crash the game on a bad pointer.
- **Rejecting garbage:** `ReadChr` must reject values that don't look like a real character (HP out
  of range, null modules). Callers write to whatever you return.
- **Caching pointers:** store adapter-private pointers in `ChrState::internal[]` so writes don't
  walk the pointer chain again.
- **The F8 probe:** `Describe()` adds lines to its output. Include whatever helps someone debug
  offsets.

### Register it

Add one line to `CMakeLists.txt`:

```cmake
add_game_module(sekiro)
```

## 3. Test it

1. Run `build.bat`. The module lands in `bin\modules\<id>\` and shows up in the app with a "dev"
   badge.
2. Start the game **offline**, open its page in the app, and let it attach.
3. Press <kbd>F8</kbd> and check `gitgudgym.log`: the player HP should match the game, the position
   should change as you move, and characters should be listed.
4. Die to an enemy, take a boss to zero, and fall off a ledge. Check the counters and the CSV.

## 4. Ship it

1. Remove the game from `catalog/planned.json` if it's listed there.
2. Add a `## <Name> v0.1.0` section to `CHANGELOG.md`.
3. Open a PR describing the game version you tested and what works.

After merge, a maintainer tags `<id>-v0.1.0`. The release workflow publishes it and the game
appears in everyone's library.
