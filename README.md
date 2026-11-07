<div align="center">

<img src="docs/images/logo.png" width="120" alt="Git Gud Gym logo">

# Git Gud Gym

**Endless boss practice for souls-likes.** Neither you nor the boss can die. You keep fighting,
learn the moveset, and every attempt is logged so you can watch yourself improve.

Install Git Gud Gym once, then add the games you play from its library.

[![Download](https://img.shields.io/github/v/release/Git-Gud-Gym/launcher?label=download&color=d4a84b&style=for-the-badge)](https://github.com/Git-Gud-Gym/launcher/releases/latest)
[![Downloads](https://img.shields.io/github/downloads/Git-Gud-Gym/launcher/total?color=d4a84b&style=for-the-badge)](https://github.com/Git-Gud-Gym/launcher/releases)
[![Build](https://img.shields.io/github/actions/workflow/status/Git-Gud-Gym/launcher/build.yml?branch=main&style=for-the-badge)](https://github.com/Git-Gud-Gym/launcher/actions/workflows/build.yml)

[![Website](https://img.shields.io/badge/website-docs-0c0d12?style=flat-square)](https://git-gud-gym.github.io/)
![Platform](https://img.shields.io/badge/platform-Windows%2010%20%2F%2011-0c0d12?style=flat-square&logo=windows)
![C++20](https://img.shields.io/badge/C%2B%2B-20-0c0d12?style=flat-square&logo=cplusplus)
[![License: MIT](https://img.shields.io/badge/license-MIT-0c0d12?style=flat-square)](LICENSE)
[![PRs welcome](https://img.shields.io/badge/PRs-welcome-0c0d12?style=flat-square)](https://github.com/Git-Gud-Gym/.github/blob/main/CONTRIBUTING.md)

### [⬇ Download GitGudGym.exe](https://github.com/Git-Gud-Gym/launcher/releases/latest/download/GitGudGym.exe)

<sub>Windows 10 / 11 · one file, no installer · [all releases](https://github.com/Git-Gud-Gym/launcher/releases)</sub>

<img src="docs/images/app.png" width="900" alt="Git Gud Gym game page during a fight">

<sub>A game page mid-fight (sample data).</sub>

</div>

---

This is the **launcher**: the desktop app you install. Game support lives in separate modules, which
the launcher downloads from its library.

| Repository | What it is |
|---|---|
| **launcher** (here) | The app: library, downloads, game pages, live stats, overlay |
| [sdk](https://github.com/Git-Gud-Gym/sdk) | Trainer core every game module is built on |
| [module-eldenring](https://github.com/Git-Gud-Gym/module-eldenring) | Elden Ring support |
| [registry](https://github.com/Git-Gud-Gym/registry) | Which modules appear in the library (`catalog.json`) |
| [git-gud-gym.github.io](https://github.com/Git-Gud-Gym/git-gud-gym.github.io) | Website and docs |

## Games

| Game | Tested on | Status |
|---|---|---|
| Elden Ring | patch 1.12.0 | ✅ [Available](https://github.com/Git-Gud-Gym/module-eldenring) |
| Sekiro: Shadows Die Twice | | 🗓️ Planned |
| Dark Souls III | | 🗓️ Planned |

Want another game? [Request it](https://github.com/Git-Gud-Gym/registry/issues/new?template=game_request.yml),
or [build the module yourself](https://github.com/Git-Gud-Gym/sdk/blob/main/docs/ADDING_A_GAME.md).

## Features

| | |
|---|---|
| **Can't die** | Lethal hits refill your HP instead of killing you, and count as a death. |
| **Boss can't die either** | At zero HP the boss refills to full, and it counts as a defeat. Phase changes triggered by HP thresholds still happen. |
| **Falls count** | Fall off a ledge and you're put back on the last solid ground you stood on. |
| **Stats** | Damage dealt and taken, hits, healing, lowest boss HP, attempt times, fastest defeat. Saved as CSV after every attempt. |
| **In-game notifications** | "YOU DIED" / "BOSS DEFEATED" pop-ups and a mini HUD over the game, with optional sounds. |
| **Game library** | Install, update and remove game modules in one click. Every download is checksum-verified. |
| **Finds your games** | Searches your Steam libraries and common folders, or you can browse to the exe. |
| **Hotkeys** | Everything important works from inside the game with F5 to F10. |

## Quick start

1. **Download [GitGudGym.exe](https://github.com/Git-Gud-Gym/launcher/releases/latest/download/GitGudGym.exe)**,
   put it anywhere, and run it. It's a single file with no installer. Accept the administrator prompt:
   the app needs it to launch the game and attach to it.
   > Windows SmartScreen may say it "protected your PC", because the app isn't code-signed yet. Click
   > **More info → Run anyway**. Each release lists the file's SHA-256 in `SHA256SUMS.txt`, so you can
   > check the download.
2. **Install a game** from the **Library**.
3. **Click Launch game** on the game's page. The app finds your install and starts the game offline,
   without anti-cheat. Once the game has loaded, the mod attaches and the status turns **Active**.
4. **Walk into a boss arena** and fight.

<p align="center"><img src="docs/images/library.png" width="760" alt="The Git Gud Gym library"></p>

> [!IMPORTANT]
> Git Gud Gym is for **offline, single-player** practice. The app refuses to attach while a game's
> anti-cheat is running. Modding a game online can get your account flagged.

> [!TIP]
> On-screen notifications and the HUD need **Borderless** or **Windowed** display mode.

Full documentation, including playing offline, stats and troubleshooting, is at
**[git-gud-gym.github.io/docs](https://git-gud-gym.github.io/docs)**.

## Controls

| Key | In game | App |
|-----|---------|-----|
| <kbd>F5</kbd> | Immortality on / off (you and the boss) | *Player can't die* / *Boss can't die* |
| <kbd>F6</kbd> | Track the next boss candidate | *Next boss* |
| <kbd>F7</kbd> | Let the boss die once (for phase changes triggered by a scripted death) | *Let boss die once* |
| <kbd>F8</kbd> | Probe: write diagnostics to the log | *Probe* |
| <kbd>F9</kbd> | Save the session and start a new one | *New session* |
| <kbd>F10</kbd> | Unload the mod | *Unload mod* |

## Building

Requires Windows 10/11 and Visual Studio 2022 (or Build Tools) with the C++ workload.

```bat
git clone https://github.com/Git-Gud-Gym/launcher.git
cd launcher
build.bat
```

The build fetches the [SDK](https://github.com/Git-Gud-Gym/sdk) version pinned in `CMakeLists.txt`.
See [CONTRIBUTING.md](CONTRIBUTING.md) for local SDK checkouts, non-admin builds and testing modules,
and [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for how the app works.

## Roadmap

- [ ] **True phase 1 reset:** reload the encounter on a boss defeat instead of only refilling HP
- [ ] Boss names in stats instead of max HP
- [ ] Progress charts from the CSV history
- [ ] More games: Sekiro, Dark Souls III, and whatever the community asks for

## Credits

- [Dear ImGui](https://github.com/ocornut/imgui) by Omar Cornut (MIT): the app's UI
- [JSON for Modern C++](https://github.com/nlohmann/json) by Niels Lohmann (MIT)
- Notification sound: "Gaming lock" from [Mixkit](https://mixkit.co/free-sound-effects/), under the Mixkit Sound Effects Free License
- Fonts: Segoe UI and Bahnschrift, loaded from Windows

Claude (Anthropic) was used as a development assistant throughout this project, helping with implementation, debugging, and technical research.

## License

[MIT](LICENSE). Git Gud Gym is a fan-made tool and isn't affiliated with or endorsed by FromSoftware,
Bandai Namco or any other publisher. Game names are trademarks of their respective owners. Use it
offline, at your own risk.
