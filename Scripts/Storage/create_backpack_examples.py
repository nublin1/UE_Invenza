# Nublin Studio 2026 All Rights Reserved.
"""Author the scoped backpack definitions, equipment slot and example character configuration.

Run with the rebuilt editor. Existing widget visibility, opacity and mouse settings are preserved.
Only assets explicitly listed in ALLOWED are saved; no maps or related Blueprints are saved.
"""
from pathlib import Path
import json
import shutil
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
PROJECT = Path(u.Paths.project_dir()).resolve()
OUT = PROJECT / "Saved/Backpacks20261027"
TOOLS = u.AssetToolsHelpers.get_asset_tools()
EDIT = u.InvenzaWidgetEditingLibrary
DATA = u.InvenzaDataTableEditingLibrary
ALLOWED = {
    "Data/DT_ItemData", "Data/Portable/ST_Backpacks",
    "Data/DT_Equipment", "UI/Core/SimpleInvGrid/WBP_EqGrid",
    "Blueprints/Components/BP_InvManager", "Blueprints/Examples/BP_Character",
}
OUT.mkdir(parents=True, exist_ok=True)


def backup(relative):
    source = PROJECT / "Plugins/InventorySystemInvenzaPlugin/Content" / (relative + ".uasset")
    destination = OUT / "Before/Plugins/InventorySystemInvenzaPlugin/Content" / (relative + ".uasset")
    if source.exists() and not destination.exists():
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)


def save(asset):
    relative = asset.get_path_name().split(".")[0].removeprefix(ROOT)
    assert relative in ALLOWED, relative
    assert u.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False), relative


def tag(name):
    value = u.GameplayTag()
    value.import_text('(TagName="' + name + '")')
    return value


def data_asset(name, cls, factory):
    path = ROOT + "Data/Portable/" + name
    result = u.load_object(None, path + "." + name) if (PROJECT / "Plugins/InventorySystemInvenzaPlugin/Content/Data/Portable" / (name + ".uasset")).exists() else None
    if result is None:
        result = TOOLS.create_asset(name, ROOT + "Data/Portable", cls, factory)
    assert result, path
    return result


texts = data_asset("ST_Backpacks", u.StringTable, u.StringTableFactory())
for key, value in {
    "Small.Name": "Small backpack", "Large.Name": "Large backpack",
    "Small.Description": "Portable inventory: 4 rows and 5 columns. Equip to access its contents.",
    "Large.Description": "Portable inventory: 6 rows and 7 columns. Equip to access its contents.",
    "Equip": "Equip backpack", "Slot": "Backpack",
}.items():
    EDIT.set_string_table_entry(texts, key, value)
save(texts)

table = u.load_asset(ROOT + "Data/DT_ItemData")
assert table
backup("Data/DT_ItemData")
for name, rows, columns, capacity, prefix, weight in [
    ("BackpackSmall", 4, 5, 50.0, "Small", 1.5),
    ("BackpackLarge", 6, 7, 65.0, "Large", 2.5),
]:
    row = u.ItemData()
    row.set_editor_property("id", name)
    metadata = row.get_editor_property("ItemMetaData").copy()
    inventory = metadata.get_editor_property("ItemInventoryData")
    inventory.set_editor_properties({"Rows": rows, "Columns": columns, "MaxWeight": capacity,
                                     "EquipAction": EDIT.string_table_text(texts, "Equip")})
    metadata.set_editor_property("ItemInventoryData", inventory)
    metadata.set_editor_property("item_category", tag("Item.Category.Backpack"))
    numeric = metadata.get_editor_property("ItemNumeraticData").copy()
    numeric.set_editor_property("weight", weight)
    numeric.set_editor_property("max_stack_size_in_character", 1)
    numeric.set_editor_property("inventory_horizontal_slots", 2)
    numeric.set_editor_property("inventory_vertical_slots", 3)
    metadata.set_editor_property("item_numeratic_data", numeric)
    text = metadata.get_editor_property("ItemTextData").copy()
    text.set_editor_property("display_name", EDIT.string_table_text(texts, prefix + ".Name"))
    text.set_editor_property("item_description", EDIT.string_table_text(texts, prefix + ".Description"))
    metadata.set_editor_property("item_text_data", text)
    assets = metadata.get_editor_property("ItemAssetData").copy()
    # Reuse one existing prototype resource until custom backpack art is supplied.
    assets.set_editor_property("icon", u.load_asset(ROOT + "Textures/ItemCategory/T_Allcategory"))
    assets.set_editor_property("mesh", u.load_asset(ROOT + "Meshes/StorageLabels/SM_StorageLabelBoxMedium"))
    assets.set_editor_property("MeshScale", u.Vector(0.5, 0.5, 0.625))
    metadata.set_editor_property("item_asset_data", assets)
    row.set_editor_property("item_meta_data", metadata)
    assert DATA.set_row_from_text(table, name, row.export_text())
save(table)

backup("Data/DT_Equipment")
equipment_table = u.load_asset(ROOT + "Data/DT_Equipment")
row = u.EquipmentSlotDefinition()
row.set_editor_property("slot_tag", tag("Equipment.Slot.Backpack"))
row.set_editor_property("allowed_category", tag("Item.Category.Backpack"))
assert DATA.set_row_from_text(equipment_table, "Backpack", row.export_text())
save(equipment_table)

backup("UI/Core/SimpleInvGrid/WBP_EqGrid")
grid_bp = u.load_asset(ROOT + "UI/Core/SimpleInvGrid/WBP_EqGrid")
tree = u.find_object(grid_bp, "WidgetTree")
panel = u.find_object(tree, "SlotsGridPanel")
backpack_slot = u.find_object(tree, "BackpackSlot")
if not backpack_slot:
    source = next(w for w in u.ObjectIterator(u.SlotbasedInventorySlot) if w.get_outer() == tree)
    backpack_slot = EDIT.add_template(grid_bp, source.get_class(), "BackpackSlot")
    # The existing shared slot Blueprint supplies the complete visual hierarchy.
    panel.add_child_to_uniform_grid(backpack_slot, 8, 0)
backpack_slot.set_editor_properties({"allowed_slot_category": tag("Item.Category.Backpack"),
                                    "linked_equipment_slot_tag": tag("Equipment.Slot.Backpack")})
backpack_slot.set_tool_tip_text(EDIT.string_table_text(texts, "Slot"))
assert EDIT.compile_widget(grid_bp)
save(grid_bp)

example_entries = []
for name in ["BackpackSmall", "BackpackLarge"]:
    handle = u.DataTableRowHandle(data_table=table, row_name=name)
    example_entries.append(u.InitItemsEntry(item=handle, amount=1))
equipment_handle = u.DataTableRowHandle(data_table=equipment_table, row_name="Backpack")
changed_components = []
for path in ["Blueprints/Components/BP_InvManager", "Blueprints/Examples/BP_Character"]:
    backup(path)
    bp = u.load_asset(ROOT + path)
    cdo = u.get_default_object(bp.generated_class())
    objects = [cdo]
    objects += [obj for obj in u.ObjectIterator(u.ActorComponent)
                if obj.get_path_name().startswith(ROOT + path + ".") and obj not in objects]
    for obj in objects:
        if isinstance(obj, u.IInventoryManager):
            obj.set_editor_property("use_portable_backpacks", True)
            obj.set_editor_property("example_ground_backpacks", example_entries)
            changed_components.append(obj.get_path_name())
        if isinstance(obj, u.EquipmentComponent):
            handles = list(obj.get_editor_property("slot_rows"))
            if not any(str(entry.row_name) == "Backpack" for entry in handles):
                handles.append(equipment_handle)
                obj.set_editor_property("slot_rows", handles)
    u.BlueprintEditorLibrary.compile_blueprint(bp)
    save(bp)

(OUT / "authored-assets.json").write_text(json.dumps({"assets": sorted(ALLOWED),
                                                     "manager_templates": changed_components}, indent=2), encoding="utf-8")
u.log("INVENZA_BACKPACK_ASSETS_OK")
