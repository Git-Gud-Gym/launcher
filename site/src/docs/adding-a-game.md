# Adding a game

Git Gud Gym is built so that new games can be added without touching the app. Each game is a **module**,
released and versioned on its own. If you know your way around a souls-like's memory, this is the most valuable
thing you can contribute.

> [!NOTE]
> This page is a high-level overview. The code structure is being refactored, so the
> [contributing guide]({{CONTRIBUTING_URL}}) in the repository is the source of truth for file layout, build steps
> and conventions.

## What a module is

A module has two parts:

- **A DLL that implements a game adapter.** It's loaded into the game's process and does the game-specific work:
  finding characters and their HP, keeping you and the boss alive, tracking where you stand, and listing nearby
  enemies. Everything else (counting deaths and defeats, stats, the UI, notifications) is shared.
- **A `module.json` manifest.** It describes the module and the game it's for: the game's exe and Steam app ID,
  which anti-cheat processes to watch for, and which game versions it has been tested on.
  The [catalog](/docs/architecture#the-catalog) that feeds the game library is built from these manifests and the
  module's releases.

## What an adapter needs to handle

Roughly, for each game:

- **Find the data.** Locate the player, nearby enemies and their HP. Pattern-scanning the exe, rather than
  hard-coding addresses, keeps a module working across patches.
- **Keep characters alive.** Stop HP at a floor instead of letting a death happen, so the shared code can count it
  and refill HP. For Elden Ring, that's the game's per-character "no death" flag.
- **Know where the ground is.** Read the player's position so falls can be caught and the player put back on solid
  ground.
- **Find the boss.** List nearby enemies with their max HP so the boss can be picked automatically.
- **Clean up.** Clear everything it changed when the module unloads.
- **Fail safely.** Reads must tolerate bad pointers, so a missing structure is skipped instead of crashing the game.

## How to get started

1. **Say hello first.** Check the [game library](/games) and the [issues]({{ISSUES_URL}}) to see whether someone is
   already working on your game, then open an issue or a [discussion]({{DISCUSSIONS_URL}}) so effort isn't duplicated.
2. **Read the contributing guide.** [CONTRIBUTING.md]({{CONTRIBUTING_URL}}) covers building from source, the adapter
   interface and how to test.
3. **Study the Elden Ring module.** It's the reference implementation.
4. **Open a pull request.** Include the game patches you tested on. Once it's merged and released, the module shows up
   in the catalog.

## Other ways to help

- **Test a new patch.** Run the existing module on a patch that hasn't been tested, and report what works with a probe
  log (`F8`). See [Troubleshooting](/docs/troubleshooting).
- **Report bugs** and suggest features in the [issue tracker]({{ISSUES_URL}}).
- **Improve these docs.** The site's source lives in the `site/` folder of the [repository]({{GITHUB_URL}}).
