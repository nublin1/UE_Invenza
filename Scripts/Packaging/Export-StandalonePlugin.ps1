# Nublin Studio 2026 All Rights Reserved.
param(
    [Parameter(Mandatory = $true)][string]$BuiltPluginDirectory,
    [Parameter(Mandatory = $true)][string]$OutputDirectory,
    [string]$SourcePluginDirectory = (Join-Path $PSScriptRoot '../../Plugins/InventorySystemInvenzaPlugin')
)
$ErrorActionPreference = 'Stop'
$sourceDirectory = [System.IO.Path]::GetFullPath($SourcePluginDirectory)
$builtDirectory = [System.IO.Path]::GetFullPath($BuiltPluginDirectory)
$releaseDirectory = [System.IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $releaseDirectory)
{
    throw 'Choose a new export directory to preserve earlier packages.'
}
foreach ($inputDirectory in @($sourceDirectory, $builtDirectory))
{
    if ($inputDirectory.StartsWith($releaseDirectory.TrimEnd('\') + '\', [System.StringComparison]::OrdinalIgnoreCase) -or
        $releaseDirectory.StartsWith($inputDirectory.TrimEnd('\') + '\', [System.StringComparison]::OrdinalIgnoreCase))
    {
        throw 'Input and export directories must not contain one another.'
    }
}
$descriptorName = 'InventorySystemInvenzaPlugin.uplugin'
foreach ($inputDirectory in @($sourceDirectory, $builtDirectory))
{
    if (-not (Test-Path -LiteralPath (Join-Path $inputDirectory $descriptorName)))
    {
        throw "Missing plugin descriptor in $inputDirectory"
    }
}
# Reusing compiled artifacts is safe only when their entire native source set matches.
$sourceFiles = @(Get-ChildItem -LiteralPath (Join-Path $sourceDirectory 'Source') -Recurse -File)
$builtFiles = @(Get-ChildItem -LiteralPath (Join-Path $builtDirectory 'Source') -Recurse -File)
$sourceNames = @($sourceFiles | ForEach-Object { $_.FullName.Substring($sourceDirectory.Length + 1).Replace('\', '/') })
$builtNames = @($builtFiles | ForEach-Object { $_.FullName.Substring($builtDirectory.Length + 1).Replace('\', '/') })
if (Compare-Object $sourceNames $builtNames)
{
    throw 'Native source sets differ. Run Build-StandalonePlugin again.'
}
foreach ($file in $sourceFiles)
{
    $relativePath = $file.FullName.Substring($sourceDirectory.Length + 1)
    if ((Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash -ne (Get-FileHash -LiteralPath (Join-Path $builtDirectory $relativePath) -Algorithm SHA256).Hash)
    {
        throw "Native source changed: $relativePath. Rebuild before exporting."
    }
}
$excluded = @(Get-Content -LiteralPath (Join-Path $sourceDirectory 'Config/FilterPlugin.ini') | Where-Object { $_.StartsWith('-/') } | ForEach-Object { $_.Substring(2).Trim() })
if ($excluded | Where-Object { $_ -match '[*?]' -or $_.Contains('...') })
{
    throw 'This exporter requires literal exclusion paths; use UAT for wildcard exclusions.'
}
New-Item -ItemType Directory -Path $releaseDirectory | Out-Null
$sourceFolders = @('Source', 'Content', 'Config', 'Resources', 'Documentation')
$copiedFiles = 0
foreach ($file in Get-ChildItem -LiteralPath $builtDirectory -Recurse -File)
{
    $relativePath = $file.FullName.Substring($builtDirectory.Length + 1).Replace('\', '/')
    $topFolder = $relativePath.Split('/')[0]
    if ($topFolder -in ($sourceFolders + @('HostProject', 'Saved', '.git')) -or $relativePath -in $excluded)
    {
        continue
    }
    $target = Join-Path $releaseDirectory $relativePath
    New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force | Out-Null
    Copy-Item -LiteralPath $file.FullName -Destination $target
    $copiedFiles++
}
foreach ($folder in $sourceFolders)
{
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $sourceDirectory $folder) -Recurse -File)
    {
        $relativePath = $file.FullName.Substring($sourceDirectory.Length + 1).Replace('\', '/')
        if ($relativePath -in $excluded)
        {
            continue
        }
        $target = Join-Path $releaseDirectory $relativePath
        New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force | Out-Null
        Copy-Item -LiteralPath $file.FullName -Destination $target
        $copiedFiles++
    }
}
& (Join-Path $PSScriptRoot 'Repair-StandalonePluginDescriptor.ps1') -SourceDescriptor (Join-Path $sourceDirectory $descriptorName) -PackagedDescriptor (Join-Path $releaseDirectory $descriptorName)
foreach ($excludedPath in $excluded)
{
    if (Test-Path -LiteralPath (Join-Path $releaseDirectory $excludedPath))
    {
        throw "Excluded file entered the export: $excludedPath"
    }
}
Write-Output "Exported $copiedFiles files; native source hashes match; publisher metadata preserved."
