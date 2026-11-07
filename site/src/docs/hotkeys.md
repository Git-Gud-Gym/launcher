# Hotkeys

Everything important works from inside the game. Each hotkey also has a matching control in the app.

| Key | In game | In the app |
|-----|---------|------------|
| `F5` | Immortality on / off (you and the boss) | *Player can't die* / *Boss can't die* |
| `F6` | Track the next boss candidate | *Next boss* |
| `F7` | Let the boss die once (for phase changes triggered by a scripted death) | *Let boss die once* |
| `F8` | Probe: write addresses, position and nearby enemies to `gitgudgym.log` | *Probe* |
| `F9` | Save the session and start a new one | *New session* |
| `F10` | Unload the mod | *Unload mod* |

## Bosses that die to change phase

Some bosses move to their next phase through a scripted death rather than an HP threshold. In Elden Ring,
that's Rennala and Radagon into the Elden Beast. With *Boss can't die* on, they stay stuck at 1 HP.

Press `F7` before the final hit. The boss is allowed to die once, so the next phase can
start.

## Picking the right boss

The boss is the enemy with the highest max HP within 80 m. Between attempts the app keeps re-checking, so it
switches to the arena's boss as you walk in; during a fight it stays locked. If it picks the wrong enemy, press
`F6` to move to the next candidate.
