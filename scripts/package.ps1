# Packages bin\ into dist\GitGudGym-v<version>.zip. Run build.bat first.
#   ./scripts/package.ps1 [-Version x.y.z]   (CI passes the tag's version; it must match CMakeLists.txt)
param([string]$Version)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$appVersion = (Select-String -Path "$root\CMakeLists.txt" -Pattern 'project\(GitGudGym VERSION ([0-9.]+)').Matches[0].Groups[1].Value
if ($Version -and $Version -ne $appVersion) {
    throw "Tag version $Version doesn't match CMakeLists.txt version $appVersion. Bump it before tagging."
}

$dist = Join-Path $root 'dist'
Remove-Item -Recurse -Force $dist -ErrorAction SilentlyContinue
$stage = Join-Path $dist 'GitGudGym'
New-Item -ItemType Directory -Force "$stage\assets" | Out-Null
Copy-Item "$root\bin\GitGudGym.exe" $stage
Copy-Item "$root\bin\assets\*" "$stage\assets"
Copy-Item "$root\LICENSE" "$stage\LICENSE.txt"
@"
Git Gud Gym v$appVersion
https://git-gud-gym.github.io/

1. Run GitGudGym.exe (it asks for administrator rights).
2. In the Library, install the module for your game.
3. Click Launch game. Play offline, without anti-cheat.

Stats and settings live in %LOCALAPPDATA%\GitGudGym.
"@ | Set-Content -Encoding UTF8 "$stage\READ ME.txt"
Compress-Archive -Path $stage -DestinationPath "$dist\GitGudGym-v$appVersion.zip"
Remove-Item -Recurse -Force $stage
Write-Host "Packaged Git Gud Gym $appVersion"
