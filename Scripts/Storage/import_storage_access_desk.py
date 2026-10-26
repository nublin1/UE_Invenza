#  Nublin Studio 2026 All Rights Reserved.

"""Import the clerk stand and update only BP_StorageAccessPoint.

Run in the regular editor via -ExecutePythonScript, not as a commandlet.
The existing level, interaction settings and other Blueprint assets are not saved.
"""

from pathlib import Path
import json
import shutil
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
DEST = ROOT + 'Meshes/StorageAccess'
PROJECT = Path(u.Paths.project_dir()).resolve()
OUT = PROJECT / 'Saved/StorageAccessMesh'
OUT.mkdir(parents=True, exist_ok=True)
u.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([ROOT], force_rescan=True)
tools = u.AssetToolsHelpers.get_asset_tools()
meshes = u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
assert meshes, 'Run via -ExecutePythonScript in the regular editor, not -run=pythonscript.'
mesh_path = DEST + '/SM_StorageAccessDesk'

if not u.EditorAssetLibrary.does_asset_exist(mesh_path):
    options = u.FbxImportUI()
    options.set_editor_properties(dict(import_mesh=True, import_as_skeletal=False,
        import_materials=False, import_textures=False, automated_import_should_detect_type=False,
        mesh_type_to_import=u.FBXImportType.FBXIT_STATIC_MESH))
    data = options.static_mesh_import_data
    data.set_editor_properties(dict(combine_meshes=True, auto_generate_collision=False,
        generate_lightmap_u_vs=True, convert_scene=True, convert_scene_unit=True,
        import_uniform_scale=1., normal_import_method=u.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS))
    task = u.AssetImportTask()
    task.set_editor_properties(dict(filename=str(PROJECT/'ArtSource/StorageAccess/SM_StorageAccessDesk.fbx'),
        destination_path=DEST, destination_name='SM_StorageAccessDesk', automated=True,
        replace_existing=False, save=False, factory=u.FbxFactory(), options=options))
    tools.import_asset_tasks([task])
mesh = u.load_asset(mesh_path)
assert isinstance(mesh, u.StaticMesh), 'Stand mesh import failed'

palette = {
    'M_StorageDesk_Wood': ((.24,.12,.052), 0., .80),
    'M_StorageDesk_Frame': ((.085,.039,.018), 0., .80),
    'M_StorageDesk_Paper': ((.68,.57,.37), 0., .95),
    'M_StorageDesk_Metal': ((.20,.16,.095), .65, .62),
    'M_StorageDesk_Ledger': ((.12,.032,.022), 0., .90),
}
materials = {}
for name, (rgb, metallic, roughness) in palette.items():
    path = DEST + '/' + name
    mat = u.load_asset(path) if u.EditorAssetLibrary.does_asset_exist(path) else None
    if mat is None:
        mat = tools.create_asset(name, DEST, u.Material, u.MaterialFactoryNew())
        color = u.MaterialEditingLibrary.create_material_expression(mat, u.MaterialExpressionConstant3Vector, -250, 0)
        color.set_editor_property('constant', u.LinearColor(*rgb, 1.))
        u.MaterialEditingLibrary.connect_material_property(color, '', u.MaterialProperty.MP_BASE_COLOR)
        for y, value, prop in [(160,roughness,u.MaterialProperty.MP_ROUGHNESS), (300,metallic,u.MaterialProperty.MP_METALLIC)]:
            scalar = u.MaterialEditingLibrary.create_material_expression(mat, u.MaterialExpressionConstant, -250, y)
            scalar.set_editor_property('r', value)
            u.MaterialEditingLibrary.connect_material_property(scalar, '', prop)
        u.MaterialEditingLibrary.recompile_material(mat)
        assert u.EditorAssetLibrary.save_loaded_asset(mat, only_if_is_dirty=False)
    materials[name] = mat

slots = list(mesh.get_editor_property('static_materials'))
assert len(slots) == len(materials), 'Unexpected material sections: ' + str(slots)
for index, slot in enumerate(slots):
    name = str(slot.get_editor_property('imported_material_slot_name'))
    if name not in materials:
        name = str(slot.material_slot_name)
    assert name in materials, name
    mesh.set_material(index, materials[name])
mesh.set_editor_property('light_map_resolution', 64)
assert u.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
bounds = mesh.get_bounds()
dimensions = [bounds.box_extent.x*2, bounds.box_extent.y*2, bounds.box_extent.z*2]
assert abs(dimensions[0]-80.) < .1 and abs(dimensions[1]-54.739) < .1 and abs(dimensions[2]-109.4425) < .1, dimensions
assert abs(bounds.origin.z-bounds.box_extent.z+50.) < .1, 'Incorrect replacement pivot'
assert meshes.get_convex_collision_count(mesh) == 1, 'Missing interaction/movement collision'

bp_path = 'Blueprints/Examples/Storage/BP_StorageAccessPoint'
source = PROJECT/'Plugins/InventorySystemInvenzaPlugin/Content'/(bp_path+'.uasset')
backup = OUT/'BP_StorageAccessPoint.before-desk.uasset'
if not backup.exists(): shutil.copy2(source, backup)
bp = u.load_asset(ROOT + bp_path)
bp.modify()
cdo = u.get_default_object(bp.generated_class())
old_window = cdo.storage_access.window_class.get_path_name()
old_collision = cdo.mesh.get_collision_profile_name()
cdo.mesh.modify()
assert cdo.mesh.set_static_mesh(mesh)
cdo.mesh.set_editor_property('override_materials', [])
cdo.mesh.set_relative_scale3d(u.Vector(1,1,1))
u.BlueprintEditorLibrary.compile_blueprint(bp)
cdo = u.get_default_object(bp.generated_class())
assert cdo.mesh.static_mesh == mesh, 'Mesh default lost on Blueprint compile'
assert cdo.mesh.get_collision_profile_name() == old_collision
assert cdo.storage_access.window_class.get_path_name() == old_window
assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)

# Reload the example level to check how the changed defaults affect existing instances.
u.get_editor_subsystem(u.LevelEditorSubsystem).load_level(ROOT + 'Maps/Map_Ex')
actors = u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors()
instances = []
for actor in actors:
    if isinstance(actor, u.StorageAccessPoint):
        assert actor.mesh.static_mesh == mesh
        instances.append(dict(name=actor.get_actor_label(), location=str(actor.get_actor_location()),
            scale=str(actor.get_actor_scale3d()), bounds=str(actor.get_actor_bounds(False)),
            collision_profile=str(actor.mesh.get_collision_profile_name())))
report = dict(mesh=mesh_path, dimensions_cm=dimensions, material_slots=len(slots),
    collision_hulls=meshes.get_convex_collision_count(mesh), instances=instances)
(OUT/'import-report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
u.log('STORAGE_ACCESS_DESK_IMPORTED: ' + json.dumps(report))
