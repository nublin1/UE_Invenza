# Withdraw a selected amount from storage

An accessible physical storage item has an additional **Take amount...** text action in its existing context menu. Other applicable item actions remain available. Its amount window shows **Take: N** and **Remaining: M**. Confirming transfers exactly `N` units into the interacting character's main physical inventory.

## Behavior

- The range is one through the unreserved quantity, including the whole stack. `Remaining` includes any reserved units still in storage.
- No temporary source-side split slot is needed. Part of a stack can be withdrawn from a completely full container.
- The server resolves the recipient from the requesting `UIInventoryManager`. It ignores a caller-supplied target inventory.
- The complete placement is planned before committing: compatible stacks, new stacks, occupied/reserved slots, stack limits and weight. The selected amount is transferred in full, or neither inventory changes.
- Insufficient capacity keeps the window open so the player can select a smaller amount. A changed stack or reservation requires confirmation against the refreshed range.
- Partial withdrawals retain the source object and create a separate receiving identity. Full withdrawals remove the original source references. Copies preserve the item class, metadata and authored state; source reservations and runtime listeners are not copied.
- Vendors retain buying behavior. Hotbars/reference inventories are not physical sources. Character equipment exchange retains its existing actions.

## Server API

`Server_TakeItemAmount` sends an `FInventoryTakeAmountRequest` through the owned manager. The server validates source membership, slot, active interaction ownership, distance, quantity/reservations, the UI's quantity snapshot and recipient capacity. The selector itself grants no access rights.

`FItemMoveData::RequestedAmount` is used by this operation. Zero preserves legacy whole-stack requests; positive amounts must use the explicit amount request rather than the legacy drag-and-drop transfer API. `HandleTakeAmount` is the authority-only native integration method.

`Client_TakeItemAmountResult` returns the outcome. Confirmation waits for that response, rejecting repeated clicks while pending. Cancel creates no transfer. Closing a window does not undo an operation the server has already committed.

## Configuration

`InvenzaSettings` registers the `TakeAmount` text action with `RequiresAmount`, the `InventoryTakeAmount` header and localized `TakeAmountMessages`.

`WBP_TakeAmountHeader` provides the amount selector. The action and other item actions use the existing `WBP_Selection` footer and text buttons, for both player inventory and external storage. The stack-splitting UI is unchanged.

New text is stored in `ST_InvenzaRuntimeTexts` under `Invenza.TakeAmount.*`. Counter formats and pending text use editable `FText` properties. Layout is authored in Widget Blueprints.

Base `UItemBase` reservations now reduce the available quantity and replicate the reserved total for the client's range. Releasing part of a reservation preserves its remainder.
