# World items and containers

Language: [Русский](../ru/WorldContainers.md) | **English**

Practical companion to section 9 of the [overview](GettingStartedOverview.md). The character should already have working inventory, UI, and [controls](InputConfiguration.md). Settings are checked against source; inspect example Blueprint values in the editor.

## Choosing an object

| Goal | Component or example |
|---|---|
| Pick up an item | `PickupComponent`, `BP_ExamplePickup` |
| Open a chest | `ContainerComponent` and `InventoryCollectionComponent` |
| Open several storages together | [StorageAccessPoint](StorageAccessPoint.md) |
| Exchange items and equip an NPC | [CharacterInventoryAccessComponent / mannequin](CharacterInventoryExchange.md) |

## Pickups

Start with `Blueprints/Examples/BP_ExamplePickup`. Configure `PickupComponent.InitialItem`: select the item table, Row Name, and positive `Amount`. The row must use `FItemData`.

Check mesh and interaction collision. Place it in front of the character, aim the camera, and use Primary interaction. Placement respects item dimensions, capacity, and stack rules. Also test insufficient space.

For a custom actor, add `PickupComponent` and a Static Mesh with suitable collision. Gameplay creation can call `InitializeDrop(FInitItemsEntry)`. To drop from UI, also assign `PickupClass` in global settings and enable the item's `bIsDroppable`.

## Ordinary containers

Create an Actor Blueprint or use `Blueprints/Examples/Containers`. Add `InventoryCollectionComponent`, `ContainerComponent`, and a mesh with collision.

Configure `ContainerComponent`:

1. `StartupInventories`: begin with one grid inventory.
2. Match its `InventoryTag` to `MainLootContainerInvTag`.
3. Set `ContainerTitle` for UI and shared storage.
4. Set `InventoryClass`, grid dimensions, slot class, and widget classes. Use `bCollectInvDataFromWidget = false` to create structure without existing UI.
5. Set `StartItems` rows and quantities.
6. Configure the Primary action for opening; check action type and prompt text for custom setups.

The collection tracks contents; the container component creates inventories and opens access. The player's manager handles display and transfers. A chest does not need its own player `UIInventoryManager`.

`bDestroyWhenEmpty` destroys the actor when its main loot inventory empties. Disable it for permanent storage. With multiple inventories, remember that this check uses the main one.

## Detection, holding, and range

The character's interaction component sets trace channel and detection distance. Target collision must Block that channel. Input chooses Primary/Secondary; object data defines the operation and hold requirement.

Detection range and active interaction distance are separate. Global `MaxInteractionDistance` is in the settings asset; storage access also checks `MaxUseDistance`. See [interaction distance](InteractionDistance.md).

## Shared storage integration

Automatic discovery requires `ContainerComponent` and registered inventories in `InventoryCollectionComponent`. Access-point radius/filters apply; explicit actor lists provide exact membership.

Refresh the catalog on the server after dynamically spawning storage. Changes to contents of an already connected inventory do not require actor rediscovery. See [shared storage](StorageAccessPoint.md).

## Checks

Test opening, transfers in both directions, full inventories, closing/reopening, and large-item placement at grid boundaries. For disposable containers, test emptying and destruction.

In network projects, enable actor/component replication and test from a client. These are suggested checks for your configuration; PIE/network tests were not run while writing this guide.
