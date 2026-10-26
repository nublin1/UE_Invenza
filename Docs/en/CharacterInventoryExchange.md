# Another character's inventory and equipment

Language: [Русский](../ru/CharacterInventoryExchange.md) | **English**

## Purpose

The mannequin demonstrates access to another owner's bag and equipment. Players can transfer items and change NPC equipment in a shared exchange view. `InventoryMannequin` is a stationary Actor without AI; the same components can be used on a moving companion.

Unlike a [storage access point](StorageAccessPoint.md), this opens two chosen inventories of one NPC rather than a catalog of storages.

## Adding a mannequin

1. Create a Blueprint based on C++ `InventoryMannequin` and place it. Collision, skeletal mesh, `InventoryCollection`, `Equipment`, and `InventoryAccess` are already created.
2. Assign a Skeletal Mesh and check its offset. The constructor uses `/Game/Characters/Mannequin_UE4/Meshes/SK_Mannequin`, outside plugin Content. Migrate its dependencies or replace it in your project.
3. Check `Equipment` slot definitions from `DT_Equipment`; categories/tags must match items and UI.
4. Set `InventoryAccess.CharacterDisplayName`. Keep `bCreateDefaultInventories = true` and `StartupInventories` empty for demo generation.
5. Populate `BagStartingItems` with table, row, and positive quantity.
6. Check global `MainInvTag`, `EquipmentInvTag`, and project tags.
7. Ensure collision Blocks the player's trace channel; Play and use normal interaction.

The demo creates a 4 × 5 bag and equipment inventory using plugin tables/widgets at their existing paths. Keep those assets for first setup, then replace configuration as needed.

## Your own NPC

Add `InventoryCollectionComponent`, `EquipmentComponent`, and `CharacterInventoryAccessComponent`. The access component derives from `ContainerComponent` and creates startup inventories. Do not add a second independent container component to create the same bag/equipment.

Disable `bCreateDefaultInventories` for custom configuration and populate access `StartupInventories`:

| Setting | Bag | Equipment |
|---|---|---|
| `InventoryTag` | Match `MainLootContainerInvTag` and global `MainInvTag` | Match `EquipmentInventoryTag` and global `EquipmentInvTag` |
| Initial contents | Entry `StartItems` | Configure with equipment logic |
| Slots | Normal grid | `LinkedEquipmentSlot` and `AllowedCategory` |
| Reference scheme | `bAllowItemReferencing = true` | `bIsReferenceContainer = true` |

The reference scheme keeps items in the bag with equipment references. Configure physical equipment storage separately if chosen. Both inventories must be registered in the NPC's collection with that NPC as owner.

Nonempty `StartupInventories` suppress demo generation even if its flag remains enabled. Custom bags use `StartItems`, not `BagStartingItems`.

World inventories that must exist before UI opens need server-created structure. Merely assigning a widget template does not populate logical equipment links in arbitrary configurations. The demo separately extracts them from `WBP_EqGrid` or equipment definitions.

## Display and access limits

The player's `UIInventoryManager` handles both owners. An NPC does not need a second local player UI just to expose its bag. AI, behavior, and companion permissions belong to project logic.

`TryBeginAccess` allows one active accessor: another is refused while the current accessor remains within range. Shared storage supports concurrent sessions instead. Closing/ending interaction revokes temporary access.

The example has no friendship, faction, or consent rules. Add such conditions to your access-opening logic; a Companion label grants no permissions.

Socket names alone do not create weapon/armor visuals. Connect the owner's equipment events to visual logic. The portrait captures existing visible meshes/attached actors; first verify the NPC's world equipment visuals.

## Suggested checks

Transfer from player bag to NPC, equip to a compatible slot, and unequip. Test wrong categories, full bags, replacing gear, and transferring equipped items across owners. Quantities must remain consistent; refusal must not remove items.

Close/reopen, move out of range, and check player panels return to HUD. In networking, test a second accessor. Existing automation below was not rerun for this documentation update.

## Existing example details

`Map_Ex` contains an `InventoryMannequin` named `Companion` near storage examples. Normal interaction shows NPC equipment/bag on the left and player's on the right. Each side arranges equipment/bag horizontally and scales them with controls if needed. Drag to transfer. Close, Escape, and global range cutoff close the view.

Demo slot placement comes from `WBP_EqGrid` and container `WBP_EquipmentInv`, as for the player. Custom equipment slots require valid `LinkedEquipmentSlot` / `AllowedCategory`; global equipment/main tags select the corresponding inventories.

`TryResolveEquipmentContext` resolves owners through registered inventory/collection identity, not `Actor::GetOwner()`. Access is limited to the selected NPC's bag/equipment while interaction lasts.

Equipment transfers first plan final placements without mutations, validating slot, category, dimensions, weight, stack count, displaced-item space, and reference items' primary storage. Refusal leaves placement/equipment unchanged. Success preserves actual item instances, updates mappings/equipment for both owners, and does not recreate quantity from Data Table rows. Transferring equipped items removes the former owner's references.

Context Equip/Unequip also respects the selected inventory owner. Equipment changes run server-side and through client `OnRep_EquipmentSlots`. Window updates use events rather than inventory Tick polling. Closing returns player panels; world shutdown does not recreate UI.

`UCharacterInventoryExchangeWidget` is built in C++ from Core Label/Button. Its background/text colors are currently in `.cpp`; existing Blueprint styling is retained. Panels scroll vertically and shrink horizontally if needed. Nested close buttons are temporarily hidden. Returning player panels restores Canvas parent, anchors, offsets, alignment, size, AutoSize, ZOrder, and visibility.

`UEquipmentInventoryWidget.SetPreviewActor` creates a separate temporary scene, SceneCapture, and RenderTarget. It copies visible meshes, materials, current pose, and attached actors. Independent lighting does not affect the world. Player portrait texture is untouched. The scene lasts only while viewed; it adds no separate inventory polling. Authored equipment cells are preserved from the template.

Large-item drag previews appear only when the full footprint fits real cells. Crossing bottom/right bounds adds no artificial scroll area. Cursor hit testing uses Slate geometry with scaling/scrolling.

The base window is 1280 × 720 with a 12-unit screen margin, scaling as a whole on smaller screens. Equipment and bag stay side by side. Styled cell sizes come from the template and a measurement after Slate layout. Hit testing uses real cell geometry, gaps, and scale rather than a hardcoded 64-cell size. Server slot constraints remain intact.

Test: `Invenza.Inventory.CharacterExchange` covers physical/reference slots, cross-owner movement, wrong slots, no room, replacement, unequipping, access revocation, and PIE shutdown with the view open. It also drags RedPotion, YellowPotion, Bow, and Armor_1 to compatible equipment slots using Slate, checks highlight placement/size, unequips, and verifies stack quantity.

Historical automation names in this guide refer to checks whose source files were removed at the owner's request. They are not available in the current distribution; use the described manual scenarios.
