# Changelog

Releases of the Git Gud Gym launcher, tagged `vX.Y.Z`. Each game module has its own changelog in its
`module-<game>` repository.

## v0.3.2 - 2026-11-07

- **One-file download:** releases now ship `GitGudGym.exe` directly. The logo and notification sound
  are built into the exe, so there's no zip to extract and no `assets` folder to keep next to it.
  `SHA256SUMS.txt` lists the exe's checksum.
- **Stable download link:** <https://github.com/Git-Gud-Gym/launcher/releases/latest/download/GitGudGym.exe>
  always points at the newest version.

## v0.3.1 - 2026-11-07

- **Game page scrolls instead of clipping:** at high Windows display scaling (e.g. 150%) the bottom cards were
  cut off. The page now scrolls as a whole, and the activity feed has a fixed height with its own scrollbar.

## v0.3.0 - 2026-11-07

First release of Git Gud Gym as a hub: install it once, then add the games you play.

- **Library:** browse supported and planned games, and install, update or remove each game's module
  in one click. Every download is checked against a SHA-256 published in the catalog before anything
  is installed.
- **Game pages:** each installed game has its own page with launch, attach, live stats, toggles and
  game location.
- **Modules in their own repos:** the game support moved to `module-<game>` repositories built on the
  [SDK](https://github.com/Git-Gud-Gym/sdk); the library comes from the
  [registry](https://github.com/Git-Gud-Gym/registry).
- **Game-agnostic attach:** the exe name, anti-cheat processes, Steam location and tested versions
  come from each module's `module.json`.
- **New data location:** settings, modules and stats live in `%LOCALAPPDATA%\GitGudGym`.
- **`GGG_DEV_MODULES`:** point the launcher at a module's build folder to test it as a dev build.
- **Update notice:** the app tells you when a new version of itself is out.

## Earlier releases

Before v0.3.0 this repository shipped Elden Ring as an all-in-one download, tagged `eldenring-vX.Y.Z`.

### Elden Ring v0.2.0 - 2026-11-07

- **Finds the game by itself:** searches every Steam library, Steam's install record, and common folders
  on every drive.
- **Game location in the sidebar:** shows where the game is, with **Browse…** and **Auto-detect** buttons.
- **Launch game starts the game offline:** on Steam copies it offers to create `steam_appid.txt`, so the
  game starts without Easy Anti-Cheat.
- **Admin on start:** the app asks for administrator rights when it starts, so it can launch and attach
  to the game even when the game runs as admin.
- The left column no longer scrolls.

### Elden Ring v0.1.0 - 2026-11-07

First release. Tested on Elden Ring **1.12.0** (`eldenring.exe` 2.2.0.0).

### Training
- **Player can't die:** lethal hits refill your HP and count as a death.
- **Boss can't die:** the boss refills to full HP at zero and it counts as a defeat. Phase changes
  triggered by HP thresholds still happen.
- **Falling counts as death:** fall too far below the last solid ground you stood on and you're put
  back there. The distance is adjustable, 8 to 60 m.
- **Let boss die once:** for bosses whose phase change is a scripted death (Rennala, Radagon).
- **Automatic boss detection:** picks the biggest enemy within 80 m and switches between attempts.
  Next boss overrides the choice.

### Stats
- Per attempt: outcome, duration, damage dealt and taken, hits landed and taken, healing, and lowest
  boss HP %. Appended to `stats/attempts.csv`.
- Per session: totals, fastest defeat, average attempt, and best boss HP %. Appended to `stats/sessions.csv`.

### App
- Desktop app with live HP bars, session stats, an activity feed, and toggle switches. Settings are saved.
- Attaches automatically when the game is running, and unloads the mod when the app closes.
- Refuses to attach while Easy Anti-Cheat is running, and warns on untested game versions.
- In-game notifications when you die or beat the boss, plus a mini HUD (borderless or windowed mode).
- Notification sounds for deaths and boss defeats.
- In-game hotkeys F5 to F10.

### Known limitations
- A boss defeat refills HP but doesn't reset the boss to phase 1.
- Fall respawn, the notifications and the HUD are new and have had little in-game testing. Please
  report issues with your `gitgudgym.log`.
