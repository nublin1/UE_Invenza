# Nublin Studio 2026 All Rights Reserved.
"""Read-only asset diagnostics for the inventory interaction fixes."""
from pathlib import Path
import json
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
OUT = Path(u.Paths.project_saved_dir()) / "InventoryInteraction20261028"
result = {}
for relative in ["UI/InventoryContainer/WBP_InvContainer", "UI/InventoryContainer/Interaction/WBP_Dual",
                 "UI/Inventory/Workspace/WBP_InventoryWorkspace"]:
    if not u.EditorAssetLibrary.does_asset_exist(ROOT + relative):
        continue
    bp = u.load_asset(ROOT + relative)
    tree = u.find_object(bp, "WidgetTree")
    nodes = []
    for obj in u.ObjectIterator(u.Widget):
        if obj.get_outer() != tree:
            continue
        row = dict(name=obj.get_name(), cls=obj.get_class().get_name(),
                   parent=obj.get_parent().get_name() if obj.get_parent() else None,
                   visibility=str(obj.get_visibility()), opacity=obj.get_render_opacity())
        if isinstance(obj, u.Image):
            row["brush"] = obj.get_editor_property("brush").export_text()
        if isinstance(obj, u.Border):
            row["background"] = obj.get_editor_property("background").export_text()
            row["brush_color"] = str(obj.get_editor_property("brush_color"))
        if obj.slot:
            row["slot"] = obj.slot.export_text() if hasattr(obj.slot, "export_text") else str(obj.slot)
        nodes.append(row)
    result[relative] = nodes
settings = u.load_asset(ROOT + "Data/Settings/InvenzaSettings")
result["pickup_class"] = settings.get_editor_property("PickupClass").get_path_name()
pickup = u.get_default_object(settings.get_editor_property("PickupClass"))
result["pickup_components"] = []
for obj in u.ObjectIterator(u.StaticMeshComponent):
    if obj.get_outer() == pickup or obj.get_path_name().startswith(pickup.get_path_name()):
        result["pickup_components"].append(dict(path=obj.get_path_name(), collision=str(obj.get_collision_enabled()),
            profile=str(obj.get_collision_profile_name()), simulate=obj.is_simulating_physics()))
meshes = u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
result["mesh_collision"] = {}
for name in ["SM_StorageLabelBoxMedium", "SM_StorageLabelBoxSmall", "SM_StorageLabelBoxLarge"]:
    mesh = u.load_asset(ROOT + "Meshes/StorageLabels/" + name)
    if not mesh:
        continue
    result["mesh_collision"][name] = dict(simple=meshes.get_simple_collision_count(mesh),
        convex=meshes.get_convex_collision_count(mesh))
character = u.load_asset(ROOT + "Blueprints/Examples/BP_Character")
manager = next(obj for obj in u.ObjectIterator(u.IInventoryManager)
    if obj.get_name() == "BP_InvManager_GEN_VARIABLE" and obj.get_path_name().startswith(character.get_path_name().split(".")[0]+"."))
result["startup"] = [item.export_text() for item in manager.get_editor_property("StartupInventories")]
result["ground_examples"] = [item.export_text() for item in manager.get_editor_property("ExampleGroundBackpacks")]
table = u.load_asset(ROOT + "Data/DT_ItemData")
result["rows"] = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(table))
result["workspace_assets"] = [p for p in u.EditorAssetLibrary.list_assets(ROOT+"UI", recursive=True) if "Workspace" in p]
(OUT / "inspection.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
u.log("INVENZA_INTERACTION_INSPECTED")

u.SystemLibrary.quit_editor()

