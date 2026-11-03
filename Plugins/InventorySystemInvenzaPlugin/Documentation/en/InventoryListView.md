# Read-only list view for any container

`WBP_InventoryListView` displays an existing `UInventoryBase` without changing its storage model. The same widget accepts `USlotbasedInventory`, including storage containers, and `UListInventory`.

## Blueprint integration

1. Use **Create Widget** with `/InventorySystemInvenzaPlugin/UI/Inventory/Views/WBP_InventoryListView`, or embed it in your Widget Blueprint.
2. Obtain the source container from `InventoryCollectionComponent` using **Get Inventory By Tag** or **Get Inventory By ID**. Pass the inventory object, not an array of items or its interactive widget.
3. Call **Set Source Inventory** on the view. This works before or after adding the widget to the screen.
4. Call **Set Source Inventory** again to switch containers, or pass `None` to disconnect. Removing the widget from the screen unsubscribes it; reconstructing it reconnects to current source data.

The view does not register as the container's primary UI. Multiple views, including the regular grid, can display the same source simultaneously.

## Search, sorting and entries

- Search matches the display name, ignoring case. **Set Search Text** controls the same local filter from Blueprint.
- The `A–Z` icon toggles ascending/descending name order. The down-arrow icon sorts by quantity, largest first. Tooltips explain the current state.
- **Set Sort Mode** also supports `SourceOrder`, the collection's item order. It never sorts the source inventory.
- **Set Category Filters** takes a `GameplayTagContainer`. An empty container accepts all categories; parent tags include child categories. Category and search filters apply together.
- Each existing stack gets its own row. Identical items in different stacks remain separate. Name and quantity use separate fields.
- **Get View Entries** returns all local entries; **Get Visible Entries** returns filtered/sorted entries; **Get Selected Entry** returns the current selection. Each entry exposes **Get Item** and **Get Source Inventory**.

Each view owns its filters and entry objects. Remaining entries retain their identity across updates. Selection and scroll offset are retained where possible while the selected entry remains visible.

## Updates and access boundaries

The view subscribes to inventory, collection and item events, coalescing notifications into one next-frame refresh. Quantity updates explicitly notify existing rows. `ListView` virtualization is preserved. Save restoration and client replication notifications feed the same subscriptions; there is no inventory polling on Tick.

Rows support selection and the existing `WBP_ItemTooltip`. They provide no dragging, quick transfers, use, splitting, dropping, destruction or modifying context menu. Item drops over the view are consumed without applying an operation, preventing them from reaching an interactive inventory underneath.

The view creates no inventory slots or `UInventoryListEntry` objects and changes no layout, reservations, capacity, contents, visual links or primary-widget registration. `UInventoryViewEntry` objects are transient display data: they are neither replicated nor saved. Their source/item references are weak. Owner end play disconnects the source and clears the view.

Read-only is a UI behavior, not a server permission. The server must authorize access separately. The view displays inventory data already available on the client; it does not open private storage or grant access.

Direct quantity assignments without an event do not automatically refresh the view. Use normal Invenza operations, emit the appropriate item/collection notification, or call **Refresh View** after your own batch operation.

## Customization

Native classes: `UInventoryListViewWidget`, `UInventoryListViewRowWidget`, `UInventoryViewEntry`. Layout lives in the two new Widget Blueprints under `UI/Inventory/Views`. Rows reuse `WBP_LabelBaseText` and the existing tooltip; icon buttons reuse `WBP_UIButtonWithSlot`. Their tooltip text is stored in editable `FText` properties.

Existing `WBP_ListInv`, `WBP_ListInvSlot`, their native classes and maps retain their behavior. Embed the new view in the screen that needs it.
