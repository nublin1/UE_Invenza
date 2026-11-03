# Nublin Studio 2026 All Rights Reserved.
"""Move backpack rows into DT_ItemData and replace labeled box duplicates with one mesh.

Only DT_ItemData, BP_InvManager and BP_Character are saved. Deletions require the prior
dependency inspection and a backup in Saved/ItemDataRefactor20261028/Before.
"""
from pathlib import Path
import json
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
OUT = Path(u.Paths.project_saved_dir()) / "ItemDataRefactor20261028"
report = json.loads((OUT / "dependency-inspection.json").read_text(encoding="utf-8"))
EDIT = u.InvenzaDataTableEditingLibrary
registry = u.AssetRegistryHelpers.get_asset_registry()
registry.search_all_assets(True)
options = u.AssetRegistryDependencyOptions(include_hard_package_references=True, include_soft_package_references=True)
allowed_saves = {ROOT + "Data/DT_ItemData", ROOT + "Blueprints/Components/BP_InvManager",
                 ROOT + "Blueprints/Examples/BP_Character"}


def save(asset):
    assert asset.get_path_name().split(".")[0] in allowed_saves
    assert u.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)


def row_from_json(entry):
    row = u.ItemData()
    row.set_editor_property("ID", entry["ID"])
    metadata = u.ItemMetaData()
    assert metadata.import_text(entry["ItemMetaData"])
    row.set_editor_property("ItemMetaData", metadata)
    return row


assert (OUT / "Before/Plugins/InventorySystemInvenzaPlugin/Content/Data/DT_ItemData.uasset").exists()
common = u.load_asset(ROOT + "Meshes/StorageLabels/SM_StorageLabelBoxMedium")
assert common
common_record = next(record for record in report["meshes"] if record["path"] == common.get_path_name())
replacements = {}
for record in report["meshes"]:
    if record["path"] == common.get_path_name():
        continue
    assert set(record["referencers"]) <= {ROOT + "Data/DT_ItemData", ROOT + "Data/Portable/DT_Backpacks"}, record
    replacements[record["path"]] = record

table = u.load_asset(ROOT + "Data/DT_ItemData")
entries = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(table))
old_names = {entry["Name"] for entry in entries}
for entry in report["tables"]["Data/Portable/DT_Backpacks"]:
    assert entry["Name"] not in old_names, "Backpack row already exists: " + entry["Name"]
    row = row_from_json(entry)
    metadata = row.get_editor_property("ItemMetaData")
    suffix = "Small" if entry["Name"] == "BackpackSmall" else "Large"
    definition = u.load_asset(ROOT + "Data/Portable/DA_Backpack" + suffix)
    assert definition
    data = metadata.get_editor_property("ItemInventoryData")
    template = definition.get_editor_property("Inventory")
    grid = template.get_editor_property("InventorySlotBasedSettings")
    data.set_editor_properties({"Rows": grid.get_editor_property("InitNumberRows"),
                                "Columns": grid.get_editor_property("InitNumColumns"),
                                "MaxWeight": template.get_editor_property("InventoryMaxWeightCapacity"),
                                "EquipAction": definition.get_editor_property("EquipAction")})
    metadata.set_editor_property("ItemInventoryData", data)
    row.set_editor_property("ItemMetaData", metadata)
    assert EDIT.set_row_from_text(table, entry["Name"], row.export_text())

changes = []
entries = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(table))
for entry in entries:
    row = row_from_json(entry)
    metadata = row.get_editor_property("ItemMetaData")
    changed = False
    for group_name, mesh_property in [("ItemAssetData", "Mesh"), ("ItemStorageData", "StorageMesh")]:
        group = metadata.get_editor_property(group_name)
        previous = group.get_editor_property(mesh_property)
        record = replacements.get(previous.get_path_name()) if previous else None
        if not record:
            continue
        scale = u.Vector(*(value / baseline for value, baseline in zip(record["size"], common_record["size"])))
        group.set_editor_property(mesh_property, common)
        group.set_editor_property("MeshScale", scale)
        group.set_editor_property("Materials", [u.load_asset(path) if path else None for path in record["materials"]])
        metadata.set_editor_property(group_name, group)
        changes.append({"row": entry["Name"], "group": group_name, "previous": record["path"],
                        "mesh": common.get_path_name(), "materials": record["materials"],
                        "scale": [scale.x, scale.y, scale.z]})
        changed = True
    if changed:
        row.set_editor_property("ItemMetaData", metadata)
        assert EDIT.set_row_from_text(table, entry["Name"], row.export_text())
save(table)

for path in ["Blueprints/Components/BP_InvManager", "Blueprints/Examples/BP_Character"]:
    bp = u.load_asset(ROOT + path)
    objects = [u.get_default_object(bp.generated_class())]
    objects += [obj for obj in u.ObjectIterator(u.ActorComponent)
                if obj.get_path_name().startswith(ROOT + path + ".") and obj not in objects]
    for obj in objects:
        if isinstance(obj, u.IInventoryManager):
            entries = list(obj.get_editor_property("ExampleGroundBackpacks"))
            for entry in entries:
                handle = entry.item
                if str(handle.row_name) in {"BackpackSmall", "BackpackLarge"}:
                    handle.set_editor_property("DataTable", table)
                    entry.set_editor_property("Item", handle)
            obj.set_editor_property("ExampleGroundBackpacks", entries)
    u.BlueprintEditorLibrary.compile_blueprint(bp)
    save(bp)

deletions = [ROOT + "Data/Portable/DT_Backpacks", ROOT + "Data/Portable/DA_BackpackSmall",
             ROOT + "Data/Portable/DA_BackpackLarge"]
deletions += [path.split(".")[0] for path in replacements]
deleted = []
for path in deletions:
    referencers = set(u.EditorAssetLibrary.find_package_referencers_for_asset(path, load_assets_to_confirm=True))
    assert referencers <= set(deletions), {"asset": path, "remaining_referencers": sorted(referencers)}
    assert u.EditorAssetLibrary.delete_asset(path), path
    deleted.append(path)

(OUT / "migration-report.json").write_text(json.dumps({"table": table.get_path_name(),
    "row_count": len(u.DataTableFunctionLibrary.get_data_table_row_names(table)), "mesh": common.get_path_name(),
    "changes": changes, "deleted": deleted, "saved": sorted(allowed_saves)}, indent=2), encoding="utf-8")
u.log("ITEM_DATA_MIGRATION_OK " + json.dumps({"rows": 17, "deleted": len(deleted), "changed_mesh_fields": len(changes)}))
