# Multi-item pickups and dismantling

Example: `/InventorySystemInvenzaPlugin/Blueprints/Examples/Pickups/BP_ScrapBox`.
Place it in a level, aim at the box, and hold Primary interaction for two seconds.
Releasing early cancels dismantling. Completion grants `Board ×2`, `Rope ×1`, and `IronIngot ×1` from the shared `DT_ItemData`.

Configure `PickupComponent`:

- `InitialItems`: item row handles and quantities. A nonempty list enables bundle mode; `InitialItem` remains the legacy single-item setting.
- `MeshSource = CustomMesh`: separately authored `CustomMesh`, `CustomMaterials`, and `CustomMeshScale`. The example reuses an existing box mesh.
- `MeshSource = FirstItem`: use the first resource's mesh, materials, and scale without changing bundle contents.
- `VisualMeshComponent`: mesh component to update; defaults to the owner's first Static Mesh.
- `OverflowDropOffset`: local offset for overflow drops. The global `PickupClass` must contain a `PickupComponent`.
- `InteractableDataMap → Primary`: editable name, action, active action, hold flag, and duration. The example's Russian labels live in `ST_PickupExamples` beside the Blueprint.

Bundles use `GetReceivingInventories`: the main backpack first, then owned ordinary storages with `bReceiveGrantedItems = true`. Widget classes and `bOpenWithExternalInventory` do not affect receipt. Equipment, hotbar references, trading, and service inventories are excluded. Weight, dimensions, and stacking apply; quantities above a stack limit are split without truncation.

Overflow becomes ordinary ground pickups. Without receiving storage the entire bundle is dropped. Processing consumes the source box. If a ground pickup cannot be created, its remainder is discarded, an error logs the resource, quantity, and instance ID, and the source is still consumed. No delivery retry occurs.

Server gameplay can call `InitializeItems` on a spawned actor. Saves preserve all bundle items and consumed sources; granting and destruction run on authority. The example actor replicates. Legacy `InitializeDrop` and singular pickup save records remain supported.

Hold duration is enforced by a server Begin/Cancel/Complete session. Completion cannot shorten the server interval; cancellation, distance, exclusive access, and duplicate completion are checked on authority. Direct inventory interaction RPCs cannot bypass this session. Server AI can start the same protocol through `UInteractionComponent::RequestInteraction`. Abandoned incomplete sessions expire after the hold plus `CompletionRequestGracePeriod`.
