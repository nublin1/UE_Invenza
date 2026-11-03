# Nublin Studio 2026 All Rights Reserved.
"""Read-only startup configuration check and optional equipment layout preview."""
from pathlib import Path
import json
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
OUT = Path(u.Paths.project_saved_dir()) / "StartingItems20261028"
character = u.load_asset(ROOT + "Blueprints/Examples/BP_Character")
manager = next(obj for obj in u.ObjectIterator(u.IInventoryManager)
               if obj.get_path_name().startswith(character.get_path_name().split(".")[0] + ".")
               and obj.get_name() == "BP_InvManager_GEN_VARIABLE")
general = list(manager.get_editor_property("GeneralStartingItems"))
assert len(general) == 8
assert sum(entry.amount for entry in general) == 483
equipment = []
for startup in manager.get_editor_property("StartupInventories"):
    tag = startup.settings.inventory_tag.export_text()
    if "UI.Invenza.MainInventory" in tag:
        assert len(startup.get_editor_property("StartItems")) == 0
    if "UI.Invenza.Equipment" in tag:
        equipment = list(startup.get_editor_property("StartItems"))
assert len(equipment) == 1 and str(equipment[0].item.row_name) == "BackpackSmall"
grid = u.load_asset(ROOT + "UI/Core/SimpleInvGrid/WBP_EqGrid")
tree = u.find_object(grid, "WidgetTree")
cell = u.find_object(tree, "BackpackSlot")
assert cell.slot.row == 8 and cell.slot.column == 0
assert cell.get_parent().get_name() == "SlotsGridPanel"
others = [obj.slot.row for obj in u.ObjectIterator(u.SlotbasedInventorySlot)
          if obj.get_outer() == tree and obj != cell]
assert max(others) == 6
assert "Item.Category.Backpack" in cell.get_editor_property("AllowedSlotCategory").export_text()
assert "Equipment.Slot.Backpack" in cell.get_editor_property("LinkedEquipmentSlotTag").export_text()
(OUT / "asset-verification.json").write_text(json.dumps({"general_rows": len(general),
    "general_quantity": sum(entry.amount for entry in general), "equipment_backpack_preserved": True,
    "separate_backpack_row": 8, "last_other_equipment_row": max(others), "no_assets_saved": True}, indent=2), encoding="utf-8")
if "StartingItemsPreview" in u.SystemLibrary.get_command_line():
    world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
    preview = u.InvenzaWidgetEditingLibrary.create_preview(world, grid.generated_class())
    assert u.InvenzaWidgetEditingLibrary.render_widget(preview, u.Vector2D(900, 900), str(OUT / "equipment-layout.png"))
    u.InvenzaWidgetEditingLibrary.release_preview()
    u.SystemLibrary.quit_editor()
u.log("INVENZA_PLAYER_STARTING_ITEMS_VERIFIED")
