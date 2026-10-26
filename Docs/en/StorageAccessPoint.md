# Base storage access point

Language: [Русский](../ru/StorageAccessPoint.md) | **English**

## Purpose and first setup

An access point opens multiple existing storages in one window. Items stay in their original inventories; no merged item collection is created. This suits a base warehouse with several chests or shelves.

First prepare player inventory, HUD, and [integration](ProjectIntegration.md)/[input](InputConfiguration.md).

1. Place two actors with `ContainerComponent` and `InventoryCollectionComponent`. Configure `StartupInventories`, `ContainerTitle`, widget classes, and contents. For grids created before UI exists, use `bCollectInvDataFromWidget = false` and configure dimensions/slot classes.
2. Place `BP_StorageAccessPoint` from `Blueprints/Examples/Storage`. Its mesh must Block the character's interaction channel.
3. Set `StorageAccess.DiscoveryMode = Explicit` and add both actors to `ExplicitContainers` to test exact membership first.
4. Check `WindowClass = WBP_StorageAccess` and choose a `WindowTitle`.
5. Coordinate `MaxUseDistance` with global `MaxInteractionDistance`. Both limit access; discovery radius is separate.
6. Play, approach, and use Primary interaction. Check both storages, player inventory, transfers, name search, closing, and reopening.

Do not reuse `CustomInventoryID` across independent inventories. Leave it empty for automatic generation if no special key is needed.

## Discovery and permissions

Use Radius or RadiusAndExplicit for an expanding warehouse. `SearchRadius` measures from the access point to container actors; 2000 cm is 20 meters. It is not the player's use distance.

For one base, set an ordinary `RequiredActorTag`, such as `Base_A`, on the access point and matching Actor Tags on containers. `RequiredInventoryTag` is a separate Gameplay Tag filter. Explicit entries bypass discovery filters, so include only intended sources.

Override `CanPlayerAccess` in Blueprint for base ownership/team permissions. `RequiredActorTag` selects sources, not players. The default accepts any valid player who passes distance checks.

After spawning a new container, call `RefreshContainers` or `AddContainer` on the server. Dynamic explicit additions need Explicit or RadiusAndExplicit. The window's search changes visibility only, not access membership.

## Suggested checks

Test adding/removing containers, distance cutoff, and access denial. In network play, open two concurrent sessions: transfers should change the real contents, and closing one must not revoke the other's access. These checks were not rerun for this documentation update.

For NPC equipment access, use [character exchange](CharacterInventoryExchange.md) instead.

## StorageAccess settings

Drag `BP_StorageAccessPoint` from the plugin's `Blueprints/Examples/Storage` onto a level. It has a replaceable mesh and `StorageAccess`. Open with normal interaction; close using Close, Escape, or the inventory key.

- `DiscoveryMode`: Radius, Explicit, or RadiusAndExplicit.
- `SearchRadius`: centimeters; automatic discovery finds actors with `UContainerComponent` and inventories in their `UInventoryCollectionComponent`.
- `RequiredActorTag`, `RequiredInventoryTag`: optional automatic-discovery filters. Explicit actors/inventories bypass them and radius.
- `ExplicitContainers`: placed actor list.
- `ExcludedInventoryNames`: blacklist matching `ContainerTitle`, falling back to inventory ID. Exact, case-insensitive matching ignores surrounding whitespace and empty entries. It applies to explicit sources too; `DisplayName` overrides do not affect matching. After server runtime changes, call `RefreshContainers`.
- `MaxUseDistance`: player-to-point limit for opening and operations.
- `WindowClass`: `WBP_StorageAccess`, already assigned on the example Blueprint.
- `WindowTitle`: heading.
- `CanPlayerAccess`: overridable ownership/team/permission check; default permits players within the separate distance limit.

Run `AddContainer`, `RemoveContainer`, `AddInventory`, `RemoveInventory`, `ClearExclusions`, and `RefreshContainers` **on the server**. Removal also excludes sources from future automatic discovery. `ClearExclusions` clears runtime removals and refreshes but keeps `ExcludedInventoryNames`. `AddContainer` clears the actor exclusion, not individual exclusions on its inventories.

An `FStorageInventoryEntry` can override `DisplayName` and `SortOrder`. Without a name, it uses ContainerTitle then inventory ID.

Radius scanning occurs on open and Refresh. Spawning/moving sources needs Refresh or AddContainer. Changes to already connected inventory membership and source destruction use events; the catalog is not polled on Tick.

## Shared workspace widget

`UI/Inventory/Workspace/WBP_InventoryWorkspace` is also used by `WBP_CraftDashboard`. Storage inventories appear in a scrollable list on the left, player inventory on the right.

In `WBP_StorageAccess → Class Defaults → Storage Access → Workspace → Workspace Settings`:

- `Player Column Min Width`: default 430.
- `Fit Player Column To Content`: fit the full container, including operations and scrollbar space.
- `Allow Inventory Collapse`: collapse sources by clicking the arrow heading.
- `Hide Inventory Close Buttons`: temporarily hide nested close buttons, including the player's.

Search/Refresh/list rebuilding preserve existing collapse state. Collapse hides only a widget body; it does not alter contents or access. Leaving restores close-button settings; a new window starts expanded.

`UInventoryWorkspaceWidget.ConfigureWorkspace` accepts these settings. Its defaults leave collapse/close hiding disabled; `UStorageAccessWidget` enables them for shared storage.

Find storage by name matches part of the **inventory name**, case-insensitively, not item names. It hides panels without recreating inventories. Clear removes the filter. Crafting sections use Resources, Fuel, and Output.

Style the Blueprints and their Core instances. `Scripts/UI/create_storage_workspace.py` uses the crafting menu palette without changing global Core buttons.

## Access-point mesh

`BP_StorageAccessPoint` uses `SM_StorageAccessDesk`, a wooden clerk desk with sloped top, ledger, and keys. It is approximately 80 × 55 × 110 cm, 1720 triangles, with one convex collision for movement/interaction.

The mesh and five flat-color materials are under `/InventorySystemInvenzaPlugin/Meshes/StorageAccess`. Material constants control color, roughness, and metallic. Blender source, FBX, and generation script are in `ArtSource/StorageAccess/`.

Mesh scale is `(1, 1, 1)`. The pivot is 50 cm above the base, matching the former cube to retain placed-instance heights. The test-map instance inherits the new mesh/scale without changing the level file. For manual placement, the base sits 50 cm below pivot; surface snapping can help.

Import using `Scripts/Storage/import_storage_access_desk.py` through a normal editor's `-ExecutePythonScript`. It saves only new mesh/materials and `BP_StorageAccessPoint`. Trace/screenshot preview: `Scripts/Storage/preview_storage_access_desk.py`.

## Transfers and networking

Transfers between shown inventories/player, Take all, Place all, Place matching, and sorting route through the player's component. Session membership replicates owner-only on `UIInventoryManager`. Server operations recheck membership, permissions, and distance. Search does not change permissions; removal or close revokes access. Contents are not copied into a virtual inventory.

Remote explicit containers require actor/collection replication. While open, source actors temporarily become Always Relevant so remote clients receive real inventories/items. This expands replication for all connections, not just the viewer; account for bandwidth on large bases. Original relevance returns after the final session closes. Chest network ownership is not transferred. Dynamic explicit membership is not automatically persisted as save-game state.

Tests: `Invenza.Inventory.StorageAccess`, `Invenza.Inventory.StorageWorkspace`, `Invenza.Inventory.TransferMatchingItems`, and `Invenza.Inventory.StorageShutdown`. The last ends PIE with storage open. Normal closing returns player inventory to HUD; teardown releases references/subscriptions without redrawing.

Historical automation names in this guide refer to checks whose source files were removed at the owner's request. They are not available in the current distribution; use the described manual scenarios.
