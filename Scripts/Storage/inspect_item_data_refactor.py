# Nublin Studio 2026 All Rights Reserved.
"""Read-only dependency inventory before consolidating item definitions and box meshes."""
from pathlib import Path
import json
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
OUT = Path(u.Paths.project_saved_dir()) / "ItemDataRefactor20261028"
registry = u.AssetRegistryHelpers.get_asset_registry()
registry.search_all_assets(True)
options = u.AssetRegistryDependencyOptions(include_hard_package_references=True, include_soft_package_references=True, include_searchable_names=True, include_soft_management_references=True, include_hard_management_references=True)
records = []
for data in registry.get_assets_by_path(ROOT + "Meshes/StorageLabels", recursive=False):
    name = str(data.asset_name)
    if not (name.startswith("SM_StorageBox_") or name.startswith("SM_StorageLabelBox")):
        continue
    mesh = data.get_asset()
    bounds = mesh.get_bounds()
    extent = bounds.box_extent
    records.append({"path": mesh.get_path_name(), "size": [extent.x * 2, extent.y * 2, extent.z * 2],
                    "materials": [str(material.material_interface.get_path_name()) if material.material_interface else None
                                  for material in mesh.get_editor_property("static_materials")],
                    "referencers": [str(value) for value in registry.get_referencers(data.package_name, options)]})
tables = {}
for relative in ["Data/DT_ItemData", "Data/Portable/DT_Backpacks"]:
    table = u.load_asset(ROOT + relative)
    assert table
    tables[relative] = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(table))
report = {"meshes": records, "tables": tables,
          "backpack_table_referencers": [str(value) for value in registry.get_referencers(ROOT + "Data/Portable/DT_Backpacks", options)]}
(OUT / "dependency-inspection.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
u.log("ITEM_DATA_DEPENDENCIES_INSPECTED " + json.dumps({"meshes": len(records), "rows": {key: len(value) for key, value in tables.items()},
                                                        "backpack_table_referencers": report["backpack_table_referencers"]}))
