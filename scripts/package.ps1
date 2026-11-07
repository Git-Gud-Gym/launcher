# Packages the release into dist\: GitGudGym.exe (a single file; the logo and sound are embedded)
# and SHA256SUMS.txt. Run build.bat first.
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
New-Item -ItemType Directory -Force $dist | Out-Null
# Always named GitGudGym.exe so https://github.com/Git-Gud-Gym/launcher/releases/latest/download/GitGudGym.exe
# keeps pointing at the newest release.
Copy-Item "$root\bin\GitGudGym.exe" $dist
$hash = (Get-FileHash "$dist\GitGudGym.exe" -Algorithm SHA256).Hash.ToLower()
"$hash  GitGudGym.exe" | Set-Content -Encoding ASCII "$dist\SHA256SUMS.txt"
Write-Host "Packaged Git Gud Gym $appVersion ($hash)"
