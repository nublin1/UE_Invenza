"""Run with UnrealEditor-Cmd -run=pythonscript -script=<this file>."""
import unreal
from pathlib import Path

source = Path(__file__).resolve().parent / 'SM_StorageSmallBox.fbx'
destination = '/InventorySystemInvenzaPlugin/Meshes/GridStorage'
asset_path = destination + '/SM_StorageSmallBox'

if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
    mesh = unreal.load_asset(asset_path)
else:
    options = unreal.FbxImportUI()
    options.set_editor_property('import_mesh', True)
    options.set_editor_property('import_as_skeletal', False)
    options.set_editor_property('mesh_type_to_import', unreal.FBXImportType.FBXIT_STATIC_MESH)
    options.set_editor_property('automated_import_should_detect_type', False)
    options.set_editor_property('import_materials', True)
    options.set_editor_property('import_textures', False)
    data = options.static_mesh_import_data
    data.set_editor_property('auto_generate_collision', False)
    data.set_editor_property('generate_lightmap_u_vs', True)
    data.set_editor_property('convert_scene', True)
    data.set_editor_property('convert_scene_unit', True)
    data.set_editor_property('import_uniform_scale', 1.0)

    task = unreal.AssetImportTask()
    task.set_editor_property('filename', str(source))
    task.set_editor_property('destination_path', destination)
    task.set_editor_property('destination_name', 'SM_StorageSmallBox')
    task.set_editor_property('automated', True)
    task.set_editor_property('replace_existing', False)
    task.set_editor_property('save', True)
    task.set_editor_property('factory', unreal.FbxFactory())
    task.set_editor_property('options', options)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.load_asset(asset_path)

assert isinstance(mesh, unreal.StaticMesh), 'Static mesh import failed'
# Make the simple material explicit; FBX material translation varies by exporter.
material_path = destination + '/M_StorageSmallBox'
material = unreal.load_asset(material_path) if unreal.EditorAssetLibrary.does_asset_exist(material_path) else None
if material is None:
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        'M_StorageSmallBox', destination, unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property('used_with_instanced_static_meshes', True)
    color = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, -300, 0)
    color.set_editor_property('constant', unreal.LinearColor(0.35, 0.20, 0.10, 1.0))
    unreal.MaterialEditingLibrary.connect_material_property(color, '', unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant, -300, 160)
    roughness.set_editor_property('r', 0.78)
    unreal.MaterialEditingLibrary.connect_material_property(roughness, '', unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
mesh.set_material(0, material)
assert mesh.get_material(0) == material
bounds = mesh.get_bounds()
extent = bounds.box_extent
dimensions = [extent.x * 2, extent.y * 2, extent.z * 2]
assert all(abs(a-b) < 0.02 for a,b in zip(dimensions, (20,20,25))), dimensions
assert abs(bounds.origin.z - extent.z) < 0.02, 'Pivot must be at bottom'
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
unreal.log('SMALL_BOX_IMPORT_OK: ' + asset_path + ' dimensions_cm=' + str(dimensions))
