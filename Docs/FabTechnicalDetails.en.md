# Invenza — Fab technical details

Counts refer to the UE 5.6 source submission containing Map_Ex, prepared on 26 October 2026.

## Features

- Grid and list inventories with stacking, item sizes and rotation
- Equipment slots and inventory-linked hotbars
- Drag-and-drop, movable inventory windows and quick item transfers
- Category filters, text search, sorting and weight limits
- World pickups, dropped items, loot containers and interaction components
- Vendor inventories, item-based currency and configurable buy/sell prices
- Crafting recipes with alternative ingredients and configurable quantities
- Production queues, crafting stations, fuel and progress tracking
- Linked shared and personal storage with group-wide exclusive access for shared inventories
- Visual storage for shelves, grids, log stacks, boxes and bulk bins
- Inventory persistence, manual save slots, autosaving and save profiles
- Server-validated inventory operations and replicated inventory, equipment, crafting and storage state
- Configuration through Data Tables, Data Assets and Gameplay Tags
- Enhanced Input integration and customizable UMG widgets
- Playable Map_Ex with its character, controller, HUD and required example assets

## Technical specification

- Number of Blueprints: **122** — 19 regular Blueprints, 98 Widget Blueprints, 3 Animation Blueprints and 2 Control Rig Blueprints.
- Number of C++ Classes: **123 reflected UCLASS classes**, plus **15 interfaces**. The project-only editor utilities are excluded from these counts.
- Module: **InventorySystemInvenzaPlugin — Runtime**.
- Input: **Keyboard and mouse**.
- Network Replicated: **Yes**.
- Supported Engine Version: **Unreal Engine 5.6**; validated with **5.6.1**.
- Supported Development Platforms: **Windows (Win64)**.
- Supported Target Build Platforms: **Windows (Win64)**.
- Mac and Linux support: **not declared or validated in the current distribution**.
- Required Unreal plugins: **Enhanced Input** and **Control Rig**. Control Rig is used by the included example character animations.
- Documentation: included in **English and Russian** under `Documentation/en` and `Documentation/ru`.
- Source and example project: https://github.com/nublin1/UE_Invenza
- Support: reddrragon1151@gmail.com

## Additional information — copy-ready text

Invenza includes its C++ source, Blueprint examples, customizable UMG widgets, and documentation in English and Russian. Items, recipes, equipment slots, and storage definitions can be configured through Data Tables and Data Assets.

The included Map_Ex demonstrates inventories, equipment, crafting, trading, world containers, character inventory exchange, and linked shared/personal storage. The example character, controller, HUD, meshes, animations, and required resources are included in the plugin.

The current release supports Unreal Engine 5.6 on Windows (Win64). Enhanced Input is required; Control Rig is required by the included example animations. Both are Epic Unreal Engine plugins.

Multiplayer integrations use server-validated item operations and replication. Linked shared storage permits one active user per storage group. Personal storage in networked projects requires a stable player identity supplied by the project's authenticated account flow. Storage persistence is scoped to the current world and save profile.

The Fab source submission excludes Binaries, Intermediate, Saved, logs, screenshots, and local build output. A compatible Unreal Engine C++ toolchain is needed when compiling the source package locally.

Support: reddrragon1151@gmail.com
