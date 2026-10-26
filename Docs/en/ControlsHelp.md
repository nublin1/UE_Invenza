# Controls help

Language: [Русский](../ru/ControlsHelp.md) | **English**

Press **F6** in the example to open `WBP_ControlsHelp` on the modal layer. It contains a scrollable action/key list and an **OK** button. Escape and Enter also close it. The previous modal is retained and shown again after help closes.

The Blueprint is at `UI/Controls/WBP_ControlsHelp` in the plugin and uses existing Core text, frame, and button widgets.

Set dynamic row font, size, and color in **Class Defaults → Controls Help → Appearance**: `Row Style` controls actions and keys; `Heading Style` controls group headings. Each style's `Text` is replaced when rows are built. The default font is Unreal's Roboto.

## Setup

Open `/InventorySystemInvenzaPlugin/Data/Settings/InvenzaSettings`, **Controls Help**:

- `Controls Help Widget Class`: window class.
- `Controls Help Action`: open action; the example uses `IA_ControlsHelp`, mapped to F6 in `Input/InputMappingInventoryContext`.
- `Include Inventory Controls`: collect actions from the current character's `UIInventoryManager.UISettings`, `InteractionComponent.KeyBindings`, and inventory quick slots.
- `Additional Controls`: extra rows with a group, display name, and Input Action. The example includes movement, look, jump, Take all, and Place all. For a different character, select its actual actions.

Rows do not store physical keys. Opening the window queries the local player's current Enhanced Input mappings. Multiple bindings use `/`; chords use `+`. Unmapped actions show `Unassigned`. Inventory mouse buttons come from `UISettings` because they use `FKey` rather than Input Actions.

For a custom button, call `UIInventoryManager.ShowControlsHelp` or the local player's `ModalWindowManager.ShowControlsHelp`. If using your own Mapping Context, add the `Controls Help Action`; the manager does not force a physical key.

Test: `Invenza.UI.ControlsHelp` covers active mappings, remapping, keyboard/gamepad, chords, unassigned actions, the modal stack, and OK.

Historical automation names in this guide refer to checks whose source files were removed at the owner's request. They are not available in the current distribution; use the described manual scenarios.
