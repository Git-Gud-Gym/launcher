# Troubleshooting

## Something's not working? Probe it.

Most problems come down to the game data not being where the module expects it, usually on a game patch that
hasn't been tested yet. The fastest way to get help:

1. In game, press `F8` (or click **Probe** in the app). This writes addresses, your position and
   nearby enemies to `gitgudgym.log`.
2. Click **Open log** in the app to find the file. It's kept with the game's stats, in
   `%LOCALAPPDATA%\GitGudGym\data\<game>\` (next to `GitGudGym.exe` in the all-in-one Elden Ring releases).
3. [Open an issue]({{NEW_ISSUE_URL}}) with the log attached, your game patch, and what you expected to happen.

## Common problems

### The app says my game version is untested

The module finds game data by pattern-scanning the game's exe rather than using fixed addresses, so other
patches may still work. Try it. If something's off, probe and open an issue as described above. Memory offsets
can also be changed in the module's ini file without rebuilding.

### The app won't attach

- **Easy Anti-Cheat is running.** The app refuses to attach while it is. Close the game and start it offline
  with **Launch game** (see [Playing offline](/docs/offline)).
- **The game is still loading.** The module attaches once the game has loaded; the game card turns **Active**.
- **No admin rights.** See below.

### The administrator prompt

The app asks for administrator rights when it starts so it can launch the game and attach to it, even when the
game runs as administrator. If you decline, it can't attach. Close it and run `GitGudGym.exe` again.

### I can't see the notifications or the HUD

Switch the game to **Borderless** or **Windowed** display mode. Exclusive fullscreen draws over the overlay.

### The app can't find my game

Click **Browse…** next to **Game location** and pick the game's exe. See [Finding your game](/docs/finding-your-game).

### The boss is stuck at 1 HP

Some bosses change phase through a scripted death (Rennala, Radagon). Press `F7` to let the boss die once.

### It's tracking the wrong enemy

Press `F6` to switch to the next boss candidate.

### I died anyway

Instant-death effects such as Deathblight may bypass immortality. That's a known limitation.

## Still stuck?

Search the [existing issues]({{ISSUES_URL}}) or ask in [Discussions]({{DISCUSSIONS_URL}}).
