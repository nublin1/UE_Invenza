# Primary Layout and layer diagnostics

Language: [Русский](../ru/UILayout.md) | **English**

`WBP_PrimaryLayout` hosts four layers: `UI.Layer.Game` for the gameplay HUD, `UI.Layer.GameMenu` for inventories and crafting, `UI.Layer.Menu` for the general menu, and `UI.Layer.Modal` for modal windows. Their visual order belongs to the Widget Blueprint Overlay. Class Defaults → `LayerDefinitions` registers each tag and its existing layer widget name. The debugger reads this registry rather than maintaining four separate layer references.

## Screens and nested windows

A layer maintains a stack of `UInvenzaBaseWidget` screens. GameMenu normally hosts one `WBP_GameMenuInvenza` screen, containing several inventories, crafting, and interaction windows. These nested windows can coexist and do not need separate top-level stack entries.

`InitialScreens` supplies startup screen classes. They are pushed once during the layout's attachment to the UI. `InitialCollapsedScreens` retains the existing initial display policy.

## Operations

- `PushContentToLayer(Tag, Class)` creates a screen in a registered layer.
- `UUILayer::PushWidget(Instance)` raises an existing instance without duplicating its entry. Different instances of the same class are allowed. One instance cannot belong to two layers or use another player's controller.
- `PopContentFromLayer(Tag)` / `UUILayer::PopContent()` removes the top screen and restores the previous one. Repeated calls on an empty stack are safe.
- `RemoveContent(Instance)` removes that exact screen, including an entry in the middle of the stack.
- `ClearLayer(Tag)` clears one layer; `ClearAllLayersExcept(Tag)` preserves that layer and clears the others; `ClearAllLayers()` clears every registered layer.

Unknown tags log a warning. An unknown exception makes `ClearAllLayersExcept` clear nothing. The last Pop and Clear reset the top pointer and collapse the layer when `bCollapseWhenStackIsEmpty` is enabled. That existing flag can remain disabled for HUD layers.

Removal and clearing share deactivation, parent removal, UI input request cleanup, and the `OnRemovedFromLayer` notification. Custom logic can use `OnLayerScreenActivated`, `OnLayerScreenDeactivated`, and `OnRemovedFromLayer`. UI input requests owned by a covered screen or its nested widgets are suspended and restored when the screen returns. Removed screens must not leave active input requests.

Close screens through their layer API rather than a separate `RemoveFromParent`, which would leave stack entries behind. The in-game general menu opens and closes through the registered MenuLayer. The **M** shortcut and separate startup menu map are retained; see [the general menu](FrontendMenu.md).

## Debugger

The **U** shortcut toggles `WBP_LayersDebugger`. It retains the original four columns: **Game**, **Game Menu**, **Menu**, and **Modal**. Each lists only its stack's screen names, starting with the top entry. The active visible screen is gold; other names are gray. Empty layers leave their columns empty.

The debugger reads the actual Primary Layout and refreshes on stack and visibility events. It does not change inspected windows or their mouse handling, and does not poll them every Tick. Nested inventory elements are not expanded into technical detail rows.

Layout and headers remain in `WBP_LayersDebugger`. Class Defaults → `EntryStyle` controls regular text and font; `ActiveColor` controls the active name. Entries reuse the existing Core `WBP_LabelBaseText_Wrapped` widget.
