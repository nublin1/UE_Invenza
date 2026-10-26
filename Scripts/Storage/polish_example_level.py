#  Nublin Studio 2026 All Rights Reserved.

"""Update Map_Ex examples and only the explicitly requested world craft Widget Blueprint.

Run via -ExecutePythonScript in the regular editor. Backups are in Saved/ExampleLevelPolish/Before.
"""

from pathlib import Path
import json
import shutil
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir()).resolve()
OUT = PROJECT/'Saved/ExampleLevelPolish'
DEST = ROOT+'Meshes/Rack'
registry = u.AssetRegistryHelpers.get_asset_registry()
registry.scan_paths_synchronous([ROOT],force_rescan=True)
mesh_editor = u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
assert mesh_editor, 'Use the regular editor, not -run=pythonscript.'


def backup(relative):
    original = PROJECT/'Plugins/InventorySystemInvenzaPlugin/Content'/relative
    target = OUT/'Before'/relative
    if not target.exists():
        target.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(original,target)


# The partial meshes share the original frame, origin, collision and material assets.
empty = u.load_asset(DEST+'/Rack_UE')
full = u.load_asset(DEST+'/Rack_Filled_UE')
materials = {str(s.material_slot_name):s.material_interface for s in full.static_materials}
variants = {}
for percent in (25,50,75):
    name = 'SM_Rack_Fill_'+str(percent)
    path = DEST+'/'+name
    if not u.EditorAssetLibrary.does_asset_exist(path):
        options = u.FbxImportUI()
        options.set_editor_properties(dict(import_mesh=True,import_as_skeletal=False,import_materials=False,
            import_textures=False,automated_import_should_detect_type=False,mesh_type_to_import=u.FBXImportType.FBXIT_STATIC_MESH))
        options.static_mesh_import_data.set_editor_properties(dict(combine_meshes=True,auto_generate_collision=False,
            generate_lightmap_u_vs=True,convert_scene=True,convert_scene_unit=True,import_uniform_scale=1.))
        task = u.AssetImportTask()
        task.set_editor_properties(dict(filename=str(PROJECT/'ArtSource/RackFill'/(name+'.fbx')),
            destination_path=DEST,destination_name=name,automated=True,replace_existing=False,save=False,
            factory=u.FbxFactory(),options=options))
        u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = u.load_asset(path)
    assert isinstance(mesh,u.StaticMesh),name
    for i,slot in enumerate(mesh.static_materials):
        assert str(slot.material_slot_name) in materials,str(slot.material_slot_name)
        mesh.set_material(i,materials[str(slot.material_slot_name)])
    bounds = mesh.get_bounds()
    assert all(abs(actual-expected)<.05 for actual,expected in
        zip((bounds.box_extent.x*2,bounds.box_extent.y*2,bounds.box_extent.z*2),(300,80,200))),str(bounds)
    assert abs(bounds.origin.z-100)<.05,str(bounds)
    assert mesh_editor.get_convex_collision_count(mesh)==1,name
    assert u.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=False)
    variants[percent/100.] = mesh

# Center a 250-unit panel inside the station's existing 500-unit widget surface.
# A SizeBox at the root alone is stretched by WidgetComponent and cannot limit its width.
world_path = 'UI/World/Craft/WBP_WorldCraftState'
backup(world_path+'.uasset')
bp = u.load_asset(ROOT+world_path)
bp.modify()
edit = u.InvenzaWidgetEditingLibrary
tree = u.find_object(bp,'WidgetTree')
size = u.find_object(tree,'SizeBox_28')
assert size
size.modify()
size.set_width_override(250)
layout = edit.add_template(bp,u.Overlay,'WorldCraftLayout')
layout.modify()
layout.clear_children()
size.remove_from_parent()
slot = layout.add_child_to_overlay(size)
slot.set_horizontal_alignment(u.HorizontalAlignment.H_ALIGN_CENTER)
slot.set_vertical_alignment(u.VerticalAlignment.V_ALIGN_CENTER)
edit.set_root(bp,layout)
assert edit.compile_widget(bp)
assert u.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)

level = u.get_editor_subsystem(u.LevelEditorSubsystem)
assert level.load_level(ROOT+'Maps/Map_Ex')
actors_api = u.get_editor_subsystem(u.EditorActorSubsystem)
actors = actors_api.get_all_level_actors()
racks = sorted([a for a in actors if a.get_components_by_class(u.InventoryVisualizer)],key=lambda a:a.get_actor_location().x)
assert len(racks)==2,[a.get_actor_label() for a in racks]
positions_before = [(a.get_actor_location(),a.get_actor_rotation(),a.get_actor_scale3d()) for a in racks]
for index,rack in enumerate(racks):
    rack.modify()
    visualizer = rack.get_component_by_class(u.InventoryVisualizer)
    visualizer.modify()
    thresholds = {0.:empty,1.:full}
    if index==1: thresholds.update(variants)
    visualizer.set_editor_property('occupancy_meshes',thresholds)

text_actors = [a for a in actors if a.get_components_by_class(u.TextRenderComponent)]
wall_controls = next(a for a in text_actors if 'WASD - Movement' in str(a.get_component_by_class(u.TextRenderComponent).text))
old_shortcuts = next(a for a in text_actors if 'Double click Primary Interaction' in str(a.get_component_by_class(u.TextRenderComponent).text))


def set_label(actor,text,location,size=20):
    actor.modify()
    text_component = actor.get_component_by_class(u.TextRenderComponent)
    text_component.modify()
    text_component.set_text(u.Text(text))
    text_component.set_world_size(size)
    text_component.set_horizontal_alignment(u.HorizTextAligment.EHTA_CENTER)
    actor.set_actor_location(location,False,False)
    return text_component


# Keep ASCII text to match the level's existing distance-field font.
set_label(wall_controls,'Press F6 for controls help',u.Vector(2898.72,1000,330),36)
wall_controls.set_actor_label('Controls help - F6')
assert actors_api.destroy_actor(old_shortcuts)
left_label = next(a for a in text_actors if str(a.get_component_by_class(u.TextRenderComponent).text)=='Press to Open')
right_label = next(a for a in text_actors if str(a.get_component_by_class(u.TextRenderComponent).text)=='Delete than empty')
for actor,rack,text in [(left_label,racks[0],'RACK\n0% / 100%'),
                        (right_label,racks[1],'RACK\n0 / 25 / 50 / 75 / 100%')]:
    center,extent = rack.get_actor_bounds(False)
    set_label(actor,text,u.Vector(center.x,center.y+extent.y+10,240),20)
    actor.set_actor_rotation(u.Rotator(pitch=0,yaw=90,roll=0),False)
group = next(a for a in text_actors if str(a.get_component_by_class(u.TextRenderComponent).text)=='Containets')
set_label(group,'Containers',u.Vector((racks[0].get_actor_location().x+racks[1].get_actor_location().x)/2,450,340),28)
hold = next(a for a in text_actors if str(a.get_component_by_class(u.TextRenderComponent).text)=='Hold to Open')
set_label(hold,'Hold to open',u.Vector(1500,450,180),20)
for rack,(location,rotation,scale) in zip(racks,positions_before):
    assert rack.get_actor_location()==location
    assert rack.get_actor_rotation()==rotation
    assert rack.get_actor_scale3d()==scale
backup('Maps/Map_Ex.umap')
assert level.save_current_level()
report = dict(controls='Press F6 for controls help',world_craft_panel_width=250,
    racks=[dict(label=a.get_actor_label(),thresholds=sorted(a.get_component_by_class(u.InventoryVisualizer).get_editor_property('occupancy_meshes').keys())) for a in racks])
(OUT/'result.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
u.log('EXAMPLE_LEVEL_POLISH_SAVED: '+json.dumps(report))
