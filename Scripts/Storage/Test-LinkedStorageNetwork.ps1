# Nublin Studio 2026 All Rights Reserved.
# Runs only the dedicated validation host; never connects to an existing game.
param([string]$RunName = 'Network')
$ErrorActionPreference = 'Stop'
$projectRoot = 'C:/Users/reddr/AppData/Local/Temp/ILS2410/H'
$evidence = 'G:/UEProjects/UE_Invenza/Saved/LinkedStorage20261024'
$network = Join-Path $evidence $RunName
if ($RunName -notmatch '^[A-Za-z0-9_-]+$' -or (Test-Path -LiteralPath $network))
{
    throw 'Choose a new RunName containing only letters, digits, underscores or hyphens.'
}
New-Item -ItemType Directory -Path $network -Force | Out-Null
$engine = 'C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$project = Join-Path $projectRoot 'InvenzaFabAudit.uproject'
$serverArgs = @($project, '/Engine/Maps/Entry?game=/Script/InvenzaFabAudit.LinkedStorageNetGameMode', '-server', '-port=17891', '-Unattended', '-NullRHI', '-NoSplash', '-NoSound', '-UTF8Output', "-AbsLog=$network/server.log", "-LinkedStorageReport=$network/server.json")
$processes = @()
try
{
    $processes += Start-Process -FilePath $engine -ArgumentList $serverArgs -WindowStyle Hidden -PassThru
    Start-Sleep -Seconds 5
    foreach ($index in @(1,2))
    {
        $clientArgs = @($project, '127.0.0.1:17891', '-game', '-Unattended', '-NullRHI', '-NoSplash', '-NoSound', '-UTF8Output', "-AbsLog=$network/client$index.log")
        $processes += Start-Process -FilePath $engine -ArgumentList $clientArgs -WindowStyle Hidden -PassThru
        Start-Sleep -Seconds 2
    }
    $deadline = (Get-Date).AddMinutes(3)
    while (-not (Test-Path -LiteralPath (Join-Path $network 'server.json')))
    {
        if ((Get-Date) -gt $deadline -or $processes[0].HasExited)
        {
            throw 'Network validation timed out or server exited before reporting.'
        }
        Start-Sleep -Seconds 1
    }
    $result = Get-Content -LiteralPath (Join-Path $network 'server.json') -Raw | ConvertFrom-Json
    if (-not $result.passed)
    {
        throw 'Two-client network validation failed; see server.log.'
    }
    Write-Output 'Dedicated server and two remote clients passed linked-storage validation.'
    Start-Sleep -Seconds 4
}
finally
{
    foreach ($process in $processes)
    {
        if (-not $process.HasExited)
        {
            Stop-Process -InputObject $process -Force
        }
    }
}
