#  Nublin Studio 2026 All Rights Reserved.

"""Reload and verify the saved examples, render the compact panel, and capture shelves without saving."""

from pathlib import Path
import json
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
OUT = Path(u.Paths.project_saved_dir())/'ExampleLevelPolish'
level = u.get_editor_subsystem(u.LevelEditorSubsystem)
level.load_level(ROOT+'Maps/Map_Ex')
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
api = u.get_editor_subsystem(u.EditorActorSubsystem)
actors = api.get_all_level_actors()
texts = [str(t.text) for a in actors for t in a.get_components_by_class(u.TextRenderComponent)]
assert sum('F6' in text for text in texts)==1
assert not any('WASD - Movement' in text or 'Double click Primary Interaction' in text for text in texts)
racks = sorted([a for a in actors if a.get_components_by_class(u.InventoryVisualizer)],key=lambda a:a.get_actor_location().x)
assert len(racks)==2
for rack,expected,x in zip(racks,[(0.,1.),(0.,.25,.5,.75,1.)],[1165.,1815.]):
    assert sorted(rack.get_component_by_class(u.InventoryVisualizer).get_editor_property('occupancy_meshes'))==list(expected)
    assert rack.get_actor_location().x==x and rack.get_actor_location().y==400
    assert rack.get_actor_rotation().yaw==0
labels = [t for a in actors for t in a.get_components_by_class(u.TextRenderComponent) if str(t.text).startswith('RACK\n')]
assert len(labels)==2
assert all(abs(t.get_world_location().z-240)<.01 for t in labels)
for actor in actors:
    text = actor.get_component_by_class(u.TextRenderComponent)
    if text and str(text.text).startswith('RACK\n'):
        rotation = actor.get_actor_rotation()
        assert abs(rotation.yaw-90)<.01 and abs(rotation.pitch)<.01 and abs(rotation.roll)<.01, str(rotation)

edit = u.InvenzaWidgetEditingLibrary
widget = edit.create_preview(world,u.load_class(None,ROOT+'UI/World/Craft/WBP_WorldCraftState.WBP_WorldCraftState_C'))
assert edit.render_widget(widget,u.Vector2D(500,220),str(OUT/'craft-panel.png'))
recipe = u.QueuedRecipe()
assert recipe.import_text('(ItemRecipeRow=(ID=Bow,DisplayName="Bow",CraftVolume=5),Count=1,CurrentProgress=3.5)')
row = recipe.get_editor_property('item_recipe_row')
recipes = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(u.load_asset(ROOT+'Data/DT_Recipe')))
for entry in recipes:
    if entry.get('Name')=='Bow' and entry.get('RecipeIcon'):
        row.set_editor_property('recipe_icon',u.load_asset(entry['RecipeIcon']))
recipe.set_editor_property('item_recipe_row',row)
widget.queue_craft_list_entry_widget.update_data(recipe)
widget.queue_craft_list_entry_widget.set_visibility(u.SlateVisibility.HIT_TEST_INVISIBLE)
widget.world_frame.set_visibility(u.SlateVisibility.HIT_TEST_INVISIBLE)
assert edit.render_widget(widget,u.Vector2D(500,220),str(OUT/'craft-panel.png'))
template = u.load_asset(ROOT+'UI/World/Craft/WBP_WorldCraftState')
panel_size = u.find_object(u.find_object(template,'WidgetTree'),'SizeBox_28')
assert abs(panel_size.get_editor_property('width_override')-250)<.01

# Preview examples at different stages in this transient editor world only.
racks[0].get_component_by_class(u.StaticMeshComponent).set_static_mesh(u.load_asset(ROOT+'Meshes/Rack/Rack_Filled_UE'))
racks[1].get_component_by_class(u.StaticMeshComponent).set_static_mesh(u.load_asset(ROOT+'Meshes/Rack/SM_Rack_Fill_50'))
center = u.Vector(1490,400,190)
location = u.Vector(1490,1400,500)
camera = api.spawn_actor_from_class(u.SceneCapture2D,location,u.MathLibrary.find_look_at_rotation(location,center))
capture = camera.get_component_by_class(u.SceneCaptureComponent2D)
target = u.RenderingLibrary.create_render_target2d(world,1280,720,u.TextureRenderTargetFormat.RTF_RGBA8)
capture.set_editor_properties(dict(texture_target=target,capture_source=u.SceneCaptureSource.SCS_FINAL_COLOR_LDR,
    capture_every_frame=False,capture_on_movement=False,fov_angle=60.))
settings = capture.get_editor_property('post_process_settings')
assert settings.import_text('(bOverride_AutoExposureMethod=True,AutoExposureMethod=AEM_Manual,bOverride_AutoExposureBias=True,AutoExposureBias=2,bOverride_AutoExposureApplyPhysicalCameraExposure=True,AutoExposureApplyPhysicalCameraExposure=False)')
capture.set_editor_property('post_process_settings',settings)
frame = 0
callback = None


def tick(delta):
    global frame
    frame += 1
    if frame in (20,21,22):
        assert edit.render_widget(widget,u.Vector2D(500,220),str(OUT/'craft-panel.png'))
        capture.capture_scene()
        u.RenderingLibrary.export_render_target(world,target,str(OUT),'shelves-preview.png')
    elif frame==23:
        u.unregister_slate_post_tick_callback(callback)
        api.destroy_actor(camera)
        edit.release_preview()
        u.log('EXAMPLE_POLISH_VERIFIED: craft panel width=250, wall hint=1, shelf stages=2/5, rack labels aligned')
        u.SystemLibrary.quit_editor()


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_post_tick_callback(tick)
