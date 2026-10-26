# Nublin Studio 2026 All Rights Reserved.
param(
    [Parameter(Mandatory = $true)][string]$PackageDirectory,
    [Parameter(Mandatory = $true)][string]$ArchivePath
)
$ErrorActionPreference = 'Stop'
$packageRoot = [System.IO.Path]::GetFullPath($PackageDirectory)
$archiveFile = [System.IO.Path]::GetFullPath($ArchivePath)
if (Test-Path -LiteralPath $archiveFile)
{
    throw 'Choose a new archive path to preserve previous releases.'
}
if ($archiveFile.StartsWith($packageRoot.TrimEnd('\') + '\', [System.StringComparison]::OrdinalIgnoreCase))
{
    throw 'The archive must be outside the package directory.'
}
$descriptorFile = Join-Path $packageRoot 'InventorySystemInvenzaPlugin.uplugin'
$settings = Get-Content -LiteralPath $descriptorFile -Raw | ConvertFrom-Json
if (-not $settings.FabURL -or -not $settings.CopyrightNotice -or -not $settings.Installed)
{
    throw 'Complete the packaged descriptor before archiving.'
}
foreach ($folder in @('Source', 'Content', 'Config', 'Resources', 'Documentation'))
{
    if (-not (Test-Path -LiteralPath (Join-Path $packageRoot $folder) -PathType Container))
    {
        throw "Missing required package directory: $folder"
    }
}
foreach ($folder in @('HostProject', 'Saved', '.git'))
{
    if (Test-Path -LiteralPath (Join-Path $packageRoot $folder))
    {
        throw "Temporary directory must not enter the archive: $folder"
    }
}
$excluded = @(Get-Content -LiteralPath (Join-Path $packageRoot 'Config/FilterPlugin.ini') | Where-Object { $_.StartsWith('-/') } | ForEach-Object { $_.Substring(2).Trim() })
foreach ($relativePath in $excluded)
{
    if (Test-Path -LiteralPath (Join-Path $packageRoot $relativePath))
    {
        throw "Excluded asset remains in the package: $relativePath"
    }
}
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
New-Item -ItemType Directory -Path (Split-Path -Parent $archiveFile) -Force | Out-Null
$stream = [System.IO.File]::Open($archiveFile, [System.IO.FileMode]::CreateNew)
$zip = [System.IO.Compression.ZipArchive]::new($stream, [System.IO.Compression.ZipArchiveMode]::Create, $false)
try
{
    foreach ($file in Get-ChildItem -LiteralPath $packageRoot -Recurse -File)
    {
        $relativePath = $file.FullName.Substring($packageRoot.Length + 1).Replace('\', '/')
        $entryName = 'InventorySystemInvenzaPlugin/' + $relativePath
        [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $file.FullName, $entryName, [System.IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
}
finally
{
    $zip.Dispose()
    $stream.Dispose()
}
Get-FileHash -LiteralPath $archiveFile -Algorithm SHA256
