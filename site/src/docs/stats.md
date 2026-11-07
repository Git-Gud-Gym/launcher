# Your stats

Every attempt is saved as CSV, one folder per game. Open the files in Excel, Google Sheets, or anything else that
reads CSV. The app's **Open stats folder** button takes you straight there.

Stats live in `%LOCALAPPDATA%\GitGudGym\data\<game>\stats\`.

> [!NOTE]
> In the all-in-one Elden Ring releases (v0.1.0 and v0.2.0), the `stats\` folder sits next to `GitGudGym.exe` and
> settings live in `gitgudgym.ini` in the same folder.

An attempt starts on the first hit and ends with your death, the boss's defeat, or you leaving.

## attempts.csv

One row per attempt.

| Column | Meaning |
|---|---|
| `outcome` | `death`, `fall`, `defeat`, or `abandoned` |
| `duration_s` | Fight length in seconds |
| `damage_dealt` / `damage_taken` | HP removed from the boss / from you |
| `hits_landed` / `hits_taken` | Number of hits |
| `healing` | HP you healed during the attempt |
| `boss_lowest_hp_pct` | How close you got: 0 means a defeat |
| `session_id`, `label` | Groups attempts. Set `session_label` in the `[general]` section of the module's ini file to tag them, e.g. `Malenia, bleed build` |

## sessions.csv

One row per session: totals, fastest defeat, average attempt length, and the best boss HP % you reached.

Press `F9` (or **New session** in the app) to save the current session and start a new one, for example
when you move on to a different boss or switch builds.

## Tips

- **Label your sessions.** A `session_label` like `Malenia, bleed build` makes it easy to filter and compare later.
- **Track `boss_lowest_hp_pct` over time.** Even before your first defeat, it shows how much closer each attempt gets.
- **Damage credit:** damage from spirit ashes and co-op summons counts as yours.
