# Invenza — overview and setup

Language: [Русский](../ru/GettingStartedOverview.md) | **English**

This document introduces Invenza and the main stages of integration: initial setup, world interaction, equipment, trading, crafting, visual storage, saving/loading, and multiplayer. Numbering follows the documentation plan. Section 14 now covers persistence; section 16 is not included. Detailed guides are linked from the relevant sections.

## 1. Overview and architecture

Invenza is a modular inventory system for Unreal Engine. It combines item storage, placement rules, and user interface. Additional components support world interaction, equipment, trading, and crafting.

Data Tables define item types. At runtime, `InventoryCollectionComponent` tracks items and their locations, while inventory objects define storage space and limits. `UIInventoryManager` connects inventories to UI and handles player operations.

The UI consists of item widgets, inventory widgets, and container windows arranged in layers. This separation allows changes to appearance and inventory configuration while retaining the shared item logic.

## 2. Installation and integration

The plugin is in `Plugins/InventorySystemInvenzaPlugin`. It includes a C++ module, Blueprint examples, widgets, Data Tables, and input assets. The demonstration project uses Unreal Engine 5.6.

Integration requires installing and enabling the plugin, building its C++ module, and configuring data and UI. The system uses Enhanced Input, Gameplay Tags, and UMG.

The plugin supplies its settings defaults and Gameplay Tags. Project-specific overrides belong in Config/DefaultInventorySystemInvenzaPlugin.ini. The standalone interaction channel defaults to Visibility; the separate Example Project uses its own Interaction channel. See [configuration and migration](IntegrationConfiguration.md).

See [integration with another project](ProjectIntegration.md) for the full procedure.

## 3. Quick start

Start with one character inventory and a few test items. You need global settings, an item table, character components, and basic UI.

Assign the settings asset, define items and tags, add character components, configure the startup inventory, connect UI, and assign input actions.

The initial result should be a character with starting items who can open the inventory and move items in its UI. The plugin's example Blueprints and widgets provide starting configurations.

## 4. Global settings and Gameplay Tags

Global configuration is a Data Asset of class `InvenzaInventorySettingsAsset`, assigned under Project Settings → Invenza Inventory. The demo uses `InvenzaSettings` in the plugin's `Data/Settings` folder.

It defines shared inventory/equipment tags, the dropped pickup class, currency, filter appearance, and modal dialogs. Individual inventory settings and input actions are configured separately.

Gameplay Tags identify inventory roles, item categories, equipment slots, and UI elements. Matching tags connect inventories to their widgets. Consistency between data and UI is essential.

## 5. Creating items

Items are Data Table rows using `FItemData`. The example `DT_ItemData` can be used as a starting point.

An item definition includes ID, name, description, icon, and mesh. Properties specify weight, grid dimensions, maximum stack size, and category. Trade settings and drop/delete permissions are also available.

The table describes an item type; runtime instances separately track quantity and placement. Starting contents reference table rows and quantities.

## 6. Character components

`InventoryCollectionComponent` tracks items and connected inventories. `UIInventoryManager` creates startup inventories, connects UI, and processes item operations.

Add `InteractionComponent` for world interaction. Equipment, crafting, and persistence have additional components or subsystems as needed.

At startup, the manager links components, creates inventories, and prepares UI for the locally controlled character. Coordinate character, input, and UI initialization. An initialization-complete event is available for dependent logic.

## 7. Inventory configuration

A character can have multiple inventories with different purposes and appearances. `UIInventoryManager.StartupInventories` provides settings and starting items for each.

The plugin supports slot/grid and list inventories. A grid can account for item size and orientation; a list shows separate entries. Settings control weight and stack-count limits, title, visibility, and widget classes.

Inventory structure can come from an authored widget or be generated from settings. The choice determines where slot placement and grid dimensions are defined. Gameplay Tags identify roles; IDs identify individual inventories.

## 8. UI and controls

The primary layout contains layers for gameplay, windows, and modal dialogs. These host inventory containers, item widgets, operation panels, and tooltips.

Reuse existing widgets as a styling base. Connect UI to the character's manager and assign inventory tags. Custom widgets must preserve child names and types expected by their base classes.

Controls use Enhanced Input and the manager's `UISettings`: window toggles, quick-transfer modifiers, dragged-item rotation, and mouse keys for selection, use, and context menus.

Available UI operations include dragging, filtering, sorting, weight display, and tooltips. Actual availability depends on item properties, inventory settings, and connected components. See [input configuration](InputConfiguration.md).

## 9. World items and containers

`InteractionComponent` finds world objects and handles player input. Object components define the available actions.

`Pickup` and `PickupComponent` handle dropped items. `ContainerComponent` with `InventoryCollectionComponent` handles storage. Configure startup inventories, contents, and the main loot inventory tag; opening a container connects its contents to the player's UI.

Interaction setup includes trace channel, range, collision responses, input, and action conditions. The demo uses the `Interaction` channel. Containers can be destroyed when empty; actions can require holding a button.

See [world items and containers](WorldContainers.md).

### 9.1. Shared storage access

`StorageAccessComponent` displays several real inventories in one window. Sources are selected by radius, tags, or explicit lists. Items stay with their source owners until transferred.

Discovery filters select sources; `CanPlayerAccess` controls player permissions. The server checks access and distance during operations. Concurrent sessions share the same real storage; remote sources require suitable replication. See [storage access points](StorageAccessPoint.md).

## 10. Equipment and quick slots

`EquipmentComponent` uses Data Table rows of `FEquipmentSlotDefinition`; the example is `DT_Equipment`. Each slot has a tag and an allowed category.

Equipment UI links visual cells to logical slots. Categories define compatibility. Equip/unequip events can drive stats, weapon visuals, or character behavior.

Quick-access panels use slots with use actions. Reference settings let such panels refer to items in another inventory. Inventory, slot, and input configuration determine behavior.

### 10.1. Mannequin and character exchange

`InventoryMannequin` is a stationary owner of a bag and equipment. `CharacterInventoryAccessComponent` opens an exchange view for transferring, equipping, and unequipping items. A custom NPC can use the same collection, equipment, and access components.

Access lasts for the interaction and permits one active nearby accessor. Transfers validate slots, categories, and placement across owners. The example's mesh lives outside plugin content and must be migrated or replaced. See [character inventory exchange](CharacterInventoryExchange.md).

## 11. Trading

`VendorComponent` connects vendor interaction, inventory, and purchase/sale processing. It finds the vendor inventory in its owner's collection by Gameplay Tag.

Global settings define currency by item row and category. Items specify base price and whether they can be sold. Vendor settings control buy/sell factors, trade mode, and assortment changes.

Transactions check funds and placement before moving goods and currency. Results indicate success or refusal; transaction events support additional game logic.

## 12. Crafting and production stations

`CraftingComponent` uses recipe rows of `FItemRecipeRow`, with examples in `DT_Recipe`. Recipes define ingredients, alternatives, outputs, work volume, and operator requirements.

The component manages recipes and queues. Settings control speed, automatic production, and resource consumption on queue addition, start, or finish. Input, output, and fuel inventories are linked by tags and may coincide when appropriate.

`CraftingStationComponent` opens station UI and handles work interaction. A station may use its own crafting system or the interacting character's. Fuel consumption and missing resources, fuel, or operators affect blocking state. Widgets display recipes, queues, and progress.

## 13. Visual storage

`InventoryVisualizer` displays stored items in the world, either individually at sockets or through occupancy-dependent mesh swaps.

`StorageGridVisualizer` links a tagged grid inventory to world cells. Settings control geometry, spacing, direction, interaction bounds, and editor preview.

Item storage data selects meshes and modes: single meshes, logs, small boxes, or [bulk bins](BulkBins.md). Display updates follow inventory changes. See also [shelf fill examples](ExampleShelfFill.md) and [restocking](RestockStorage.md).

## 14. Saving and loading

`InvenzaSaveSubsystem` captures registered collections belonging to players, NPCs, vendors, and storage. Snapshots include item instances and placements, equipment/hotbar references, world pickups, and crafting queues. Temporary windows and permissions must be opened again after loading.

Profile, auto-load, auto-save, and graceful-exit saving are configured under Project Settings → Invenza Save System. The host manages world snapshots in network games. Persistent owner/inventory/item identities match records during restoration; items that cannot currently be placed remain in recovery.

See [inventory saving](InventorySaving.md) for setup, API, constraints, and file behavior; see [frontend menus](FrontendMenu.md) for save/load pages.

## 15. Multiplayer

Storage access uses temporary sessions, while mannequin exchange grants access to two registered NPC inventories. These mechanisms do not require transferring network ownership of chests to a player. Shared storage temporarily expands source relevance; see [storage access points](StorageAccessPoint.md).

The code includes server operations and replication for inventories, items, equipment, and crafting. Startup inventories and resources are created on the server; mutation requests use server RPCs.

UI is created for locally controlled characters and displays replicated state. Coordinate actor/component replication, ownership, and initialization. Player mutations should use the system's requests so the server processes and replicates them.

Validate listen-server and dedicated-server configurations separately. Test pickup, transfers, shared storage, equipment, trading, and crafting. Network code alone does not establish that every scenario has been verified in the current configuration.
