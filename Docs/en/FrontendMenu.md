# Main menu, settings, and network sessions

Language: [Русский](../ru/FrontendMenu.md) | **English**

Map: `/Game/Frontend/Maps/L_MainMenu`, assigned as `GameDefaultMap`. The editor startup map remains `Map_Ex`; open `Content/Frontend/Maps/L_MainMenu` and Play to view the menu.

## Screens and layers

`AFrontendMenuHUD` derives from `AGameHUD_Inz`, creates the existing `WBP_PrimaryLayout`, and pushes `WBP_FrontendMenu` to the third layer using `PushContentToLayer(UI.Layer.Menu, ...)`. Gameplay layers are collapsed on this map. The main screen links to saving, loading, settings, and networking. Each section is a separate Widget Blueprint; Esc returns to the main screen. Display-mode confirmation stays within Settings and ends through a button or timer.

Assets are in `Content/Frontend`. Edit `WBP_FrontendMenu`, `WBP_MenuButton`, `WBP_ServerRow` in UMG Designer. Screens derive from `UInvenzaBaseWidget`, buttons from `UUIButton`, and labels use `ULabelBaseText`. Original shared plugin Widget Blueprints were not changed for this frontend.

## Graphics and audio

`UInvenzaGameUserSettings` stores resolution, display mode, VSync, FPS, quality preset, and three volume levels in the user's `GameUserSettings.ini`. Resolution/mode changes require confirmation within 15 seconds; cancel/timeout restores saved settings. Defaults changes the form first; apply to commit.

Sound classes: `SC_Master` → `SC_Music`, `SC_Effects`. `SM_UserSettings` reapplies volumes on map changes. New project sounds default to Effects; assign `SC_Music` to music Sound Waves/Cues. Include existing custom classes in this hierarchy if master volume should affect them. No demo music was added.

## LAN on one computer

The default uses `OnlineSubsystemNull` and `IpNetDriver`, without Steam login. From the project root, use two PowerShell terminals:

```powershell
.\Scripts\Launch-Frontend.ps1 -Profile LAN -WindowX 30 -Port 7777
.\Scripts\Launch-Frontend.ps1 -Profile LAN -WindowX 700 -Port 7779
```

First window: Network game → name/player count → Create and play. Second: Find/Refresh → select the session, or connect to `127.0.0.1:7777`. From another LAN computer, use the host's IPv4 address. Allow local network traffic through Windows Firewall; discovery uses UDP broadcast.

The host opens `Map_Ex?listen`. Press **M** to open the common menu during standalone or network play. **M**, **Escape**, or Continue closes it. No persistent session-bar widget is created. Continue game closes it; Main menu ends the session with saving. Function keys are not assigned for menu entry, save/load, or exit. A disconnect returns clients to the menu with a message. `UInvenzaSessionsSubsystem` retains operation state across map travel.

For inventory network development, PIE with two players, New Editor Window, Play As Listen Server connects players automatically; it does not test the session browser.

## Steam

The project enables `OnlineSubsystemSteam` and `SteamSockets`. `Config/Custom/Steam/DefaultEngine.ini` selects `SteamSocketsNetDriver` and test `SteamDevAppId=480`. This shared App ID uses a project search filter and result revalidation. If Steam is unavailable, the menu reports an error instead of silently creating LAN.

Log into Steam, then run:

```powershell
.\Scripts\Launch-Frontend.ps1 -Profile Steam
```

This launches the editor executable in standalone mode with `-game -CustomConfig=Steam`. Test Steam connections on two computers with different accounts and matching project versions. Use LAN for two local development instances.

For a **Development** package, pass `-CustomConfig=Steam`. Maps and Frontend are included in cook; the Steam configuration directory is staged. When launching outside Steam, if needed, place `steam_appid.txt` containing `480` next to the actual game executable in `Binaries/Win64`. Do not ship that file in a Steam distribution. For release, replace 480 with your App ID and select the Steam configuration during building; Shipping command-line CustomConfig switching is restricted by the engine by default.

## Checks and maintenance

- `Invenza.Frontend.MenuFlow`: editor checks for layering, navigation, canceled settings, IP validation, and canceled search.
- `Invenza.Frontend.LANHost` / `LANClient`: two separate `-game -nosteam` processes sharing a unique `-FrontendTestRun=<id>`; real discovery, joining, travel, and host-exit recovery.
- `Invenza.Frontend.SteamLifecycle`: standalone `-CustomConfig=Steam`; Steam initialization, temporary session, Steam Sockets transport, exit.
- `Scripts/UI/create_menu_hub.py`: creates current pages after building the Editor target and removes old save/load bindings. `create_frontend.py` belongs to the original version. Regeneration replaces manual edits to regenerated pages.
- `Scripts/UI/preview_frontend.py`: three-screen PNGs in `Saved/Frontend`; run through a full editor's `-ExecutePythonScript -RenderOffscreen`, not a Python commandlet.

Inventory network correctness was outside the original frontend work. Travel-related deferred UI initialization was adjusted to use a UObject-bound timer canceled in EndPlay.

The existing project notes report successful checks on **16 October 2026, UE 5.6.1**: Development Editor build, MenuFlow PIE, separate LANHost/LANClient processes, and SteamLifecycle with Steam running/App ID 480. Two-account Steam joining and full packaging were not verified there. Previews were checked at 1440 × 960 and 1280 × 720. These historical checks were not rerun for this translation.

## Common menu pages and settings

- `/Game/Frontend/UI/WBP_FrontendMenu`: root navigation.
- `/Game/Frontend/UI/WBP_SettingsPage`: graphics/audio and display confirmation.
- `/Game/Frontend/UI/WBP_NetworkPage`: hosting, finding, joining.
- `/InventorySystemInvenzaPlugin/UI/SaveLoad/WBP_SaveGamePage` and `WBP_LoadGamePage`: separate persistence pages.
- In-game access: **M**, configured by `MenuKey` under **Project Settings → Invenza Main Menu → Input**. `WBP_SessionBar` is a legacy unused asset and is not added to the viewport.

Under **Project Settings → Invenza Main Menu**, assign root menu, menu key, PrimaryLayout, gameplay world, and String Table. New page text is in `/Game/Frontend/Localization/ST_FrontendMenu`, with source `Scripts/UI/Data/FrontendMenuTexts.csv`. Edit dimensions/styling in Designer. Mode, quality, FPS lists, and confirmation duration are in `WBP_SettingsPage` Class Defaults.

Saving is disabled in the startup menu; loading opens the selected gameplay save. During gameplay it applies the snapshot without returning to the startup map. The common menu blocks movement/look and restores them on close. The network page requests returning to the main menu before connecting elsewhere.

Checks after page separation: `Invenza.Frontend.MenuFlow`, `Invenza.Inventory.MenuLoad`. Page structure is authored outside C++.

The automated test sources mentioned in historical notes were removed at the owner's request. They are not part of the current source distribution. Follow the manual scenarios instead.
