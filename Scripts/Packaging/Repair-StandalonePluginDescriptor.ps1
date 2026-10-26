# Nublin Studio 2026 All Rights Reserved.
param(
    [Parameter(Mandatory = $true)][string]$SourceDescriptor,
    [Parameter(Mandatory = $true)][string]$PackagedDescriptor
)
$ErrorActionPreference = 'Stop'
$sourcePath = [System.IO.Path]::GetFullPath($SourceDescriptor)
$packagedPath = [System.IO.Path]::GetFullPath($PackagedDescriptor)
if ($sourcePath -eq $packagedPath)
{
    throw 'The source descriptor must not be rewritten.'
}
$sourceSettings = Get-Content -LiteralPath $sourcePath -Raw | ConvertFrom-Json
$packagedSettings = Get-Content -LiteralPath $packagedPath -Raw | ConvertFrom-Json
$sourceVersion = [System.Version]::Parse($sourceSettings.EngineVersion)
$builtVersion = [System.Version]::Parse($packagedSettings.EngineVersion)
if ($sourceVersion.Major -ne $builtVersion.Major -or $sourceVersion.Minor -ne $builtVersion.Minor)
{
    throw 'Source and packaged engine release lines do not match.'
}
# Keep UAT's installation/version fields, and preserve all publisher metadata.
foreach ($property in $sourceSettings.PSObject.Properties)
{
    if ($property.Name -notin @('Installed', 'EngineVersion'))
    {
        $packagedSettings | Add-Member -NotePropertyName $property.Name -NotePropertyValue $property.Value -Force
    }
}
$packagedSettings | Add-Member -NotePropertyName Installed -NotePropertyValue $true -Force
$descriptorJson = ConvertTo-Json -InputObject $packagedSettings -Depth 64
[System.IO.File]::WriteAllText($packagedPath, $descriptorJson + [System.Environment]::NewLine, [System.Text.UTF8Encoding]::new($false))
