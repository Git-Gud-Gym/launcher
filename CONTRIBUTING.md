# Contributing to Git Gud Gym

Thanks for helping. Whether you want to report a bug, support a new game, polish the app or
improve the website, this page gets you started.

## Ways to help

- **Report a bug.** Use the [bug report form](../../issues/new?template=bug_report.yml). Press
  <kbd>F8</kbd> in game first and attach `gitgudgym.log` (*Log* button on the game page).
- **Request a game.** Use the [game request form](../../issues/new?template=game_request.yml).
  Upvote existing requests with 👍.
- **Test a game patch.** Tell us if a module works on a game version we haven't tested. The app shows
  "untested version" in that case.
- **Add a game.** See [docs/ADDING_A_GAME.md](docs/ADDING_A_GAME.md). This is the most valuable
  contribution.
- **Improve the app or the website.** Pick an issue labelled `good first issue` or `help wanted`.

## Project layout

```
src/app/              Git Gud Gym hub: library, downloads, dashboard, overlay (C++20, Dear ImGui, D3D11)
src/core/             Shared trainer: immortality, falls, boss tracking, stats (compiled into every module)
src/games/<id>/       One folder per game: adapter + module.json + default ini
src/common/           The app <-> module shared-memory contract, paths, version info
site/                 Website and docs (React + Vite + Tailwind), deployed to GitHub Pages
scripts/              Packaging and catalog generation
catalog/planned.json  Games we intend to support
```

[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) explains how the pieces fit together.

## Building

Requirements: Windows 10/11 and Visual Studio 2022 (or Build Tools) with the C++ workload.

```bat
build.bat
```

This produces `bin\GitGudGym.exe` and `bin\modules\<id>\`. The app treats `bin\modules` as a
**development install**: modules there show a "dev" badge and take priority over downloaded copies.
So rebuild, then restart the app.

The app requires administrator rights. For UI work without UAC prompts, configure with
`-DGGG_REQUIRE_ADMIN=OFF`.

Website: `cd site && npm ci && npm run dev`. See [site/README.md](site/README.md).

## Pull requests

1. Fork, then branch from `main`.
2. Keep each PR focused: one feature or fix.
3. Match the surrounding style. C++ is formatted with the repo's [.clang-format](.clang-format).
4. CI must pass, and new warnings at `/W4` count as failures.
5. For gameplay changes, say which game, which patch, and what you tested in game.
6. Add a line to the right section of [CHANGELOG.md](CHANGELOG.md).

## Releases

Maintainers release the hub and each game module separately:

| Tag | Releases | Version lives in |
|---|---|---|
| `app-vX.Y.Z` | `GitGudGym.exe` | `project(... VERSION)` in `CMakeLists.txt` |
| `<id>-vX.Y.Z` | one game module | `src/games/<id>/module.json` |

Pushing the tag builds, packages and publishes the release, then refreshes the website's game
library (`catalog.json`), which the app reads.

## Ground rules

- **Offline and single-player only.** We never help bypass anti-cheat for online play. Modules must
  list the game's anti-cheat processes so the app refuses to attach while they run.
- **No publisher assets.** No game art, logos or files from the games in this repo or on the website.
- **Be kind.** See the [Code of Conduct](CODE_OF_CONDUCT.md).
