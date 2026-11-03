# Nublin Studio 2026 All Rights Reserved.
"""Verify migrated saved assets in a fresh editor process without saving any assets."""
from pathlib import Path
import json
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
OUT = Path(u.Paths.project_saved_dir()) / "ItemDataRefactor20261028"
report = json.loads((OUT / "migration-report.json").read_text(encoding="utf-8"))
table = u.load_asset(ROOT + "Data/DT_ItemData")
assert table
rows = {row["Name"]: row for row in json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(table))}
assert len(rows) == report["row_count"]
metadata = {}
for name, entry in rows.items():
    value = u.ItemMetaData()
    assert value.import_text(entry["ItemMetaData"])
    metadata[name] = value
for name, size, capacity in [("BackpackSmall", (4, 5), 35.0), ("BackpackLarge", (6, 7), 65.0)]:
    data = metadata[name].get_editor_property("ItemInventoryData")
    assert (data.rows, data.columns, data.max_weight) == (*size, capacity)
for change in report["changes"]:
    group = metadata[change["row"]].get_editor_property(change["group"])
    mesh_key = "Mesh" if change["group"] == "ItemAssetData" else "StorageMesh"
    assert group.get_editor_property(mesh_key).get_path_name() == report["mesh"]
    materials = [value.get_path_name() if value else None for value in group.get_editor_property("Materials")]
    assert materials == change["materials"], change
    scale = group.get_editor_property("MeshScale")
    assert all(abs(a-b) < 0.0001 for a, b in zip((scale.x, scale.y, scale.z), change["scale"]))
for path in report["deleted"]:
    relative = path.removeprefix(ROOT) + ".uasset"
    assert not (Path(u.Paths.project_dir()) / "Plugins/InventorySystemInvenzaPlugin/Content" / relative).exists()
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
actor_class = u.load_class(None, ROOT + "Blueprints/Examples/BP_Character.BP_Character_C")
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
actor = actors.spawn_actor_from_class(actor_class, u.Vector(0, 0, -10000))
try:
    manager = actor.get_component_by_class(u.IInventoryManager)
    handles = manager.get_editor_property("ExampleGroundBackpacks")
    assert len(handles) == 2
    assert all(entry.item.data_table == table for entry in handles)
finally:
    actors.destroy_actor(actor)
(OUT / "verification.json").write_text(json.dumps({"row_count": len(rows), "backpacks_in_common_table": True,
    "legacy_save_coverage": "Invenza.Inventory.ItemRowConfiguration", "box_mesh_count": 1, "verified_material_fields": len(report["changes"]),
    "no_assets_saved": True}, indent=2), encoding="utf-8")
u.log("ITEM_DATA_REFACTOR_VERIFIED")
