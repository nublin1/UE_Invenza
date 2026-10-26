# Bulk bins

Language: [Русский](../ru/BulkBins.md) | **English**

The map `/InventorySystemInvenzaPlugin/Maps/Map_Ex` contains a dedicated actor named **Bulk Bins - Iron Ore**, in front of the player spawn at **X=1450, Y=1050, Z=0**. A BULK BINS label marks it.

After Play, the storage contains **145 units of iron ore**: a full stack of 100 and a partial stack of 45. Each stack occupies its own 2 × 2 cell bin. Open storage using normal interaction; take some ore, split a stack, or move it to empty cells. The pile and walls update through inventory events. Before Play, the editor shows platforms; starting items are created at runtime.

## Item settings

`DT_ItemData` contains the **IronOreBulk** row; other rows were not changed for this example.

Under `ItemMetaData → ItemStorageData`:

- `StorageMethod = BulkBins` enables this mode.
- `StorageMesh` is the mesh of one rock or other pile element.
- `BulkMaterial` optionally replaces mesh materials. Leave empty to keep them.
- `BulkFillHeight` sets maximum pile height in grid-local centimeters.
- `BulkChunkSize` sets the desired element size. Density is automatically capped for large bins.

Fill fraction is `Quantity / MaxStackSizeInCharacter`. Geometry represents the fraction, not the exact unit count: each bin uses at most 512 mesh instances. Materials must support `Used with Instanced Static Meshes`.

## Storage settings

Under `StorageGridVisualizer → Bulk Bins`:

- `BulkWallMesh`: wall mesh; defaults to the engine cube.
- `BulkWallMaterial`: wall material.
- `BulkWallHeight`, `BulkWallThickness`: wall dimensions.
- `BulkBinInset`: inset from the outer boundary of occupied cells.

The wall mesh is fitted to all four sides. There are no dividers between cells of one stack; neighboring stacks have separate walls. Reducing quantity keeps the bin boundary. Removing the stack removes the pile and walls. Interaction collision covers both.

Layout is deterministic: remaining rocks keep their positions when quantity changes. Rotation and movement affect the whole bin. Physics simulation, settling, shared walls, and per-bin tooltips are not used.

## Example sources and checks

- `ArtSource/StorageGrid/BulkBins`: Blender scene, FBX, and icon PNG.
- `Scripts/Storage/create_bulk_ore_mesh.py`: Blender geometry generation.
- `Scripts/Storage/setup_bulk_bins_example.py`: Unreal Python import and setup; rerunning updates only IronOreBulk and the dedicated example.
- `Invenza.Storage.BulkBins.Layout`: bounds, arbitrary pivot, determinism, geometry limit.
- `Invenza.Storage.BulkBins.Inventory`: adding, consumption, rotated movement, splitting, rebuilding, and wall removal.
- `Invenza.Storage.BulkBins.ExampleMap`: saved map in PIE, stacks 100 + 45, and screenshot `Saved/BulkBins/preview.png`.

Historical automation names in this guide refer to checks whose source files were removed at the owner's request. They are not available in the current distribution; use the described manual scenarios.
