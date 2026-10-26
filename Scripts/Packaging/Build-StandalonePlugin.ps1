# Nublin Studio 2026 All Rights Reserved.

param(
    [Parameter(Mandatory = $true)]
    [string]$EngineDirectory,
    [Parameter(Mandatory = $true)]
    [string]$OutputDirectory,
    [string]$TargetPlatforms = 'Win64',
    [switch]$StrictIncludes
)

$ErrorActionPreference = 'Stop'
$projectDirectory = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$pluginDescriptor = Join-Path $projectDirectory 'Plugins\InventorySystemInvenzaPlugin\InventorySystemInvenzaPlugin.uplugin'
$packageDirectory = [System.IO.Path]::GetFullPath($OutputDirectory)
$enginePath = [System.IO.Path]::GetFullPath($EngineDirectory)
$automationTool = Join-Path $enginePath 'Engine\Build\BatchFiles\RunUAT.bat'

if (-not (Test-Path -LiteralPath $automationTool -PathType Leaf))
{
    throw "RunUAT was not found: $automationTool"
}

# BuildPlugin clears an existing destination. Require a new directory explicitly.
if (Test-Path -LiteralPath $packageDirectory)
{
    throw 'The output directory already exists. Choose a new directory to preserve previous builds.'
}

$pluginSettings = Get-Content -LiteralPath $pluginDescriptor -Raw | ConvertFrom-Json
$requestedPlatforms = @($TargetPlatforms.Split('+', [System.StringSplitOptions]::RemoveEmptyEntries) | ForEach-Object { $_.Trim() })
if ($requestedPlatforms.Count -eq 0)
{
    throw 'Specify at least one target platform.'
}

foreach ($requestedPlatform in $requestedPlatforms)
{
    if ($requestedPlatform -notin $pluginSettings.SupportedTargetPlatforms)
    {
        throw "The plugin does not declare support for $requestedPlatform. Supported platforms: $($pluginSettings.SupportedTargetPlatforms -join ', ')."
    }
}

$engineVersionFile = Join-Path $enginePath 'Engine\Build\Build.version'
$engineVersion = Get-Content -LiteralPath $engineVersionFile -Raw | ConvertFrom-Json
$pluginEngineVersion = [System.Version]::Parse($pluginSettings.EngineVersion)
if ($engineVersion.MajorVersion -ne $pluginEngineVersion.Major -or $engineVersion.MinorVersion -ne $pluginEngineVersion.Minor)
{
    throw "The plugin declares UE $($pluginEngineVersion.Major).$($pluginEngineVersion.Minor); the selected engine is UE $($engineVersion.MajorVersion).$($engineVersion.MinorVersion). Validate that version before packaging it."
}

$pluginDirectory = [System.IO.Path]::GetDirectoryName($pluginDescriptor)
if ($pluginDirectory.StartsWith($packageDirectory.TrimEnd('\') + '\', [System.StringComparison]::OrdinalIgnoreCase))
{
    throw 'The output directory must not contain the source plugin.'
}

$buildArguments = @('BuildPlugin', "-Plugin=$pluginDescriptor", "-Package=$packageDirectory", "-TargetPlatforms=$($requestedPlatforms -join '+')")
if ($StrictIncludes)
{
    $buildArguments += '-StrictIncludes'
}

& $automationTool @buildArguments
if ($LASTEXITCODE -ne 0)
{
    throw "BuildPlugin failed with exit code $LASTEXITCODE."
}

& (Join-Path $PSScriptRoot 'Repair-StandalonePluginDescriptor.ps1') -SourceDescriptor $pluginDescriptor -PackagedDescriptor (Join-Path $packageDirectory 'InventorySystemInvenzaPlugin.uplugin')
if (Test-Path -LiteralPath (Join-Path $packageDirectory 'HostProject'))
{
    throw 'UAT left a temporary HostProject. Export a clean release copy before archiving.'
}
