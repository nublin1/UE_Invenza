"""Import the BulkBins example and seed a separate demo storage in Map_Ex."""
from pathlib import Path
import unreal as u
import json
import shutil

ROOT = '/InventorySystemInvenzaPlugin/'
DEST = ROOT + 'Meshes/GridStorage/BulkBins'
project = Path(u.Paths.project_dir())
source = project / 'ArtSource/StorageGrid/BulkBins'
saved = project / 'Saved/BulkBins'
saved.mkdir(parents=True, exist_ok=True)
tools = u.AssetToolsHelpers.get_asset_tools()

def import_file(filename, options=None):
    path = DEST + '/' + Path(filename).stem
    existing_asset = u.load_asset(path)
    if existing_asset:
        return existing_asset
    task = u.AssetImportTask()
    task.set_editor_property('filename', str(source / filename))
    task.set_editor_property('destination_path', DEST)
    task.set_editor_property('automated', True)
    task.set_editor_property('save', True)
    if options: task.set_editor_property('options', options)
    tools.import_asset_tasks([task])
    result = u.load_asset(path)
    assert result, filename
    return result

options = u.FbxImportUI()
options.set_editor_property('import_mesh', True)
options.set_editor_property('import_as_skeletal', False)
options.set_editor_property('import_materials', True)
options.set_editor_property('import_textures', False)
options.set_editor_property('automated_import_should_detect_type', False)
options.set_editor_property('mesh_type_to_import', u.FBXImportType.FBXIT_STATIC_MESH)
options.static_mesh_import_data.set_editor_property('combine_meshes', True)
options.static_mesh_import_data.set_editor_property('auto_generate_collision', False)
mesh = import_file('SM_BulkIronOre.fbx', options)
icon = import_file('T_BulkIronOre.png')
icon.set_editor_property('lod_group', u.TextureGroup.TEXTUREGROUP_UI)
u.EditorAssetLibrary.save_loaded_asset(icon)
for slot in mesh.get_editor_property('static_materials'):
    mat = slot.material_interface
    if isinstance(mat, u.Material):
        mat.set_editor_property('used_with_instanced_static_meshes', True)
        u.MaterialEditingLibrary.recompile_material(mat)
        u.EditorAssetLibrary.save_loaded_asset(mat)

table = u.load_asset(ROOT + 'Data/DT_ItemData')
backup = saved / 'DT_ItemData.before_bulk_bins.uasset'
if not backup.exists(): shutil.copy2(project/'Plugins/InventorySystemInvenzaPlugin/Content/Data/DT_ItemData.uasset', backup)
before = u.DataTableFunctionLibrary.export_data_table_to_json_string(table)
(saved/'items_before_setup.json').write_text(before, encoding='utf-8')
row = u.ItemData()
row.set_editor_property('id', 'IronOreBulk')
meta = row.get_editor_property('item_meta_data')
assets = meta.get_editor_property('item_asset_data')
assets.set_editor_property('mesh', mesh)
assets.set_editor_property('icon', icon)
meta.set_editor_property('item_asset_data', assets)
texts = meta.get_editor_property('item_text_data')
texts.set_editor_property('display_name', u.Text('Iron Ore'))
texts.set_editor_property('item_description', u.Text('Bulk storage example. A 2x2 bin fills as its stack approaches 100 units. Split, move or withdraw ore to change the pile.'))
meta.set_editor_property('item_text_data', texts)
stats = meta.get_editor_property('item_numeratic_data')
for key,value in [('weight',0.5),('inventory_horizontal_slots',2),('inventory_vertical_slots',2),('max_stack_size_in_character',100)]:
    stats.set_editor_property(key,value)
meta.set_editor_property('item_numeratic_data',stats)
storage = meta.get_editor_property('item_storage_data')
storage.set_editor_property('storage_method',u.StorageMethod.BULK_BINS)
storage.set_editor_property('storage_mesh',mesh)
storage.set_editor_property('bulk_fill_height',65.0)
storage.set_editor_property('bulk_chunk_size',25.0)
meta.set_editor_property('item_storage_data',storage)
# Copy the known resource category without depending on GameplayTag construction in Python.
tag = u.GameplayTag()
tag.import_text('(TagName="Item.Category.Ingredient")')
meta.set_editor_property('item_category',tag)
row.set_editor_property('item_meta_data',meta)
assert u.InvenzaDataTableEditingLibrary.set_row_from_text(table,'IronOreBulk',row.export_text())
assert 'IronOreBulk' in [str(n) for n in u.DataTableFunctionLibrary.get_data_table_row_names(table)]
u.EditorAssetLibrary.save_loaded_asset(table)

level = u.get_editor_subsystem(u.LevelEditorSubsystem)
actors_api = u.get_editor_subsystem(u.EditorActorSubsystem)
level.load_level(ROOT+'Maps/Map_Ex')
actors = actors_api.get_all_level_actors()
(saved/'actors.txt').write_text('\n'.join(f'{a.get_actor_label()} {a.get_class().get_name()} {a.get_actor_location()}' for a in actors),encoding='utf-8')
existing = next((a for a in actors if a.get_actor_label()=='Bulk Bins - Iron Ore'),None)
if existing:
    demo = existing
else:
    original = next(a for a in actors if a.get_class().get_name()=='BP_Grid_C')
    demo = actors_api.spawn_actor_from_class(original.get_class(),u.Vector(1450,1050,0))
    original_container = original.get_component_by_class(u.ContainerComponent)
    new_container = demo.get_component_by_class(u.ContainerComponent)
    new_container.set_editor_property('startup_inventories',original_container.get_editor_property('startup_inventories'))
    new_container.set_editor_property('main_loot_container_inv_tag',original_container.get_editor_property('main_loot_container_inv_tag'))
    demo.set_actor_label('Bulk Bins - Iron Ore')
    demo.set_actor_location(u.Vector(1450,1050,0),False,False)
    demo.set_actor_rotation(u.Rotator(0,0,0),False)
container = demo.get_component_by_class(u.ContainerComponent)
startup = list(container.get_editor_property('startup_inventories'))
assert startup
entry = startup[0]
settings = entry.get_editor_property('settings')
settings.set_editor_property('container_title',u.Text('Bulk Bins - Iron Ore'))
settings.set_editor_property('custom_inventory_id','')
settings.set_editor_property('collect_inv_data_from_widget',False)
gridsettings = settings.get_editor_property('inventory_slot_based_settings')
gridsettings.set_editor_property('init_number_rows',4)
gridsettings.set_editor_property('init_num_columns',4)
gridsettings.set_editor_property('ignore_item_size',False)
settings.set_editor_property('inventory_slot_based_settings',gridsettings)
entry.set_editor_property('settings',settings)
item = u.InitItemsEntry()
handle = u.DataTableRowHandle()
handle.set_editor_property('data_table',table)
handle.set_editor_property('row_name','IronOreBulk')
item.set_editor_property('item',handle)
item.set_editor_property('amount',145)
entry.set_editor_property('start_items',[item])
container.set_editor_property('startup_inventories',[entry])
grid = demo.get_component_by_class(u.StorageGridVisualizer)
grid.set_editor_property('preview_grid_size',u.IntPoint(4,4))
grid.set_editor_property('bulk_wall_height',48.0)
grid.set_editor_property('bulk_wall_thickness',5.0)
grid.set_editor_property('bulk_wall_material',u.load_asset(ROOT+'Meshes/GridStorage/M_StorageSmallBox'))
grid.rebuild_grid()
sign = next((a for a in actors if a.get_actor_label()=='Bulk Bins - Sign'),None)
if not sign:
    sign = actors_api.spawn_actor_from_class(u.TextRenderActor,u.Vector(1430,1250,170),u.Rotator(pitch=0,yaw=180,roll=0))
    sign.set_actor_label('Bulk Bins - Sign')
sign.set_actor_rotation(u.Rotator(pitch=0,yaw=180,roll=0),False)
text = sign.get_component_by_class(u.TextRenderComponent)
text.set_text(u.Text('BULK BINS\nIRON ORE  /  145 UNITS'))
text.set_world_size(24.0)
text.set_horizontal_alignment(u.HorizTextAligment.EHTA_CENTER)
text.set_text_render_color(u.Color(230,201,153,255))
assert level.save_current_level()
after = u.DataTableFunctionLibrary.export_data_table_to_json_string(table)
previous = {r['Name']:r for r in json.loads(before) if r['Name']!='IronOreBulk'}
current = {r['Name']:r for r in json.loads(after) if r['Name']!='IronOreBulk'}
assert previous==current, 'Existing data table rows changed'
(saved/'setup_result.json').write_text(json.dumps({'row':'IronOreBulk','amount':145,'actor':demo.get_path_name(),'location':[1450,1050,0],'existing_rows_preserved':True},indent=2),encoding='utf-8')
u.log('BULK_SETUP_DONE')
