# Linked shared and personal chests

Linked chests open one physical inventory owned by a persistent-level actor.
Items are not copied between access points. Destroying a chest or unloading its
streaming level preserves the inventory.

## Examples

`Blueprints/Examples/Storage/Linked` contains `BP_LinkedStorageChest`,
`BP_SharedStorageChest` and `BP_PersonalStorageChest`. They reuse the existing
chest mesh and `WBP_StorageAccess` window. The Example Project includes
`Maps/Map_Ex` with two chests of each mode, labelled `SHARED` and `PERSONAL`,
along the far side of the platform at Y=1900. The map, character, GameMode, HUD and required assets are included in the plugin ZIP; open the map and press Play. Each new physical
inventory starts with three vines; reopening or loading an empty inventory does
not issue them again.

The mesh collision must block the channel selected by `Invenza Inventory → InteractionTraceChannel`.
The ready-made examples are configured for this; the Example Project uses `Interaction`.

## Configure a group

1. Create a `LinkedStorageDefinition` Data Asset and assign a nonempty `StorageGroupID`.
2. Configure `Inventory.Settings`: physical inventory class, capacity, restrictions,
   existing widget classes, and row/column counts for a grid.
3. Optionally configure `Inventory.StartItems`. All initial items must fit.
4. Assign the **same definition asset** to all chests in the group.
5. Set each chest's `LinkedStorage.StorageAccessMode` to `Shared` or `Personal`.
6. Assign the existing window class and an editable/localizable `WindowTitle`.

Different groups and access modes have independent inventories. Different
definition assets with the same group ID are rejected. Changing an established
group ID creates a different storage identity; keep IDs stable across saves.
Initial quantities larger than one stack are split into separate stacks. Invalid
or overflowing initial contents cause creation to fail without exposing a partial
inventory. The owner disables item referencing and client-widget capacity discovery.

## Exclusive group access

One player at a time can open a shared group. Every other chest in that group
rejects access until the active player closes it, leaves interaction range, loses
possession, disconnects, or the active chest is destroyed. A stale close request
cannot release another player's lease. Both `MaxUseDistance` and the system's
interaction distance apply.

Personal inventories are leased per account. Different players can access their
own private inventories through the same chest simultaneously. Contents are sent
only to the currently authorized connection, regardless of actor distance.

## Personal account identity

Standalone play and the local listen-server host use `LocalHost`, scoped to the
current saved playthrough. Supported non-NULL Online Subsystem account IDs use
their provider type and value. The project's login flow must authenticate these
identities on the server.

For custom authentication or LAN, attach `LinkedStorageIdentityComponent` to the
`PlayerController`. After successful server authentication, call
`SetPersistentPlayerID` before opening a personal chest. Assign the same account
ID after reconnection. Assignment is server-only and write-once; it has no client
RPC and is not replicated. Never trust an account ID supplied by an unauthenticated
client. Display names and temporary NULL/LAN IDs are not personal identities.
Remote LAN clients without an assigned stable ID cannot open personal storage.
Shared storage does not require an account ID.

## Persistence and reset

The existing save system captures each physical owner once, including offline
accounts and groups without remaining chests. Storage identities include mode,
group and, for personal storage, the account ID. Loading recreates owners and
items without refilling empty containers. Open windows and leases are transient.
`ResetPlaythrough` resets both modes with the rest of the current world.

Storage is scoped to the current world and save profile. Sharing one personal
inventory across unrelated maps or servers requires a separate account service.

Transfers across a linked-storage owner boundary create the physical item under
its new UObject owner. Full transfers preserve persistent identity and custom
state; partial transfers retain the untransferred quantity in the source.
Closing a storage actor channel therefore does not invalidate an item already
withdrawn into a player's inventory.

## Validation

`Invenza.Storage.LinkedStorage` covers group leases, private inventories,
controller replacement, distance, partial/full transfers and save/load. A separate
project-only test host checks actual replication with one dedicated server and two
clients, including a withdrawn item surviving storage-channel closure.
