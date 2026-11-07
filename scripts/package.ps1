# Packages a build from bin\ into dist\<target>\. Run build.bat first.
#
#   ./scripts/package.ps1 -Target app            # GitGudGym-v<version>.zip
#   ./scripts/package.ps1 -Target eldenring      # the module's files + a zip for manual installs
#
# -Version (optional) must match the source version; CI passes the tag's version so a mistagged
# release fails instead of shipping the wrong build.
param(
    [Parameter(Mandatory)][string]$Target,
    [string]$Version
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$dist = Join-Path $root "dist\$Target"
Remove-Item -Recurse -Force $dist -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $dist | Out-Null

function Assert-Version([string]$expected) {
    if ($Version -and $Version -ne $expected) {
        throw "Tag version $Version doesn't match the source version $expected. Bump it before tagging."
    }
}

if ($Target -eq 'app') {
    $appVersion = (Select-String -Path "$root\CMakeLists.txt" -Pattern 'project\(GitGudGym VERSION ([0-9.]+)').Matches[0].Groups[1].Value
    Assert-Version $appVersion
    $stage = Join-Path $dist 'GitGudGym'
    New-Item -ItemType Directory -Force "$stage\assets" | Out-Null
    Copy-Item "$root\bin\GitGudGym.exe" $stage
    Copy-Item "$root\bin\assets\*" "$stage\assets"
    Copy-Item "$root\LICENSE" "$stage\LICENSE.txt"
    @"
Git Gud Gym v$appVersion
https://github.com/ryyanrashid01/git-gud-gym

1. Run GitGudGym.exe (it asks for administrator rights).
2. In the Library, install the module for your game.
3. Click Launch game. Play offline, without anti-cheat.

Stats and settings live in %LOCALAPPDATA%\GitGudGym.
"@ | Set-Content -Encoding UTF8 "$stage\READ ME.txt"
    Compress-Archive -Path $stage -DestinationPath "$dist\GitGudGym-v$appVersion.zip"
    Remove-Item -Recurse -Force $stage
    Write-Host "Packaged app $appVersion"
    return
}

$moduleDir = Join-Path $root "bin\modules\$Target"
$manifest = Get-Content "$moduleDir\module.json" -Raw | ConvertFrom-Json
Assert-Version $manifest.version
# The app downloads these individually (checksummed by the catalog)...
foreach ($file in $manifest.files) { Copy-Item (Join-Path $moduleDir $file) $dist }
# ...and the zip is for people who install by hand into %LOCALAPPDATA%\GitGudGym\modules\<id>.
$name = ($manifest.name -replace '[^A-Za-z0-9]', '')
$stage = Join-Path $env:TEMP "ggg-package-$Target"
Remove-Item -Recurse -Force $stage -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force "$stage\$Target" | Out-Null
foreach ($file in $manifest.files) { Copy-Item (Join-Path $moduleDir $file) "$stage\$Target" }
Compress-Archive -Path "$stage\$Target" -DestinationPath "$dist\GitGudGym-$name-module-v$($manifest.version).zip"
Remove-Item -Recurse -Force $stage
Write-Host "Packaged $($manifest.name) module $($manifest.version)"
