# Contributing to the launcher

Start with the organization-wide
**[contributing guide](https://github.com/Git-Gud-Gym/.github/blob/main/CONTRIBUTING.md)**: where
to report what, PR rules and ground rules. This page covers working on the launcher itself.

> Want to add a game? That happens in its own module repo. See the SDK's
> [Adding a game](https://github.com/Git-Gud-Gym/sdk/blob/main/docs/ADDING_A_GAME.md) guide.

## Layout

```
src/app/            the launcher (C++20, Dear ImGui, Direct3D 11)
  app.*             state: library, installs, attaching, events, settings
  ui.*              every screen and widget
  catalog.* http.*  the registry's catalog.json, downloads, SHA-256
  modules.*         installed modules and their module.json
  locator.*         finding game installs (Steam libraries, common folders, file picker)
  injector.*        loading a module into the game process
  overlay.*         toasts and mini HUD over the game (GDI+ layered windows)
third_party/        Dear ImGui, nlohmann/json (vendored)
docs/               architecture notes and README images
```

The shared-memory contract the launcher and modules talk through comes from the
[SDK](https://github.com/Git-Gud-Gym/sdk) (`include/shared_state.h`).

## Building

```bat
build.bat                                                  :: fetches the pinned SDK release
build.bat -DFETCHCONTENT_SOURCE_DIR_GGG_SDK=..\sdk         :: or a local SDK checkout
build.bat -DGGG_REQUIRE_ADMIN=OFF                          :: no UAC prompt, for UI work
```

## Testing with a module you're building

Set `GGG_DEV_MODULES` to the module's `bin` folder (several folders can be separated with `;`)
before starting the launcher. The module shows up with a **dev** badge and takes priority over a
downloaded copy:

```bat
set GGG_DEV_MODULES=C:\src\module-eldenring\bin
bin\GitGudGym.exe
```

`GitGudGym.exe --install <id>` installs a module from the catalog on startup, which is handy for
testing the download path.

## Releasing

1. Bump `project(GitGudGym VERSION x.y.z)` in `CMakeLists.txt`.
2. Add a `## vx.y.z` section to `CHANGELOG.md`.
3. Push the tag `vx.y.z`.

The release workflow builds, packages and publishes the release. The registry's catalog picks up
the new version, and running copies of the app show an update notice.
