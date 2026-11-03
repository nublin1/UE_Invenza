# Nublin Studio 2026 All Rights Reserved.
"""Read-only verification and rendered preview of the authored backpack assets."""
from pathlib import Path
import json
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
OUT = Path(u.Paths.project_saved_dir()) / "Backpacks20261027"
EDIT = u.InvenzaWidgetEditingLibrary
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
results = []
table = u.load_asset(ROOT + "Data/DT_ItemData")
assert table
entries = {entry["Name"]: entry for entry in json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(table))}
for name, size in [("Small", (4, 5)), ("Large", (6, 7))]:
    metadata = u.ItemMetaData()
    assert metadata.import_text(entries["Backpack" + name]["ItemMetaData"])
    data = metadata.get_editor_property("ItemInventoryData")
    assert (data.rows, data.columns) == size
    results.append({"row": "Backpack" + name, "size": size, "capacity": data.max_weight})

actor_class = u.load_class(None, ROOT + "Blueprints/Examples/BP_Character.BP_Character_C")
actor = u.get_editor_subsystem(u.EditorActorSubsystem).spawn_actor_from_class(actor_class, u.Vector(0, 0, -10000))
try:
    manager = actor.get_component_by_class(u.IInventoryManager)
    assert manager and manager.get_editor_property("use_portable_backpacks")
    assert len(manager.get_editor_property("example_ground_backpacks")) == 2
    equipment = actor.get_component_by_class(u.EquipmentComponent)
    assert any(str(handle.row_name) == "Backpack" for handle in equipment.get_editor_property("slot_rows"))
finally:
    u.get_editor_subsystem(u.EditorActorSubsystem).destroy_actor(actor)

bp = u.load_asset(ROOT + "UI/Core/SimpleInvGrid/WBP_EqGrid")
tree = u.find_object(bp, "WidgetTree")
slot = u.find_object(tree, "BackpackSlot")
assert slot and str(slot.get_editor_property("linked_equipment_slot_tag").export_text()).find("Equipment.Slot.Backpack") >= 0
preview = EDIT.create_preview(world, bp.generated_class())
assert preview
assert EDIT.render_widget(preview, u.Vector2D(900, 900), str(OUT / "equipment-preview.png"))
EDIT.release_preview()
(OUT / "asset-verification.json").write_text(json.dumps({"definitions": results,
    "character_uses_backpacks": True, "equipment_slot_present": True,
    "no_assets_saved": True}, indent=2), encoding="utf-8")
u.log("INVENZA_BACKPACK_EXAMPLES_VERIFIED")
if "BackpackVerificationExit" in u.SystemLibrary.get_command_line():
    u.SystemLibrary.quit_editor()
