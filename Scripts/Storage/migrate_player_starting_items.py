# Nublin Studio 2026 All Rights Reserved.
"""Migrate only BP_Character's ordinary startup items and separate WBP_EqGrid's backpack cell."""
from pathlib import Path
import json
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
OUT = Path(u.Paths.project_saved_dir()) / "StartingItems20261028"
OUT.mkdir(parents=True, exist_ok=True)


def entries(values):
    return [{"table": value.item.data_table.get_path_name() if value.item.data_table else None,
             "row": str(value.item.row_name), "amount": value.amount} for value in values]


def visibility_snapshot(tree):
    return {obj.get_name(): {"visibility": str(obj.get_editor_property("visibility")),
                             "opacity": obj.get_editor_property("render_opacity")}
            for obj in u.ObjectIterator(u.Widget) if obj.get_outer() == tree}


character = u.load_asset(ROOT + "Blueprints/Examples/BP_Character")
managers = [obj for obj in u.ObjectIterator(u.IInventoryManager)
            if obj.get_path_name().startswith(character.get_path_name().split(".")[0] + ".")
            and obj.get_name() == "BP_InvManager_GEN_VARIABLE"]
assert len(managers) == 1, [obj.get_path_name() for obj in managers]
manager = managers[0]
inventories = list(manager.get_editor_property("StartupInventories"))
equipment_before = [entries(entry.get_editor_property("StartItems")) for entry in inventories
                    if "UI.Invenza.Equipment" in entry.settings.inventory_tag.export_text()]
general = list(manager.get_editor_property("GeneralStartingItems"))
old_general = entries(general)
migrated = []
for index, entry in enumerate(inventories):
    if "UI.Invenza.MainInventory" in entry.settings.inventory_tag.export_text():
        starting = list(entry.get_editor_property("StartItems"))
        migrated += entries(starting)
        general += starting
        entry.set_editor_property("StartItems", [])
        inventories[index] = entry
manager.set_editor_property("GeneralStartingItems", general)
manager.set_editor_property("StartupInventories", inventories)
u.BlueprintEditorLibrary.compile_blueprint(character)
assert entries(manager.get_editor_property("GeneralStartingItems")) == old_general + migrated
assert [entries(entry.get_editor_property("StartItems")) for entry in manager.get_editor_property("StartupInventories")
        if "UI.Invenza.Equipment" in entry.settings.inventory_tag.export_text()] == equipment_before
assert u.EditorAssetLibrary.save_loaded_asset(character, only_if_is_dirty=False)

grid = u.load_asset(ROOT + "UI/Core/SimpleInvGrid/WBP_EqGrid")
tree = u.find_object(grid, "WidgetTree")
before = visibility_snapshot(tree)
cell = u.find_object(tree, "BackpackSlot")
assert cell and cell.get_parent().get_name() == "SlotsGridPanel"
category = cell.get_editor_property("AllowedSlotCategory").export_text()
tag = cell.get_editor_property("LinkedEquipmentSlotTag").export_text()
old_row = cell.slot.get_editor_property("Row")
cell.slot.set_editor_property("Row", 8)
cell.slot.set_editor_property("Column", 0)
assert u.InvenzaWidgetEditingLibrary.compile_widget(grid)
assert visibility_snapshot(tree) == before
assert cell.get_editor_property("AllowedSlotCategory").export_text() == category
assert cell.get_editor_property("LinkedEquipmentSlotTag").export_text() == tag
assert u.EditorAssetLibrary.save_loaded_asset(grid, only_if_is_dirty=False)

(OUT / "migration.json").write_text(json.dumps({"general_starting_items": entries(general),
    "migrated": migrated, "equipment_starting_items_preserved": equipment_before,
    "backpack_row_before": old_row, "backpack_row_after": 8,
    "visibility_opacity_preserved": True,
    "saved_assets": [character.get_path_name(), grid.get_path_name()]}, indent=2), encoding="utf-8")
u.log("INVENZA_STARTING_ITEMS_MIGRATED")
