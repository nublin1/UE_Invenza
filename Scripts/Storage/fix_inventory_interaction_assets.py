# Nublin Studio 2026 All Rights Reserved.
"""Apply only the assets authorized for the inventory interaction fixes."""
from pathlib import Path
import json
import re
import shutil
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
PROJECT = Path(u.Paths.project_dir()).resolve()
OUT = PROJECT / "Saved/InventoryInteraction20261028"
EDIT = u.InvenzaWidgetEditingLibrary
ALLOWED = {
    "UI/InventoryContainer/WBP_InvContainer",
    "UI/InventoryContainer/Interaction/WBP_Dual",
    "UI/Inventory/Workspace/WBP_InventoryWorkspace",
    "Data/DT_ItemData",
    "Meshes/StorageLabels/SM_StorageLabelBoxMedium",
    "Blueprints/Examples/BP_Character",
}
saved = []


def asset(relative):
    assert relative in ALLOWED
    source = PROJECT / "Plugins/InventorySystemInvenzaPlugin/Content" / (relative + ".uasset")
    backup = OUT / "Before/Content" / (relative + ".uasset")
    if not backup.exists():
        backup.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, backup)
    return u.load_asset(ROOT + relative)


def save(obj):
    relative = obj.get_path_name().split(".")[0].removeprefix(ROOT)
    assert relative in ALLOWED
    assert u.EditorAssetLibrary.save_loaded_asset(obj, only_if_is_dirty=False)
    saved.append(relative)


bp = asset("UI/InventoryContainer/WBP_InvContainer")
tree = u.find_object(bp, "WidgetTree")
before = {obj.get_name(): (str(obj.get_visibility()), obj.get_render_opacity())
          for obj in u.ObjectIterator(u.Widget) if obj.get_outer() == tree}
white = u.load_asset("/Engine/EngineResources/WhiteSquareTexture")
assert white
for obj in u.ObjectIterator(u.Border):
    if obj.get_outer() != tree:
        continue
    brush = obj.get_editor_property("background").copy()
    if not brush.get_editor_property("resource_object"):
        brush.set_editor_properties(dict(resource_object=white, draw_as=u.SlateBrushDrawType.IMAGE,
                                         image_type=u.SlateBrushImageType.FULL_COLOR))
        obj.set_editor_property("background", brush)
assert before == {obj.get_name(): (str(obj.get_visibility()), obj.get_render_opacity())
                  for obj in u.ObjectIterator(u.Widget) if obj.get_outer() == tree}
assert EDIT.compile_widget(bp)
save(bp)

bp = asset("UI/InventoryContainer/Interaction/WBP_Dual")
tree = u.find_object(bp, "WidgetTree")
root = u.find_object(tree, "RootCanvas")
scroll = u.find_object(tree, "PlayerInventories")
if not scroll:
    scroll = EDIT.add_template(bp, u.ScrollBox, "PlayerInventories")
    slot = root.add_child_to_canvas(scroll)
    slot.set_anchors(u.Anchors(u.Vector2D(1, 0), u.Vector2D(1, 1)))
    slot.set_alignment(u.Vector2D(1, 0))
    slot.set_offsets(u.Margin(-16, 16, 430, 16))
assert EDIT.compile_widget(bp)
save(bp)

bp = asset("UI/Inventory/Workspace/WBP_InventoryWorkspace")
tree = u.find_object(bp, "WidgetTree")
slot = u.find_object(tree, "PlayerSlot")
if not u.find_object(tree, "PlayerInventoryList"):
    panel = EDIT.add_template(bp, u.VerticalBox, "PlayerInventoryList")
    slot.add_child(panel)
assert EDIT.compile_widget(bp)
save(bp)

table = asset("Data/DT_ItemData")
rows_before = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(table))
row = next(row for row in rows_before if row["Name"] == "BackpackSmall")
metadata, count = re.subn(r"(ItemInventoryData=\(Rows=4,Columns=5,MaxWeight=)[\d.]+", r"\g<1>50.000000", row["ItemMetaData"], count=1)
assert count == 1
assert u.InvenzaDataTableEditingLibrary.set_row_from_text(table, "BackpackSmall",
    '(ID="BackpackSmall",ItemMetaData=' + metadata + ')')
rows_after = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(table))
assert [row for row in rows_before if row["Name"] != "BackpackSmall"] == [row for row in rows_after if row["Name"] != "BackpackSmall"]
save(table)

mesh = asset("Meshes/StorageLabels/SM_StorageLabelBoxMedium")
meshes = u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
if meshes.get_simple_collision_count(mesh) == 0 and meshes.get_convex_collision_count(mesh) == 0:
    assert meshes.add_simple_collisions(mesh, u.ScriptingCollisionShapeType.BOX) >= 0
body = mesh.get_editor_property("body_setup")
body.set_editor_property("collision_trace_flag", u.CollisionTraceFlag.CTF_USE_SIMPLE_AND_COMPLEX)
assert meshes.get_simple_collision_count(mesh) > 0
save(mesh)

bp = asset("Blueprints/Examples/BP_Character")
manager = next(obj for obj in u.ObjectIterator(u.IInventoryManager)
    if obj.get_name() == "BP_InvManager_GEN_VARIABLE" and obj.get_path_name().startswith(bp.get_path_name().split(".")[0]+"."))
manager.set_editor_property("ExampleGroundBackpacks", [])
u.BlueprintEditorLibrary.compile_blueprint(bp)
save(bp)

(OUT / "asset-changes.json").write_text(json.dumps(dict(saved=saved, small_backpack_capacity=50,
    common_box_collision=meshes.get_simple_collision_count(mesh),
    existing_widget_visibility_and_opacity_preserved=True), indent=2), encoding="utf-8")
u.log("INVENZA_INTERACTION_ASSETS_FIXED")
u.SystemLibrary.quit_editor()
