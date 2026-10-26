# Configuring Invenza controls

Language: [Русский](../ru/InputConfiguration.md) | **English**

Detailed companion to section 8 of the [overview](GettingStartedOverview.md). First connect `InventoryCollectionComponent`, `UIInventoryManager`, and UI using the [integration guide](ProjectIntegration.md).

Bindings are based on current C++ code. Suggested keys below are examples, not verified Blueprint defaults; inspect Mapping Contexts and example assets in the editor.

## 8.1. Where controls are configured

| Location | Purpose |
|---|---|
| `UIInventoryManager → UISettings` | Window actions, transfer modifiers, rotation, mouse keys |
| Input Mapping Context | Physical key to Input Action mappings |
| `InteractionComponent → KeyBindings` | Action to Primary/Secondary interaction mapping |

An Input Action represents intent; a Mapping Context maps physical input and must be active in Enhanced Input Local Player Subsystem. See [Enhanced Input for UE 5.6](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine?application_version=5.6).

Global `InvenzaSettings` configures other system behavior; inventory control bindings live on the character's manager.

## 8.2. Enhanced Input setup

Enable Enhanced Input and select `EnhancedPlayerInput` / `EnhancedInputComponent` in Project Settings → Input.

Reuse plugin actions or create Boolean actions for buttons. Start without extra Hold, Tap, or Chorded triggers. When replacing an action, assign the same asset in the context and component settings.

The manager adds the inventory context at priority 1. Your gameplay logic adds movement and any separate interaction context. `InteractionComponent` does not add a Mapping Context itself.

Check conflicts with existing controls, context priorities, and Consume Input, then test both actions. Context priority alone does not guarantee the desired behavior throughout the input handler chain.

## 8.3. Inventory actions

Select the character's `UIInventoryManager` and expand `UISettings`:

| Field | Plugin asset | Purpose |
|---|---|---|
| `InventoryMappingContext` | `Input/InputMappingInventoryContext` | Inventory mappings |
| `ToggleInventoryAction` | `Input/UIActions/IA_ToggleInventory` | Open/close inventory UI |
| `ToggleCraftAction` | `Input/UIActions/IA_ToggleCraftMenu` | Switch character crafting state |
| `IA_Mod_QuickGrab` | `Input/UIActions/IA_QuickGrab` | Quick-transfer modifier |
| `IA_Mod_GrabAllSame` | `Input/UIActions/IA_GrabAllSame` | Same-type transfer modifier |
| `IA_RotateDraggedItem` | `Input/UIActions/IA_RotateItem` | Rotate while dragging |

Example context mappings:

| Action | Suggested key |
|---|---|
| Inventory | `I` |
| Character crafting | `C` |
| Quick transfer | Hold `Left Shift` |
| Same-type transfer | Hold `Left Ctrl` |
| Rotate dragged item | `R` |

Window toggles/rotation use `Started`. Modifiers turn on with `Started` and off with `Completed`. Begin with simple button actions. For complex triggers, check release and cancellation: modifier bindings currently do not handle `Canceled`.

`ToggleEquipmentAction` and `IA_ToggleEquipment` exist, but the manager's `InitializeBindings` does not automatically bind that field. Add a custom binding to open equipment UI if needed.

`ToggleCraftAction` needs the character's `CraftingComponent`. It does not switch character crafting while a station provider is active. Actual window display depends on connected UI logic.

## 8.4. Mouse buttons

Selection, use, and context menu use direct `FKey` values rather than Input Actions:

| Field | Purpose | Initial suggestion |
|---|---|---|
| `ItemSelectKey` | Select / begin drag | Left Mouse Button |
| `ItemUseKey` | Use item | Right Mouse Button |
| `ItemMenuKey` | Request context menu | Middle Mouse Button |

The constructor defaults to left/right for selection/use. `ItemMenuKey` has no explicit constructor default; assign it in Blueprint. Middle mouse is a separate test binding suggested here.

The grid checks use before menu. Sharing one key may prevent the menu from opening when use is allowed. List handling may execute both operations, so keep them distinct there too.

Use depends on inventory `bAllowItemUsage` and the item's `UseItem` implementation. A binding alone does not implement healing, weapons, or other gameplay behavior.

## 8.5. Quick transfer and rotation

Press `ItemSelectKey` while holding a modifier for quick transfer; without modifiers, it begins dragging.

Open a chest or other available destination, hold QuickGrab, and click an item. Repeat with GrabAllSame. Placement rules, accessible destinations, and available space determine results. If both modifiers are active, grid/list handlers check single-item quick transfer first; give them separate keys.

Rotation requires an active `ItemDragDropOperation`. Test a rectangular item, such as 2 × 1, in a grid that respects item size. Pressing the key outside a drag does not rotate stored items.

## 8.6. World interaction

Add these entries under `InteractionComponent → KeyBindings` if both actions are needed:

| Action | Type | Example key in context |
|---|---|---|
| `Input/Actions/IA_PrimaryIteract` | `Primary` | `E` |
| `Input/Actions/IA_SecondaryIteract` | `Secondary` | `F` |

`Iteract` matches the current asset spelling. Both actions need mappings in an active context. Add them to your gameplay context or inspect and activate `GameplayMappingContext`.

Interaction begins on `Started` and ends on `Completed`. Hold duration belongs to the interactable object's action data. Do not layer a Hold trigger on top for the initial setup.

Check camera, trace channel, distance, and object collision. The key selects Primary/Secondary; the target component determines pickup, opening, or station work.

## 8.7. Slot hotkeys

Grid slots can carry `UseAction`. The manager iterates available slots and binds each action to that slot's use request.

Assign actions to quick slots and map them in an active context. Plugin actions `IA_HotkeySelect_1`–`IA_HotkeySelect_8` can map to `1`–`8`.

For authored grids, ensure slot actions are collected into initialization data. Custom generators must populate `UseAction`. Slots added or changed after initial input binding require coordinated binding updates in your implementation.

## 8.8. Cursor, focus, and checks

`UIInputModeSubsystem` switches Game Only / Game and UI and cursor visibility based on UI-owner requests. Coordinate custom menus with this subsystem.

1. Start PIE and click the game viewport to focus it.
2. Open/close inventory; check cursor and character control restoration.
3. Drag, rotate, and place an item.
4. Test each transfer modifier and release it; normal dragging should work afterward.
5. Test separate use/menu mouse buttons.
6. Test Primary/Secondary interaction and configured holds.
7. Test every assigned quick-slot key.

When input fails, check the assigned action, active mapping, and Input Component. Inventory binds to PlayerController index 0; interaction binds to the locally controlled character. Multiple local players need separate adaptation.

For the dynamic F6 help window, see [controls help](ControlsHelp.md). Save/load commands and custom bindings are covered in [inventory saving](InventorySaving.md).
