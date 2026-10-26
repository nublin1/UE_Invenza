param(
    [ValidateSet('LAN','Steam')][string]$Profile = 'LAN',
    [string]$EditorPath = 'C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe',
    [int]$WindowX = 80,
    [int]$Port = 7777
)
$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'UE_GridInventory.uproject'
$launchArguments = @($projectFile, '/Game/Frontend/Maps/L_MainMenu', '-game', '-windowed', '-ResX=1280', '-ResY=900', "-WinX=$WindowX", '-WinY=40', "-port=$Port", '-log')
if ($Profile -eq 'Steam') { $launchArguments += '-CustomConfig=Steam' }
else { $launchArguments += '-nosteam' }
& $EditorPath @launchArguments
