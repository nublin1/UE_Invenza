# Portable backpack inventories

Language: [Русский](../ru/PortableBackpacks.md) | **English**

A backpack is an item with its own persistent container. Equipping it makes that container the character's main inventory. Replacing a backpack switches the active container: the previous contents do not move into the new bag and remain with the old bag when dropped.

Backpack definitions use the shared `Data/DT_ItemData`. Separate backpack Data Tables or Data Assets are not required.

## Character setup

1. Add `InventoryCollectionComponent`, `EquipmentComponent`, and `UIInventoryManager`. To expose an NPC's equipment to another player, also add `CharacterInventoryAccessComponent`; see [character inventory exchange](CharacterInventoryExchange.md).
2. Enable `Use Portable Backpacks` (`bUsePortableBackpacks`) in the manager. Its native default is false. When enabled, a startup `MainInventory` entry matching the global `MainInvTag` does not create a permanent bag: the equipped backpack supplies the main inventory.
3. Keep the startup equipment inventory with the global `EquipmentInvTag`. Define a backpack slot and its `AllowedCategory` in `EquipmentComponent`. The authored equipment cell must use the matching `LinkedEquipmentSlot` and category.
4. In `Project Settings → Invenza Inventory`, check `Portable Inventory Category` and the shared `CharacterBagTemplate`: inventory, container, grid, cell, and operations panel classes. Enable `bAllowItemReferencing` if hotbar or equipment references should point to contents. Layout belongs in Widget Blueprint.
5. Configure the item row and optional starting items below.

`PortableInventoryCategory` defaults to `Item.Category.Backpack`. Matching that tag **or a descendant** enables portable inventory behavior. Equipment compatibility is a separate check: the slot's `AllowedCategory` must **exactly match** the item's category. If you add a backpack subcategory, configure a compatible slot for it.

### Backpacks and permanent pockets

Without an equipped backpack, there is no active main inventory (`MainPawnInventoryRef`). Equipment, a reference hotbar, and additional permanent inventories can still exist. For pockets, add a separate `StartupInventories` entry with its own tag that does not meet the main-inventory skip condition.

An additional inventory does not automatically become the main inventory. General startup grants and bundles use `GetReceivingInventories`: main storage first, then owned ordinary storage with `bReceiveGrantedItems = true`, even without UI. Reference, equipment, vendor, external, and crafting service inventories are excluded. Disable the receiving flag for other internal service storage. Operations explicitly targeting `MainPawnInventoryRef`, including ordinary single-item pickup and bulk transfers from main storage, keep that destination.

## Item row

Set `ItemMetaData.ItemCategory = Item.Category.Backpack`, or your compatible category, and configure:

| Field | Purpose |
|---|---|
| `ItemMetaData.ItemInventoryData.Rows`, `Columns` | Initial internal rows and columns; each must be between 1 and 256 |
| `ItemInventoryData.MaxWeight` | Content weight limit; `-1` is unlimited |
| `ItemInventoryData.EquipAction` | Localized additional ground interaction text; empty uses the standard equip action |
| `ItemNumeraticData.Weight` | Empty backpack weight |
| `ItemNumeraticData.InventoryHorizontalSlots`, `InventoryVerticalSlots` | Item footprint in another inventory, independent of internal capacity |
| `ItemTextData.DisplayName` | Localized item name and internal container title |
| `ItemAssetData.Icon`, `Mesh`, `Materials`, `MeshScale` | Icon, item mesh, material overrides by slot index, and scale |

Shortened field paths in this table are also inside `ItemMetaData`. `MaxWeight` limits contents; the bag's own weight is added to its item weight. Backpacks always have quantity 1 and never stack.

The common table includes `BackpackSmall` (4×5, content weight limit 50) and `BackpackLarge` (6×7, limit 65). Their text uses `Data/Portable/ST_Backpacks`; mesh and icon are prototypes. `BP_InvManager.ExampleGroundBackpacks` supplies additional ground examples when portable mode is enabled. The list is empty in `BP_Character`.

For storage visualization, configure `ItemStorageData.StorageMesh`, its `Materials`, and `MeshScale` separately. Demonstration boxes share `SM_StorageLabelBoxMedium`: materials provide labels and scale provides dimensions. Empty overrides retain mesh materials.

## Equipment, transfers, and restrictions

- One equipped backpack is active. Primary ground interaction opens its contents; secondary interaction equips it.
- Replacement drops the previous backpack with its contents; items are not moved into the replacement. If the ground item cannot be prepared, replacement is cancelled without losing the old bag.
- Filled backpacks cannot enter ordinary inventories or another backpack. Empty bags can be carried, but their contents cannot be opened while stored inside another inventory. A bag cannot be placed into its own container.
- A filled backpack can be transferred directly into another character's compatible equipment slot through character exchange. Its identity and contents are preserved; this is distinct from putting it in a bag.
- Craft reservations and pending refunds block removal, transfer, replacement, and resizing. Recovery items also count when determining whether a backpack is empty.
- Active trading blocks removing, dropping, destroying, or replacing the equipped bag. A filled bag cannot be destroyed as an empty item.

Removing or replacing a bag detaches the previous main container. The manager cancels related dragging, removes its panel, and clears the player's references to its contents, including corresponding equipment. Physical contents remain in the removed bag. Inventory content and configuration events update the active UI.

## Starting items

Manager `Startup Inventories → Equipment → Start Items` defines startup equipment. `EquipmentInvTag` identifies the inventory; item category and `EquipmentComponent` compatibility identify the slot. Backpacks are processed before other equipment. The server reads authored Widget Blueprint slots without constructing UI. The demonstration `WBP_EqGrid` has a separate backpack cell below the other slots.

`General Starting Items` supplies ordinary contents to receiving storage after equipment is prepared. This list never automatically equips items. The example character's eight ordinary startup entries have been moved here. Do not duplicate the same grant across lists.

Without a bag, or when capacity is exceeded, remaining startup items are dropped. The manager's `Starting Items Drop Offset` specifies a character-local position. Failed drops retain their item instance and outstanding quantity for `SetupStartingResources` retries; inventory changes also trigger retries. Saving is blocked while startup delivery remains pending.

Restoring a saved owner cancels startup grants, including ground examples. Repeated initialization must not produce a second starting set.

## Content admission rules

The internal container supports common [storage admission rules](StorageAdmission.md): its filter takes priority over permissive cells, empty rules without exclusions allow everything, and existing disallowed contents can still be rearranged internally or removed. Rules are saved for that particular container.

**Current configuration limitation:** neither `FItemInventoryData` nor `CharacterBagTemplate` (`FInventoryTemplateSettings`) exposes `AdmissionRules` or `bAllowAdmissionRulesEditing`. A newly created bag uses empty rules with editing disabled. Setting those fields on an ordinary `StartupInventories` entry does not configure a dynamic backpack's filter.

For custom server integration, obtain the actual container through `UItemBase::EnsurePortableState()` and `APortableInventoryState::GetInventory()`. Set editing permission in its `FInventorySettings`, preserving other values, and apply rules through `UInventoryBase::ApplyAdmissionRules`. These are native C++ integration points, not ready-made item row settings. Do not overwrite restored user rules with startup defaults after loading.

The outer chest's filter determines whether it accepts **the empty backpack item**. The inner container's filter determines allowed contents. Allowing backpack items in a chest does not bypass the separate filled-bag restriction.

## Interaction with other containers

A container view shows the active backpack and additional ordinary player inventories such as pockets. Additional storage needs `bOpenWithExternalInventory = true` and widget classes. Reference containers, equipment, `VendorInventory` and `LootContainer` types, and linked external/crafting inventories are excluded.

`WBP_Dual.PlayerInventories` and `WBP_InventoryWorkspace.PlayerInventoryList` are list hosts authored in Widget Blueprint. Each inventory reuses one registered widget. Closing the interaction restores panel positions and prior open/closed states. A dynamic main container sizes its Canvas slot to content; optional configured placement is independent.

Permanent ordinary and vendor inventories are created independently of portable mode. Vendor stock, for example, does not require an equipped backpack.

## Persistence and resizing

A replicated `APortableInventoryState` retains contents under the item's persistent instance GUID. Its own `InventoryCollectionComponent` owns the contents, rather than the collection of the character currently wearing it. Transfers preserve the GUID. Integrations should use the internal inventory's `GetItemCollectionLinked()` and avoid creating a new container on every equip.

Saving restores backpack state, actual grid size, contents, admission rules, and valid external hotbar/equipment references. Core Redirects map old `Data/Portable/DT_Backpacks` save references to the common `DT_ItemData`.

Row `Rows`, `Columns`, and `MaxWeight` supply initial parameters when a container is created. Editing the table does not rebuild an existing container. To resize a grid during play, call `USlotbasedInventory::TryResizeInventory(FIntPoint(Rows, Columns))` on the server:

- Both dimensions must be between 1 and 256.
- The grid must not collect preauthored UI cells or contain linked equipment slots.
- Backpack craft obligations must be resolved.
- All current items must fit; otherwise the method returns `false` without changing contents or placements.

Do not replace this operation with a direct size assignment or grid reinitialization. Successful resizing and sorting preserve item instances and identities. Replacing a bag remains a container switch rather than a resize operation.

## Checking your configuration

1. Start without a backpack: main storage should be absent while configured permanent pockets remain. Equip a bag and check dimensions, title, and weight.
2. Add contents and a hotbar reference, then replace the bag. The old bag should be on the ground with its contents, the new bag should show its own contents, and the old player reference should be cleared.
3. Check carrying an empty bag into a chest, rejecting a filled bag, and directly equipping a filled bag on an NPC.
4. Check startup grants, overflow, missing drop surfaces, and repeated initialization. Loading a save must not grant the starting set again.
5. Enlarge the grid, then attempt to shrink it below required capacity. Failure should leave placements unchanged.
6. Save/load equipped and ground backpacks; check contents, sizes, rules, and valid references. In networked projects, separately check client equip, replacement, and opening.

Development checks are in the Example Project's `Source/InvenzaUIEditor/Private/Tests/PortableBackpackTests.cpp`, `ItemRowConfigurationTests.cpp`, and `InventoryStartingItemsTests.cpp`. Automation names are `Invenza.Inventory.PortableBackpacks`, `Invenza.Inventory.ItemRowConfiguration`, and `Invenza.Inventory.StartingItems`. They are Example Project development tools, not a plugin runtime dependency. The scenarios above are checks for your configuration, not a claim that they have been executed in your project.

## State lifecycle and replication

`UPortableInventorySubsystem` indexes GUIDs to weak state references and inventories to crafting obligations. Normal transfers preserve the state. Permanent deletion of an empty, unreserved bag destroys its state and removes its retained save owner; ordinary mapping removal never deletes it.

Contents replicate only to the carrier and players with active authorized access, including NPC equipment and ground containers. Changing carrier or closing access changes that audience. Hotbar aliases do not grant contents access. Publicly equipped item data retains normal equipment replication. Item replication carries a separate contents-weight/has-contents summary, so other viewers do not need private item lists to display the bag. Loading an existing ground pickup restores its saved transform and clears simulated linear/angular velocities, just as recreation restores its position.

`FInventoryGrantService` reports added, dropped, discarded, and remaining quantities. Bundle overflow drops or explicitly discards on drop failure; startup overflow retains failed drops for retry. Ordinary single-item pickup requires full placement in main storage.
