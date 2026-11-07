# Getting started

Git Gud Gym is a free desktop app for Windows 10 and 11. It attaches a small module to your game so that
neither you nor the boss can die, and it logs every attempt so you can watch yourself improve.

## Requirements

- Windows 10 or 11
- A supported game (see the [game library](/games)), played **offline**
- Administrator rights: the app asks for them when it starts

## 1. Download Git Gud Gym

Grab the latest release from the [releases page]({{RELEASES_URL}}) and extract the zip anywhere you like,
for example `C:\Games\GitGudGym`. There's no installer, and nothing is written outside that folder except
the files the app creates next to your game when you ask it to (see [Playing offline](/docs/offline)).

> [!NOTE]
> Releases up to **Elden Ring v0.2.0** ship as a single `GitGudGym-EldenRing-v*.zip` that contains the app
> and the Elden Ring module together. Extract it and run `GitGudGym.exe`; there is no separate module to install.

## 2. Run the app

Run `GitGudGym.exe` and accept the administrator prompt. The app needs admin rights to attach to the game and
to launch it, even when the game itself runs as administrator.

## 3. Add your game

Install the module for the game you want to practice from the app's game library. Each game is a separate
module with its own version, so you only download what you play.

The app then looks for the game on your PC by itself. If it can't find it, or finds the wrong copy, see
[Finding your game](/docs/finding-your-game).

## 4. Launch the game offline

Click **Launch game**. The app starts the game offline, without Easy Anti-Cheat. The first time, it asks before
creating the small file that makes that possible. Once the game has loaded, the module attaches automatically and
the game card turns **Active**.

> [!IMPORTANT]
> Use Git Gud Gym **offline only**. The app refuses to attach while Easy Anti-Cheat is running. Modding a game
> online can get your account flagged.

## 5. Fight

Walk into a boss arena and start fighting. The boss is detected automatically and shows up under **Live fight**.

- **Lethal hits** refill your HP and count as a death.
- **The boss** refills to full HP at zero and it counts as a defeat. It stays in its current phase; phase
  changes triggered by HP thresholds still happen.
- **Falling** off a ledge puts you back on the last solid ground you stood on, and counts as a death.
- **Wrong boss?** Press `F6` to track the next candidate.
- **Boss stuck at 1 HP?** Some bosses change phase through a scripted death. Press `F7` to let the boss
  die once.

Every attempt is saved to CSV as you go. See [Your stats](/docs/stats) for what's recorded, and
[Hotkeys](/docs/hotkeys) for everything you can do without leaving the game.

> [!TIP]
> On-screen notifications and the HUD need **Borderless** or **Windowed** display mode. Exclusive fullscreen
> draws over them.

## Known limitations

- **No phase 1 reset:** a boss defeat refills HP but doesn't reset the boss to phase 1. It's on the roadmap.
- **Instant-death effects:** Deathblight and similar effects may bypass immortality.
- **Damage credit:** damage from spirit ashes and co-op summons counts as yours.
