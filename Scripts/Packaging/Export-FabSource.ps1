# Nublin Studio 2026 All Rights Reserved.

<#
.SYNOPSIS
Creates a clean source-only plugin copy and a verified Fab ZIP.
.DESCRIPTION
Accepts a working plugin or a BuildPlugin result. Never deletes or modifies input.
Does not build code, cook assets, or establish Fab submission readiness.
#>
[CmdletBinding()]
param(
    [string]$PluginDirectory,
    [Parameter(Mandatory = $true)]
    [string]$OutputDirectory,
    [string]$ArchivePath
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

if (-not $PluginDirectory)
{
    $PluginDirectory = Join-Path $PSScriptRoot '../../Plugins/InventorySystemInvenzaPlugin'
}

function Test-FabPathInside
{
    param([string]$Candidate, [string]$Root)
    return $Candidate.Equals($Root, [System.StringComparison]::OrdinalIgnoreCase) -or
        $Candidate.StartsWith($Root.TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar,
            [System.StringComparison]::OrdinalIgnoreCase)
}

$inputRoot = (Resolve-Path -LiteralPath $PluginDirectory).ProviderPath.TrimEnd('\', '/')
$outputRoot = [System.IO.Path]::GetFullPath($OutputDirectory).TrimEnd('\', '/')
if (-not $ArchivePath)
{
    $ArchivePath = $outputRoot + '.zip'
}
$zipPath = [System.IO.Path]::GetFullPath($ArchivePath)
if ((Test-FabPathInside $outputRoot $inputRoot) -or (Test-FabPathInside $inputRoot $outputRoot))
{
    throw 'Input and output directories must be separate and must not contain one another.'
}
if ((Test-FabPathInside $zipPath $inputRoot) -or (Test-FabPathInside $zipPath $outputRoot))
{
    throw 'The ZIP must be outside the input and output directories.'
}
if ([System.IO.Path]::GetExtension($zipPath) -ine '.zip')
{
    throw 'ArchivePath must end with .zip.'
}
foreach ($target in @($outputRoot, $zipPath))
{
    if (Test-Path -LiteralPath $target)
    {
        throw "Choose a new destination; this path already exists: $target"
    }
}

$descriptors = @(Get-ChildItem -LiteralPath $inputRoot -File -Filter '*.uplugin')
if ($descriptors.Count -ne 1)
{
    throw 'PluginDirectory must contain exactly one .uplugin file.'
}
$descriptor = $descriptors[0]
$pluginName = $descriptor.BaseName
$settings = Get-Content -LiteralPath $descriptor.FullName -Raw | ConvertFrom-Json
$allowedFolders = @('Source', 'Content', 'Config', 'Resources', 'Documentation')
foreach ($folder in @('Source', 'Content', 'Config', 'Resources'))
{
    if (-not (Test-Path -LiteralPath (Join-Path $inputRoot $folder) -PathType Container))
    {
        throw "Missing required input directory: $folder"
    }
}

# Respect the existing distribution exclusions, including UE ..., * and ? patterns.
$filterRules = [System.Collections.Generic.List[string]]::new()
$filterPath = Join-Path $inputRoot 'Config/FilterPlugin.ini'
if (Test-Path -LiteralPath $filterPath -PathType Leaf)
{
    $inFilterSection = $false
    foreach ($line in Get-Content -LiteralPath $filterPath)
    {
        $rule = $line.Trim()
        if ($rule.StartsWith('['))
        {
            $inFilterSection = $rule -ieq '[FilterPlugin]'
        }
        elseif ($inFilterSection -and $rule.StartsWith('-/'))
        {
            $pattern = $rule.Substring(2).Replace('\', '/')
            if ($pattern.Split('/') -contains '..' -or $pattern.Contains(':'))
            {
                throw "Invalid exclusion path: $rule"
            }
            $escaped = [regex]::Escape($pattern).Replace('\.\.\.', '.*').Replace('\*', '[^/]*').Replace('\?', '[^/]')
            $filterRules.Add('(?i)^' + $escaped + '(?:/.*)?$')
        }
    }
}

function Test-FabExcluded
{
    param([string]$RelativePath)
    $parts = $RelativePath.Replace('\', '/').Split('/')
    $temporaryFolders = @('Binaries', 'Intermediate', 'Saved', 'DerivedDataCache', 'HostProject',
        '.git', '.svn', '.hg', '.vs', '.idea', '.vscode', '.cache', '__pycache__', '.codex', '.agents')
    for ($index = 0; $index -lt $parts.Count - 1; $index++)
    {
        if ($parts[$index] -in $temporaryFolders)
        {
            if ($RelativePath -match '(?i)^Source/ThirdParty/' -and $parts[$index] -in @('Binaries', 'Intermediate'))
            {
                throw "ThirdParty dependency path requires review; it cannot be silently stripped: $RelativePath"
            }
            return $true
        }
    }
    foreach ($rule in $filterRules)
    {
        if ($RelativePath -match $rule)
        {
            return $true
        }
    }
    # Libraries, headers and redistributable DLLs under Source/ThirdParty are authored dependencies.
    if ($RelativePath -notmatch '(?i)^Source/ThirdParty/')
    {
        if ($RelativePath -match '(?i)(\.generated\.h|\.gen\.cpp|\.(obj|o|pch|gch|pdb|ipdb|iobj|precompiled|modules|target|log|tmp|bak|pyc|pyo|sln|suo|opensdf|sdf|rsp|response)|\.VC\.(db|opendb))$')
        {
            return $true
        }
    }
    return $false
}

# Explicit walk avoids following junctions/symlinks outside the selected plugin.
$files = [System.Collections.Generic.List[object]]::new()
$pending = [System.Collections.Generic.Stack[System.IO.DirectoryInfo]]::new()
$pending.Push((Get-Item -LiteralPath $inputRoot))
$skipped = 0
while ($pending.Count -gt 0)
{
    $directory = $pending.Pop()
    foreach ($entry in Get-ChildItem -LiteralPath $directory.FullName -Force)
    {
        $relative = $entry.FullName.Substring($inputRoot.Length + 1).Replace('\', '/')
        $top = $relative.Split('/')[0]
        if ($top -notin $allowedFolders -and $relative -ne $descriptor.Name -and
            $relative -notmatch '(?i)^(README|LICENSE|LICENCE|NOTICE|THIRDPARTYNOTICES)(\.[^/]+)?$')
        {
            $skipped++
            continue
        }
        if (Test-FabExcluded ($relative + $(if ($entry.PSIsContainer) { '/_' } else { '' })))
        {
            $skipped++
            continue
        }
        if ($entry.Attributes -band [System.IO.FileAttributes]::ReparsePoint)
        {
            throw "Linked paths are not copied into source archives: $relative"
        }
        if ($entry.PSIsContainer)
        {
            $pending.Push($entry)
        }
        else
        {
            $files.Add([pscustomobject]@{ RelativePath = $relative; FullName = $entry.FullName })
        }
    }
}

$required = [System.Collections.Generic.List[string]]::new()
$required.Add($descriptor.Name)
foreach ($module in $settings.Modules)
{
    $required.Add("Source/$($module.Name)/$($module.Name).Build.cs")
}
# Invenza's playable map and its resource packages are part of this source distribution.
if ($pluginName -eq 'InventorySystemInvenzaPlugin')
{
    $required.Add('Content/Maps/Map_Ex.umap')
    $required.Add('Content/Data/UI/DA_UITheme_Default.uasset')
}
$selectedNames = @($files | ForEach-Object { $_.RelativePath })
foreach ($path in $required)
{
    if ($path -notin $selectedNames)
    {
        throw "Required file is missing or excluded: $path"
    }
}

New-Item -ItemType Directory -Path $outputRoot | Out-Null
foreach ($file in $files)
{
    $target = Join-Path $outputRoot $file.RelativePath
    New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force | Out-Null
    Copy-Item -LiteralPath $file.FullName -Destination $target
}

Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
New-Item -ItemType Directory -Path (Split-Path -Parent $zipPath) -Force | Out-Null
$stream = [System.IO.File]::Open($zipPath, [System.IO.FileMode]::CreateNew)
$zip = [System.IO.Compression.ZipArchive]::new($stream, [System.IO.Compression.ZipArchiveMode]::Create, $false)
try
{
    foreach ($file in $files)
    {
        [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip,
            (Join-Path $outputRoot $file.RelativePath), "$pluginName/$($file.RelativePath)",
            [System.IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
}
finally
{
    $zip.Dispose()
    $stream.Dispose()
}

# Verify the actual ZIP, including file contents, instead of assuming the copy succeeded.
$zip = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
$sha = [System.Security.Cryptography.SHA256]::Create()
try
{
    if ($zip.Entries.Count -ne $files.Count)
    {
        throw 'ZIP verification failed: unexpected number of entries.'
    }
    $zipNames = @($zip.Entries | ForEach-Object { $_.FullName })
    foreach ($path in $required)
    {
        if ("$pluginName/$path" -cnotin $zipNames)
        {
            throw "ZIP verification failed: required file missing: $path"
        }
    }
    foreach ($entry in $zip.Entries)
    {
        if (-not $entry.FullName.StartsWith("$pluginName/", [System.StringComparison]::Ordinal))
        {
            throw "Unexpected ZIP root: $($entry.FullName)"
        }
        $relative = $entry.FullName.Substring($pluginName.Length + 1)
        if ($relative -notin $selectedNames -or (Test-FabExcluded $relative))
        {
            throw "Unexpected or temporary ZIP entry: $($entry.FullName)"
        }
        $entryStream = $entry.Open()
        try
        {
            $actualHash = [System.BitConverter]::ToString($sha.ComputeHash($entryStream)).Replace('-', '')
        }
        finally
        {
            $entryStream.Dispose()
        }
        $expectedHash = (Get-FileHash -LiteralPath (Join-Path $outputRoot $relative) -Algorithm SHA256).Hash
        if ($actualHash -ne $expectedHash)
        {
            throw "ZIP verification failed: content mismatch: $relative"
        }
    }
}
finally
{
    $sha.Dispose()
    $zip.Dispose()
}
Write-Host "Clean source copy: $outputRoot"
Write-Host "Verified Fab source ZIP: $zipPath"
Write-Host "Included $($files.Count) files; skipped $skipped temporary/excluded files or directories."
Get-FileHash -LiteralPath $zipPath -Algorithm SHA256
