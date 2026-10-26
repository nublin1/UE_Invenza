# Restocking storage

Language: [Русский](../ru/RestockStorage.md) | **English**

The `+=` button in `WBP_Inventory_OperationsPanel`, next to Place all, transfers stacks from the player's main inventory whose types are already present in the target storage.

Types are captured at operation start and compared by `ItemID`, as in `bIsSameItems`. An empty storage accepts nothing through this button. Matching items may merge with stacks or occupy new cells. Transfer uses `ItemTransferRequest`; quantities that do not fit remain at the source.

Blueprint API: `UIInventoryManager.TransferMatchingItems(SourceInventory, TargetInventory)`. The source may be another accessible inventory. Both inventories must pass the manager's access checks, including current external/storage access. Client requests are checked again on the server. Trading and reference containers are excluded.

To add the action to a custom operations panel, use a Core button named `Button_PlaceMatching`; `UInventoryContainerWidget` binds it automatically. The current tooltip is in `WBP_Inventory_OperationsPanel`. `PlaceMatchingLabel` uses `+=`; its font and color are controlled by the operations panel's `Restock Label Style`, defaulting to Roboto.

Test: `Invenza.Inventory.TransferMatchingItems`.

Historical automation names in this guide refer to checks whose source files were removed at the owner's request. They are not available in the current distribution; use the described manual scenarios.
