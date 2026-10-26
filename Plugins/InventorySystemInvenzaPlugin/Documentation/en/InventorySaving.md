# Saving inventory state

Language: [Русский](../ru/InventorySaving.md) | **English**

## Integration with your project

`InvenzaSaveSubsystem` is an automatically created GameInstance subsystem. `InventoryCollectionComponent` and `PickupComponent` register themselves through their implementations. A separate SaveGame manager on every chest or NPC is unnecessary.

1. Set up working inventories using the [integration guide](ProjectIntegration.md).
2. Under Project Settings → Invenza Save System, choose a profile and configure Auto Load, Auto Save, Save On Exit, and Enable In PIE. Use a separate profile such as `InventoryTest` for isolated checks.
3. Assign unique persistent `PersistentOwnerID` values to placed owners and `PersistentPickupID` values to placed pickups. Set `CustomInventoryID` for inventories whose order or tags may change. Do not duplicate GUIDs across independent instances.
4. Retain item tables/rows, equipment definitions, and recipes, or provide compatible replacements. A snapshot references definitions; moving a save file does not move those assets.
5. For custom UI, get `InvenzaSaveSubsystem` from GameInstance and call `SaveSession`, `LoadSession`, or `ShowSaveMenu` for the local controller. The example [frontend](FrontendMenu.md) also requires demo-project maps and classes.
6. Check `SaveMenuClass` and `TextTable`. For loading from the startup menu, assign the gameplay level through `Invenza Main Menu.PlayWorld`.
7. Change a bag and chest, save a manual slot, change them again, and load that slot. Check equipment, world pickups, and unfinished crafting separately.

`UIInventoryManager` notifies the subsystem after player initialization. `InvenzaSaveManager` provides compatibility commands and `OnGameLoaded`. Its legacy `SaveSlotName` no longer selects the profile; use project settings. `SaveAction` and `LoadAction` remain in the class but are not automatically bound by the current component. Connect custom keys explicitly to the API.

## Custom buttons and events

For a save button, call `SaveSession("Manual1", true)` on the host. Load it with `LoadSession("Manual1")`; `LoadSession("Continue")` chooses the latest successful record. Verify built-in manual slot keys against the widget implementation when building your own list.

An asynchronous `SaveSession` return does not mean disk writing has finished. Use `OnStateChanged`, `Status`, and `bBusy` to update UI; `OnSessionLoaded` reports snapshot application. Avoid load callbacks that grant rewards already restored by the snapshot.

Use `GetSaveSlotsForWorld` and `LoadWorldSave` to load from another map. Do not apply a different world's snapshot over the current level. Confirm before calling `ResetPlaythrough`: it removes current profile/world progress and restarts the level.

Reopen shared storage, mannequin exchange, and trading windows after loading. Their owners' items are saved through collections; temporary permissions and window state are not.

## Current implementation

The common menu opens separate **Save game** and **Load game** pages, `WBP_SaveGamePage` and `WBP_LoadGamePage`. In gameplay, the **Menu** button is at the upper right. The example does not assign `-`, `=`, F5/F9, or F10 to these operations. `WBP_InvenzaSaveMenu` remains the shared base; Core buttons and `WBP_SaveLabel` supply styling, while C++ supplies behavior and bindings.

On the startup map, Load game lists saves for `Invenza Main Menu.PlayWorld`. Selecting one opens that level and applies the selected snapshot after inventories become ready; auto-load does not replace that selection. Saving is unavailable before gameplay begins. In network games, the host controls saving and loading.

Snapshots include registered player, NPC, vendor, and container collections; grid/list placements; physical and reference equipment slots. Each item instance is stored once; equipment and hotbar entries reference it. Interaction windows, permissions, and active trade sessions are not restored.

Stations save crafting queues, selected alternatives, progress, blocks, fuel time, paid resources, and pending outputs. Operators and current visitors are not saved; conditions are checked again. Missing definitions or recipients required to restore crafting cause rejection before inventories are changed.

`PickupComponent` participates separately: world items, collected placed pickups, and items dropped after a save are reconciled on load. Old floor items must not remain as extra copies of restored items.

## Commands

- Quick save: dedicated quick slot.
- Save to slot 1–3: manual saves.
- Continue: latest successful record for the current profile and world.
- Load selected save: selected record.
- Retry items in recovery: attempt to place saved stacks not yet restored.
- Start again / Reset level: confirmation, new playthrough generation, and level restart. Old saves for this profile/world are removed. A new-playthrough marker prevents old files from auto-loading even if deletion fails. Graphics, audio, and control settings are preserved.

Only the host can save, load, or reset in network play. Server reset uses ServerTravel; standalone reloads the current level.

## Settings

Project Settings → Invenza Save System (`UInvenzaSaveSettings`, Config=InventorySystemInvenzaPlugin):

- `Profile`: profile name, default `Default`.
- `bAutoLoad`: load the latest record after player inventories are ready.
- `bAutoSave`: automatically save changed sessions.
- `AutoSaveSeconds`: interval, default 120 seconds.
- `AutoSaveCount`: rotating auto-save count, default 3.
- `bSaveOnExit`: save on graceful exit/world teardown, not forced process termination.
- `bEnableInPIE`: editor saves use a separate PIE prefix and cannot overwrite normal-run files.
- `ClientTrust`: `ServerValidated` (default) or `TrustedClient`.
- `SaveMenuClass`: default `/InventorySystemInvenzaPlugin/UI/SaveLoad/WBP_InvenzaSaveMenu`.
- `TextTable`: default `/InventorySystemInvenzaPlugin/Data/Localization/ST_InvenzaSaveTexts` for UI/status text.

## Styling and localization

- Edit layout, dimensions, background, and padding in `WBP_InvenzaSaveMenu` Designer.
- Configure text font/color on its `WBP_SaveLabel` instances without changing shared Core assets.
- Button text, headings, reset confirmation, slot labels, date/content formats, and status text come from `ST_InvenzaSaveTexts`. Designer text properties retain table-entry references.
- Gather translations from this asset through Localization Dashboard. Source values also live in `Scripts/SaveLoad/Data/InvenzaSaveTexts.csv`; `create_save_menu.py` reimports them, so synchronize CSV with table changes before rerunning it.
- `SlotEntry`, `RecoveryItem`, `AutoSlot`, `LoadedRecovery`, and `RecoveryRemaining` use `{0}` / `{1}` arguments. Labels are not parsed into commands: the window stores a separate label/file-key mapping.
- Select another window class or String Table in Project Settings; the Blueprint path is not hardcoded in the C++ constructor.
- Detailed errors go to the log; UI shows localizable error messages.

Example `DefaultInventorySystemInvenzaPlugin.ini`:

```ini
[/Script/InventorySystemInvenzaPlugin.InvenzaSaveSettings]
Profile=Default
bAutoLoad=True
bAutoSave=True
AutoSaveSeconds=120.0
AutoSaveCount=3
bSaveOnExit=True
bEnableInPIE=True
ClientTrust=ServerValidated
```

Files use SaveGame API under `Saved/SaveGames`. Names contain profile, world key, logical slot, and generation 0/1. The older generation is written while the other successful generation remains a backup. Snapshots are captured on the game thread; normal writes are asynchronous. One write runs at a time. Other slots queue in order; repeated requests for the same waiting slot are coalesced. Changes made during writing remain dirty until a later save.

Graceful world teardown uses a separate synchronous Shutdown save. Exit saving is disabled in unattended automated checks to avoid overwriting user progress.

## Identities

- `InventoryCollectionComponent.PersistentOwnerID`: permanent GUID of an owner.
- `PickupComponent.PersistentPickupID`: permanent GUID of a placed world item.
- `InventorySettings.CustomInventoryID`: recommended stable key within an owner. Otherwise the tag and index among inventories with that tag are used; changing order or tags requires migration.
- `UItemBase.PersistentInstanceID`: GUID of an instance/stack. Creation and splitting generate new IDs; transfer preserves them.

`Scripts/SaveLoad/assign_example_identities.py` assigns level-instance GUIDs in `Map_Ex`. Set `PersistentOwnerID` in Details for other placed owners. Without a GUID, the compatible object-path fallback survives restarts but not renaming or moving actors between levels. Duplicated instances must receive unique GUIDs.

The host uses `Player:Host`. Network players use server-side `UniqueNetId`. LAN without persistent accounts falls back to PlayerState name; production account integration should supply stable identity through `PersistentOwnerID`. The server-approved identity replicates to clients.

Dynamic inventory actors retain runtime identity, class, and transform for recreation. The snapshot does not serialize arbitrary level/component, AI, or character state; integrate that separately. Unloaded collections remain in retained records until owners appear and become ready.

## API and restoration

The GameInstance coordinator exposes `SaveSession`, `LoadSession`, `ResetPlaythrough`, `GetSaveSlots`, `GetSaveSlotsForWorld`, `LoadWorldSave`, `RetryRecovery`, and `ShowSaveMenu`. Collection persistence uses `CapturePersistence`, `PreparePersistence`, and `CommitPersistence`, separate from legacy `SerializeForSave` / `DeserializeFromSave` used by transfer simulation.

Loading validates identities and prepares new instances and placements first. Only after successful preparation does it close interactions and replace collections. Empty records clear inventories. Equipment, totals, and visual representations are then restored.

Ordinary items that cannot be restored because of missing definitions, changed stack size, or insufficient space keep their full record in `RecoveryItems`. A conflicting original cell first triggers a search for another suitable location. Recovery is visible in the menu and through `GetRecoveryItems` and is included in later saves.

Extend custom mutable item state through `UItemBase.CaptureSaveState` / `RestoreSaveState`. Implementations must operate only on the created instance's state and must not trigger external gameplay operations.

## Trusted clients

Only the server selects this mode, through settings or the host menu checkbox. The value replicates. Ordinary transfers retain their existing transactional path.

For integrations where clients compute their own bag state, `UIInventoryManager.SubmitTrustedInventoryState` sends an owner record through the player's manager. The server accepts that character's whole record without item-origin, weight, or stack-count-limit checks. Clients can create items by design in this mode.

Integrity checks remain: own owner, registered inventories/dimensions, existing definitions, valid positive quantities, equipment categories, no overlaps, and no duplicate/foreign GUIDs. Clients cannot change inventory definitions, other owners, crafting queues, world files, or reset state. Only the server writes full snapshots.

Loading a server save does not reapply weight or stack-count limits: previously accepted state survives disabling client trust. Item dimensions, categories, per-stack quantities, and non-overlap are still checked.

## Checks

- `Invenza.Inventory.SessionSave`: serialization, coordinated gold restore, empty containers, equipment without copies, duplicate GUIDs, NPC/pickup recreation, recovery, accepted over-limit state, prepaid crafting continuation, backup generation, and save window.
- `Invenza.Inventory.MenuLoad`: gameplay-to-menu travel, an explicitly selected older save, content restoration, and standalone menu entry.
- `Invenza.Inventory.SessionReset`: asynchronous writes, queued requests, changes during writes, real level restart with auto-load, and reset to initial state.
- `Invenza.Inventory.SaveNetwork`: listen-server/client PIE, trust replication, real owner-state RPC, refusal with trust disabled, and client inability to save/reset the world.

Existing transfer, trade, storage, and interaction-distance checks complement these tests. They were not rerun as part of this documentation update.

Historical automation names in this guide refer to checks whose source files were removed at the owner's request. They are not available in the current distribution; use the described manual scenarios.
