# Fab source archive

Run from the project directory after a successful standalone plugin build:

```powershell
& ".\Scripts\Packaging\Export-FabSource.ps1" `
    -PluginDirectory "G:\UEBuilds\Invenza_Fab_Check_UE56" `
    -OutputDirectory "G:\UEBuilds\Invenza_Fab_Source_UE56" `
    -ArchivePath "G:\UEBuilds\Invenza_UE56_Fab_Source.zip"
```

Choose new output paths for every release. Without `PluginDirectory`, the script
uses this repository's current plugin sources. The input is never changed or deleted.

The output contains the descriptor, authored Source, Content, Config, Resources,
Documentation and root license/readme files. It honors exclusions in
`Config/FilterPlugin.ini`. Binaries, Intermediate, Saved, DerivedDataCache,
HostProject, IDE/VCS caches, generated code and temporary files are excluded.
Source/ThirdParty libraries and redistribution files retain their extensions;
conflicting Binaries/Intermediate directories there cause an explicit failure
instead of silently stripping dependencies.

The ZIP has exactly one plugin root. The script reopens it, checks required module
build files, Invenza's example map and theme, checks the entry list for temporary
files, and compares the SHA-256 of each archived file with its clean copy. It prints
the resulting archive's SHA-256. Full Content is retained except explicit filter
exclusions; it does not analyze Unreal's asset dependency graph or fix broken assets.

This prepares a source archive. It does not run BuildPlugin, asset validation,
cook, or a packaged-game test. Keep the binary build used for local testing separate
from the Fab source ZIP. The older Export-StandalonePlugin and
Compress-StandalonePlugin scripts serve that binary workflow.
