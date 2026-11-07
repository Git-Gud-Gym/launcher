# Packages bin\ into dist\GitGudGym-EldenRing-v<version>.zip. Run build.bat first.
param(
    # Defaults to the version in CMakeLists.txt; CI passes the tag's version and this checks they match.
    [string]$Version
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent

$cmakeVersion = (Select-String -Path "$root\CMakeLists.txt" -Pattern 'project\(GitGudGym VERSION ([0-9.]+)').Matches[0].Groups[1].Value
if (-not $Version) { $Version = $cmakeVersion }
if ($Version -ne $cmakeVersion) {
    throw "Tag version $Version doesn't match CMakeLists.txt version $cmakeVersion. Bump the version before tagging."
}

$name = "GitGudGym-EldenRing-v$Version"
$stage = Join-Path $root "dist\$name"
Remove-Item -Recurse -Force $stage -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force "$stage\assets" | Out-Null

Copy-Item "$root\bin\GitGudGym.exe", "$root\bin\gitgudgym.dll" $stage
# The pristine default config, never a developer's edited copy from bin\.
Copy-Item "$root\config\gitgudgym.ini" $stage
Copy-Item "$root\assets\logo_mark.png" "$stage\assets\logo.png"
Copy-Item "$root\assets\mixkit-gaming-lock-2848.wav" "$stage\assets\notify.wav"
Copy-Item "$root\LICENSE" "$stage\LICENSE.txt"
@"
Git Gud Gym v$Version for Elden Ring
https://github.com/ryyanrashid01/git-gud-gym

1. Start Elden Ring OFFLINE, without Easy Anti-Cheat (see the README on GitHub).
2. Run GitGudGym.exe. It attaches to the game automatically.
3. Walk into a boss arena and start practicing.

Stats are saved to the stats folder next to GitGudGym.exe.
Problems? Press F8 in game, then open an issue on GitHub with gitgudgym.log attached.
"@ | Set-Content -Encoding UTF8 "$stage\READ ME.txt"

$zip = Join-Path $root "dist\$name.zip"
Remove-Item -Force $zip -ErrorAction SilentlyContinue
Compress-Archive -Path $stage -DestinationPath $zip
Write-Host "Packaged $zip"
