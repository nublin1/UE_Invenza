# Shelf fill examples

Language: [Русский](../ru/ExampleShelfFill.md) | **English**

Two existing racks in `Map_Ex` use different `Occupancy Meshes`:

| Instance | Thresholds (0–1) | Appearance |
|---|---|---|
| `BP_Container`, X=1165 | 0, 1 | Empty / full |
| `BP_Container2`, X=1815 | 0, 0.25, 0.5, 0.75, 1 | Empty / 25 / 50 / 75 / 100% |

The second rack selects the highest threshold not exceeding actual occupancy. For example, 60% uses the 50% mesh. Both inventories are grids, so occupancy counts occupied cells rather than units in a stack. Starting contents were preserved when setting up this example.

Intermediate meshes are in `/InventorySystemInvenzaPlugin/Meshes/Rack/`: `SM_Rack_Fill_25`, `SM_Rack_Fill_50`, and `SM_Rack_Fill_75`. They preserve the original frame, dimensions of 300 × 80 × 200 cm, pivot, materials, and collision. They contain 4, 8, and 11 boxes; the original full mesh has 15. Filling proceeds from the bottom shelf upward. Sources are in `ArtSource/RackFill/`.

Settings are level-instance overrides; the shared container Blueprint was not changed. Labels are centered above the racks at the same height. Two old control blocks were replaced with `Press F6 for controls help`.
