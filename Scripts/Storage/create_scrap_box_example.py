# Nublin Studio 2026 All Rights Reserved.
"""Create only the requested dismantling example and its localized labels."""
from pathlib import Path
import json
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
FOLDER = ROOT + "Blueprints/Examples/Pickups"
OUT = Path(u.Paths.project_saved_dir()) / "MultiPickup20261028"
EDIT = u.InvenzaWidgetEditingLibrary
TOOLS = u.AssetToolsHelpers.get_asset_tools()
ALLOWED = {FOLDER + "/BP_ScrapBox", FOLDER + "/ST_PickupExamples"}
saved = []


def save(asset):
    path = asset.get_path_name().split(".")[0]
    assert path in ALLOWED
    assert u.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
    saved.append(path)


def create(name, factory):
    path = FOLDER + "/" + name
    # Re-runs update only this task's assets; old pickup Blueprints are never saved.
    if u.EditorAssetLibrary.does_asset_exist(path):
        return u.load_asset(path)
    result = TOOLS.create_asset(name, FOLDER, None, factory)
    assert result, path
    return result


texts = create("ST_PickupExamples", u.StringTableFactory())
for key, value in {
    "ScrapBox.Name": "Коробка с барахлом",
    "ScrapBox.Action": "Разобрать",
    "ScrapBox.ActiveAction": "Разбираем…",
}.items():
    EDIT.set_string_table_entry(texts, key, value)
save(texts)

factory = u.BlueprintFactory()
factory.set_editor_property("parent_class", u.Actor)
bp = create("BP_ScrapBox", factory)
mesh = EDIT.add_blueprint_component(bp, u.StaticMeshComponent, "BoxMesh")
pickup = EDIT.add_blueprint_component(bp, u.PickupComponent, "Pickup")
assert mesh and pickup
box_mesh = u.load_asset(ROOT + "Meshes/StorageLabels/SM_StorageLabelBoxMedium")
assert box_mesh
mesh.set_static_mesh(box_mesh)
mesh.set_editor_property("mobility", u.ComponentMobility.MOVABLE)
mesh.set_collision_profile_name("BlockAllDynamic")
channel = u.get_default_object(u.InvenzaInventoryDeveloperSettings).get_editor_property("interaction_trace_channel")
mesh.set_collision_response_to_channel(channel, u.CollisionResponseType.ECR_BLOCK)
mesh.set_simulate_physics(False)
table = u.load_asset(ROOT + "Data/DT_ItemData")
assert table
rows = set(str(name) for name in u.DataTableFunctionLibrary.get_data_table_row_names(table))
entries = []
for name, amount in [("Board", 2), ("Rope", 1), ("IronIngot", 1)]:
    assert name in rows, name
    entries.append(u.InitItemsEntry(item=u.DataTableRowHandle(data_table=table, row_name=name), amount=amount))
ref = u.ComponentReference()
ref.set_editor_property("component_property", "BoxMesh")
pickup.set_editor_properties({
    "InitialItems": entries,
    "MeshSource": u.PickupMeshSource.CUSTOM_MESH,
    "CustomMesh": box_mesh,
    "CustomMeshScale": u.Vector(1, 1, 1),
    "CustomMaterials": [u.load_asset(ROOT + "Meshes/StorageLabels/M_StorageCardboard")
                        for _ in range(len(box_mesh.get_editor_property("static_materials")))],
    "VisualMeshComponent": ref,
})
# Reuse existing resource geometry as decorative junk inside the box.
for component_name, mesh_path, location, rotation in [
    ("ScrapBoard", "Meshes/Logs/Sm_Log_190x30", u.Vector(-2, -5, 42), u.Rotator(12, 25, 0)),
    ("ScrapRope", "Meshes/StorageLabels/SM_StorageRopeBundle", u.Vector(5, 5, 44), u.Rotator(0, -20, 0)),
]:
    resource_mesh = u.load_asset(ROOT + mesh_path)
    assert resource_mesh
    decoration = EDIT.add_blueprint_component(bp, u.StaticMeshComponent, component_name)
    decoration.set_static_mesh(resource_mesh)
    extent = resource_mesh.get_bounds().box_extent
    scale = 22.0 / max(extent.x * 2, extent.y * 2, extent.z * 2, 1)
    decoration.set_relative_scale3d(u.Vector(scale, scale, scale))
    decoration.set_relative_location(location, False, False)
    decoration.set_relative_rotation(rotation, False, False)
    decoration.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
    decoration.set_editor_property("mobility", u.ComponentMobility.MOVABLE)

data = u.InteractableData()
data.set_editor_properties({
    "default_interactable_type": u.InteractableType.PICKUP,
    "name": EDIT.string_table_text(texts, "ScrapBox.Name"),
    "action": EDIT.string_table_text(texts, "ScrapBox.Action"),
    "active_action": EDIT.string_table_text(texts, "ScrapBox.ActiveAction"),
    "quantity": -1,
    "interactable_duration": 2.0,
    "hold_to_interact": True,
})
pickup.set_editor_property("InteractableDataMap", {u.InteractionType.PRIMARY: data})
assert EDIT.compile_actor_blueprint(bp)
cdo = u.get_default_object(bp.generated_class())
cdo.set_editor_property("replicates", True)
save(bp)
OUT.mkdir(parents=True, exist_ok=True)
(OUT / "created-assets.json").write_text(json.dumps(dict(assets=saved, contents={"Board": 2, "Rope": 1, "IronIngot": 1}, hold_seconds=2), indent=2), encoding="utf-8")
u.log("INVENZA_SCRAP_BOX_CREATED")
if __name__ == "__main__":
    u.SystemLibrary.quit_editor()
