param(
    [string]$ThirdPartyDir = "third_party",
    [switch]$Force
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$raylibRef = if ($env:RAYLIB_REF) { $env:RAYLIB_REF } else { "5.5" }
$rayguiRef = if ($env:RAYGUI_REF) { $env:RAYGUI_REF } else { "1c2365a" }
$jsonRef = if ($env:NLOHMANN_JSON_REF) { $env:NLOHMANN_JSON_REF } else { "553c314fb" }
$nbnetRef = if ($env:NBNET_REF) { $env:NBNET_REF } else { "2.0" }
$thirdPartyArchiveUrl = $env:THIRD_PARTY_ARCHIVE_URL
$ultralightSdkUrl = $env:ULTRALIGHT_SDK_URL
if ([string]::IsNullOrWhiteSpace($thirdPartyArchiveUrl) -and [string]::IsNullOrWhiteSpace($ultralightSdkUrl)) {
    throw "Neither THIRD_PARTY_ARCHIVE_URL nor ULTRALIGHT_SDK_URL is set."
}

$deps = @(
    @{ Name = "raylib-master"; Url = "https://github.com/raysan5/raylib.git"; Ref = $raylibRef },
    @{ Name = "raygui-master"; Url = "https://github.com/raysan5/raygui.git"; Ref = $rayguiRef },
    @{ Name = "json-develop"; Url = "https://github.com/nlohmann/json.git"; Ref = $jsonRef },
    @{ Name = "nbnet"; Url = "https://github.com/nathhB/nbnet.git"; Ref = $nbnetRef }
)

function Sync-Repo {
    param(
        [hashtable]$Dep
    )

    $path = Join-Path $ThirdPartyDir $Dep.Name

    if (Test-Path $path) {
        if ($Force) {
            Write-Host "Removing existing $($Dep.Name) ..."
            Remove-Item -Recurse -Force $path
        } else {
            Write-Host "Skipping existing $($Dep.Name) (use -Force to refresh)."
            return
        }
    }

    Write-Host "Cloning $($Dep.Name) from $($Dep.Url) ..."
    git clone $Dep.Url $path
    if ($LASTEXITCODE -ne 0) { throw "Failed to clone $($Dep.Name)." }

    git -C $path checkout $Dep.Ref
    if ($LASTEXITCODE -ne 0) { throw "Failed to checkout ref '$($Dep.Ref)' for $($Dep.Name)." }

    git -C $path submodule update --init --recursive
    if ($LASTEXITCODE -ne 0) { throw "Failed to update submodules for $($Dep.Name)." }

    $sha = git -C $path rev-parse --short HEAD
    if ($LASTEXITCODE -ne 0) { throw "Failed to query HEAD for $($Dep.Name)." }
    Write-Host "$($Dep.Name) checked out at $sha"
}

function Get-SevenZipExe {
    $candidates = @(
        (Join-Path $env:ProgramFiles "7-Zip\7z.exe"),
        (Join-Path ${env:ProgramFiles(x86)} "7-Zip\7z.exe"),
        "7z"
    )

    foreach ($candidate in $candidates) {
        if ($candidate -eq "7z") {
            $cmd = Get-Command 7z -ErrorAction SilentlyContinue
            if ($cmd) { return $cmd.Source }
        } elseif (Test-Path $candidate) {
            return $candidate
        }
    }

    throw "7-Zip executable not found. Please install 7-Zip on the runner."
}

function Install-UltralightSdk {
    $sdkDir = Join-Path $ThirdPartyDir "ultralight"
    $requiredSdkFiles = @(
        (Join-Path $sdkDir "include\Ultralight\Ultralight.h"),
        (Join-Path $sdkDir "lib\Ultralight.lib"),
        (Join-Path $sdkDir "bin\Ultralight.dll"),
        (Join-Path $sdkDir "resources\icudt67l.dat")
    )

    $hasSdk = $true
    foreach ($file in $requiredSdkFiles) {
        if (-not (Test-Path $file)) {
            $hasSdk = $false
            break
        }
    }

    if ($hasSdk -and -not $Force) {
        Write-Host "Skipping existing ultralight SDK (use -Force to refresh)."
        return
    }

    if (Test-Path $sdkDir) {
        Remove-Item -Recurse -Force $sdkDir
    }

    $baseTemp = if ($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { [System.IO.Path]::GetTempPath() }
    $tempDir = Join-Path $baseTemp "ultralight-sdk"
    if (Test-Path $tempDir) {
        Remove-Item -Recurse -Force $tempDir
    }
    New-Item -ItemType Directory -Path $tempDir -Force | Out-Null

    $archivePath = Join-Path $tempDir "ultralight-sdk.7z"
    Write-Host "Downloading Ultralight SDK from $ultralightSdkUrl ..."
    Invoke-WebRequest -Uri $ultralightSdkUrl -OutFile $archivePath

    $sevenZip = Get-SevenZipExe
    $extractDir = Join-Path $tempDir "extract"
    New-Item -ItemType Directory -Path $extractDir -Force | Out-Null

    & $sevenZip x $archivePath "-o$extractDir" -y
    if ($LASTEXITCODE -ne 0) { throw "Failed to extract Ultralight SDK archive." }

    $candidates = @($extractDir) + (Get-ChildItem $extractDir -Directory -Recurse | Select-Object -ExpandProperty FullName)
    $sdkRoot = $candidates |
        Where-Object {
            (Test-Path (Join-Path $_ "include\Ultralight\Ultralight.h")) -and
            (Test-Path (Join-Path $_ "lib\Ultralight.lib")) -and
            (Test-Path (Join-Path $_ "bin\Ultralight.dll"))
        } |
        Select-Object -First 1

    if (-not $sdkRoot) {
        throw "Unable to locate Ultralight SDK root in extracted archive."
    }

    New-Item -ItemType Directory -Path $sdkDir -Force | Out-Null
    Get-ChildItem $sdkRoot -Force | ForEach-Object {
        Copy-Item $_.FullName $sdkDir -Recurse -Force
    }

    $rendererHeader = Join-Path $sdkDir "include\Ultralight\Renderer.h"
    if (-not (Test-Path $rendererHeader)) {
        throw "Ultralight SDK missing Renderer.h: $rendererHeader"
    }

    $hasRefreshDisplay = Select-String -Path $rendererHeader -Pattern "RefreshDisplay\(" -Quiet
    if (-not $hasRefreshDisplay) {
        throw "Ultralight SDK API is incompatible (Renderer::RefreshDisplay missing). Use your local-compatible 1.4 SDK URL."
    }

    Write-Host "Ultralight SDK installed to $sdkDir"
}

function Resolve-ExtractedThirdPartyRoot {
    param(
        [string]$ExtractDir
    )

    $directMatch = @(
        (Join-Path $ExtractDir "raylib-master"),
        (Join-Path $ExtractDir "raygui-master"),
        (Join-Path $ExtractDir "json-develop"),
        (Join-Path $ExtractDir "nbnet"),
        (Join-Path $ExtractDir "ultralight")
    ) | ForEach-Object { Test-Path $_ } | Where-Object { $_ } | Measure-Object

    if ($directMatch.Count -eq 5) {
        return $ExtractDir
    }

    $tpNested = Join-Path $ExtractDir "third_party"
    if (Test-Path (Join-Path $tpNested "raylib-master")) {
        return $tpNested
    }

    foreach ($dir in (Get-ChildItem $ExtractDir -Directory -ErrorAction SilentlyContinue)) {
        if (Test-Path (Join-Path $dir.FullName "raylib-master")) {
            return $dir.FullName
        }
    }

    throw "Unable to locate extracted third_party root in $ExtractDir"
}

function Install-ThirdPartyArchive {
    if ([string]::IsNullOrWhiteSpace($thirdPartyArchiveUrl)) {
        return $false
    }

    Write-Host "Installing third_party from archive URL: $thirdPartyArchiveUrl"

    if (Test-Path $ThirdPartyDir) {
        if ($Force) {
            Remove-Item -Recurse -Force $ThirdPartyDir
        } else {
            Write-Host "Skipping archive install because $ThirdPartyDir exists (use -Force to refresh)."
            return $true
        }
    }

    $baseTemp = if ($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { [System.IO.Path]::GetTempPath() }
    $tempDir = Join-Path $baseTemp "third-party-archive"
    if (Test-Path $tempDir) {
        Remove-Item -Recurse -Force $tempDir
    }
    New-Item -ItemType Directory -Path $tempDir -Force | Out-Null

    $archiveName = [System.IO.Path]::GetFileName(($thirdPartyArchiveUrl -split '\?')[0])
    if ([string]::IsNullOrWhiteSpace($archiveName)) {
        $archiveName = "third_party.zip"
    }
    $archivePath = Join-Path $tempDir $archiveName
    $extractDir = Join-Path $tempDir "extract"
    New-Item -ItemType Directory -Path $extractDir -Force | Out-Null

    $headers = @{}
    if (($thirdPartyArchiveUrl -match "github\.com") -and -not [string]::IsNullOrWhiteSpace($env:GITHUB_TOKEN)) {
        $headers["Authorization"] = "Bearer $($env:GITHUB_TOKEN)"
        $headers["Accept"] = "application/octet-stream"
    }

    if ($headers.Count -gt 0) {
        Invoke-WebRequest -Uri $thirdPartyArchiveUrl -OutFile $archivePath -Headers $headers
    } else {
        Invoke-WebRequest -Uri $thirdPartyArchiveUrl -OutFile $archivePath
    }

    if ($archivePath.ToLowerInvariant().EndsWith(".zip")) {
        Expand-Archive -Path $archivePath -DestinationPath $extractDir -Force
    } else {
        $sevenZip = Get-SevenZipExe
        & $sevenZip x $archivePath "-o$extractDir" -y
        if ($LASTEXITCODE -ne 0) { throw "Failed to extract third_party archive." }
    }

    $resolvedRoot = Resolve-ExtractedThirdPartyRoot -ExtractDir $extractDir
    New-Item -ItemType Directory -Path $ThirdPartyDir -Force | Out-Null
    Get-ChildItem $resolvedRoot -Force | ForEach-Object {
        Copy-Item $_.FullName $ThirdPartyDir -Recurse -Force
    }

    Write-Host "third_party installed from archive."
    return $true
}

New-Item -ItemType Directory -Path $ThirdPartyDir -Force | Out-Null

$installedFromArchive = Install-ThirdPartyArchive
if (-not $installedFromArchive) {
    foreach ($dep in $deps) {
        Sync-Repo -Dep $dep
    }

    Install-UltralightSdk
}

$required = @(
    "raylib-master/src/raylib.h",
    "raygui-master/src/raygui.h",
    "json-develop/include/nlohmann/json.hpp",
    "nbnet/nbnet.h",
    "ultralight/include/Ultralight/Ultralight.h",
    "ultralight/lib/Ultralight.lib",
    "ultralight/bin/Ultralight.dll",
    "ultralight/resources/icudt67l.dat"
)

foreach ($relativePath in $required) {
    $fullPath = Join-Path $ThirdPartyDir $relativePath
    if (-not (Test-Path $fullPath)) {
        throw "Missing dependency file: $fullPath"
    }
}

Write-Host "third_party dependencies are ready."
