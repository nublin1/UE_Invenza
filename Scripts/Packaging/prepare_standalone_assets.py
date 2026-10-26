# Nublin Studio 2026 All Rights Reserved.
"""Apply only the four packages listed in Saved/FabReadiness/asset_change_manifest.md."""

from pathlib import Path
import csv
import json
import shutil

import unreal as u


project = Path(u.Paths.project_dir()).resolve()
root = "/InventorySystemInvenzaPlugin/"
output = project / "Saved/FabReadiness"
backup = output / "Before"
edit = u.InvenzaWidgetEditingLibrary
registry = u.AssetRegistryHelpers.get_asset_registry()
registry.search_all_assets(True)


def preserve(package):
    path = project / "Plugins/InventorySystemInvenzaPlugin/Content" / (package.removeprefix(root) + ".uasset")
    destination = backup / path.relative_to(project)
    if path.exists() and not destination.exists():
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, destination)


def widget(blueprint, name):
    tree = u.find_object(blueprint, "WidgetTree")
    result = u.find_object(tree, name)
    assert result, name
    result.modify()
    return result


# Keep source captions editable outside runtime C++ and preserve the existing language.
captions = {}
with (project / "Scripts/UI/Data/FrontendMenuTexts.csv").open(encoding="utf-8", newline="") as stream:
    captions = {row["Key"]: row["SourceString"] for row in csv.DictReader(stream) if row["Key"] in {"SaveTitle", "LoadTitle", "Back"}}
assert len(captions) == 3
table_package = root + "Data/Localization/ST_InvenzaMenuPages"
preserve(table_package)
table = u.load_asset(table_package)
if table is None:
    directory, name = table_package.rsplit("/", 1)
    table = u.AssetToolsHelpers.get_asset_tools().create_asset(name, directory, None, u.StringTableFactory())
assert table
for key, value in captions.items():
    edit.set_string_table_entry(table, key, value)
assert u.EditorAssetLibrary.save_loaded_asset(table, only_if_is_dirty=False)

for name, title in [("WBP_SaveGamePage", "SaveTitle"), ("WBP_LoadGamePage", "LoadTitle")]:
    package = root + "UI/SaveLoad/" + name
    preserve(package)
    blueprint = u.load_asset(package)
    assert blueprint
    blueprint.modify()
    widget(blueprint, "TitleLabel").set_editor_property("text", edit.string_table_text(table, title))
    widget(blueprint, "CloseButtonLabel").set_editor_property("text", edit.string_table_text(table, "Back"))
    assert edit.compile_widget(blueprint)
    assert u.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)

# Replace one object reference without round-tripping localized row data through JSON.
items_package = root + "Data/DT_ItemData"
preserve(items_package)
items = u.load_asset(items_package)
before = u.DataTableFunctionLibrary.export_data_table_to_json_string(items)
old_mesh = u.load_asset("/Game/LevelPrototyping/Meshes/SM_Cube")
new_mesh = u.load_asset("/Engine/BasicShapes/Cube")
assert old_mesh and new_mesh
replacements = u.InvenzaDataTableEditingLibrary.replace_table_object_reference(items, old_mesh, new_mesh)
after = u.DataTableFunctionLibrary.export_data_table_to_json_string(items)
old_path = old_mesh.get_path_name()
new_path = new_mesh.get_path_name()
assert json.loads(before.replace(old_path, new_path)) == json.loads(after), "Unexpected item-row changes"
assert old_path not in after
if replacements:
    assert u.EditorAssetLibrary.save_loaded_asset(items, only_if_is_dirty=False)
(output / "item_rows_before.json").write_text(before, encoding="utf-8")
(output / "item_rows_after.json").write_text(after, encoding="utf-8")
(output / "asset_changes.json").write_text(json.dumps({
    "saved_packages": [table_package, root + "UI/SaveLoad/WBP_SaveGamePage", root + "UI/SaveLoad/WBP_LoadGamePage", items_package],
    "item_mesh_references_replaced": replacements,
}, indent=2), encoding="utf-8")
u.log("INVENZA_STANDALONE_ASSETS_READY")
