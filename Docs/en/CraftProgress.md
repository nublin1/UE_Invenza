# Crafting progress

Language: [Русский](../ru/CraftProgress.md) | **English**

The queue and world station panel use the shared Core widget `/InventorySystemInvenzaPlugin/UI/Core/Progress/WBP_CraftProgressDisplay`. Both show a bar and percentage by default. Progress refers to the current unit being crafted; Remaining shows the outstanding quantity.

## Display settings

- Queue: `WBP_QueueCraftList_Dashboard → Class Defaults → Queue → Progress → Progress Settings`.
- World panel: `WBP_WorldCraftState → Class Defaults → World Craft → Progress → Progress Settings`.

| Use Progress Bar | Show Percentage | Result |
|---|---|---|
| On | On | Bar and percentage |
| On | Off | Bar only |
| Off | On | Percentage only |
| Off | Off | Completed / required work, such as 2 / 5 |

Queue and world settings are independent. Change them at runtime with `SetProgressDisplaySettings`; the current value is retained. Updates use existing queue/station events, without Tick. A station binds its world widget after initialization for standalone/host and when the reference replicates on clients. Clearing the reference unsubscribes and hides the widget.

## Appearance

- Fill/track colors, bar height, font, and text color: `WBP_CraftProgressDisplay → Class Defaults → Craft Progress → Appearance`.
- For local overrides, select `CraftProgress` in the queue row or `WBP_WorldCraftEntry` Designer.
- World background, border, and padding: `WBP_WorldCraftState → Class Defaults → World Craft → Appearance`.
- `WBP_WorldCraftEntry` is a world-specific row with a larger title. The shared `WBP_QueueCraftList_ItemBasic` was not changed for it.

The world panel is 250 Slate units wide. In `WBP_WorldCraftState`, `SizeBox_28` is centered in `WorldCraftLayout`, occupying half of a default 500-wide WidgetComponent surface without shrinking font or height. Change `SizeBox_28 → Width Override` in Designer to adjust width.

An empty station hides the whole panel, including its background. Advancing to the next unit shows that unit's progress. Values are clamped to 0–100%; zero required work shows 0% without division by zero.

## Checks

Automation: `Invenza.UI.CraftProgress`; real station PIE integration: `Invenza.UI.WorldCraftStation`.

For offscreen previews, run `Scripts/UI/preview_craft_progress.py` through a normal editor's `-ExecutePythonScript` with `-RenderOffscreen`. Four-mode screenshots go to `Saved/CraftProgress/`; assets are not saved.

`Scripts/UI/update_craft_progress.py` creates the Core component and local world row, updating only the queue row and world widget. Existing files are backed up to `Saved/CraftProgress/Before/` before saving.

Historical automation names in this guide refer to checks whose source files were removed at the owner's request. They are not available in the current distribution; use the described manual scenarios.
