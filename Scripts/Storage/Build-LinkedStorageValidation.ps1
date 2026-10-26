# Nublin Studio 2026 All Rights Reserved.
param([switch]$Prepare)
$ErrorActionPreference = 'Stop'
$taskRoot = 'C:/Users/reddr/AppData/Local/Temp/ILS2410'
$projectRoot = Join-Path $taskRoot 'H'
$source = 'G:/UEProjects/UE_Invenza/Plugins/InventorySystemInvenzaPlugin'
$evidence = 'G:/UEProjects/UE_Invenza/Saved/LinkedStorage20261024'
if ($Prepare)
{
    if (Test-Path -LiteralPath $taskRoot)
    {
        throw 'Validation root already exists; preserve previous results.'
    }
    New-Item -ItemType Directory -Path $projectRoot -Force | Out-Null
    Copy-Item 'C:/Users/reddr/AppData/Local/Temp/IFab2410/H/Source' $projectRoot -Recurse
    Copy-Item 'C:/Users/reddr/AppData/Local/Temp/IFab2410/H/InvenzaFabAudit.uproject' $projectRoot
    New-Item -ItemType Directory -Path (Join-Path $projectRoot 'Config'),(Join-Path $projectRoot 'Saved/UnrealBuildTool') -Force | Out-Null
    Copy-Item 'C:/Users/reddr/AppData/Local/Temp/IFab2410/H/Saved/UnrealBuildTool/BuildConfiguration.xml' (Join-Path $projectRoot 'Saved/UnrealBuildTool')
    [System.IO.File]::WriteAllText((Join-Path $projectRoot 'Config/DefaultEngine.ini'), "[/Script/EngineSettings.GameMapsSettings]`nGameDefaultMap=/Engine/Maps/Entry`nEditorStartupMap=/Engine/Maps/Entry`n`n[/Script/Engine.RendererSettings]`nr.AllowStaticLighting=False`n")
    [System.IO.File]::WriteAllText((Join-Path $projectRoot 'Config/DefaultInventorySystemInvenzaPlugin.ini'), "[/Script/InventorySystemInvenzaPlugin.InvenzaSaveSettings]`nProfile=LinkedStorageValidation`nbAutoLoad=False`nbAutoSave=False`nbSaveOnExit=False`n")
}
& robocopy (Join-Path $source 'Source') (Join-Path $projectRoot 'Plugins/InventorySystemInvenzaPlugin/Source') /E /R:0 /W:0 /NFL /NDL /NJH /NJS /NP
if ($LASTEXITCODE -gt 7)
{
    throw 'Source copy failed.'
}
if ($Prepare)
{
    foreach ($folder in @('Content','Config','Resources'))
    {
        & robocopy (Join-Path $source $folder) (Join-Path $projectRoot "Plugins/InventorySystemInvenzaPlugin/$folder") /E /R:0 /W:0 /NFL /NDL /NJH /NJS /NP
        if ($LASTEXITCODE -gt 7)
        {
            throw "Copy failed: $folder"
        }
    }
    Copy-Item (Join-Path $source 'InventorySystemInvenzaPlugin.uplugin') (Join-Path $projectRoot 'Plugins/InventorySystemInvenzaPlugin')
    Copy-Item (Join-Path $source 'Config/DefaultInventorySystemInvenzaPlugin.ini') (Join-Path $projectRoot 'Config/DefaultInventorySystemInvenzaPlugin.ini')
    Add-Content (Join-Path $projectRoot 'Config/DefaultInventorySystemInvenzaPlugin.ini') "`n[/Script/InventorySystemInvenzaPlugin.InvenzaSaveSettings]`nProfile=LinkedStorageValidation`nbAutoLoad=False`nbAutoSave=False`nbSaveOnExit=False"
}
& 'C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat' InvenzaFabAuditEditor Win64 Development "-Project=$projectRoot/InvenzaFabAudit.uproject" -NoUBA -NoHotReloadFromIDE -WaitMutex *> (Join-Path $evidence 'build-editor.log')
if ($LASTEXITCODE -ne 0)
{
    Get-Content (Join-Path $evidence 'build-editor.log') -Tail 65
    throw 'Linked storage editor validation build failed.'
}
Write-Output 'Linked storage editor build passed.'
