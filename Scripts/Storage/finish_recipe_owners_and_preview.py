# Nublin Studio 2026 All Rights Reserved.
"""Update the two approved recipe owners, then render unsaved preview geometry."""
import json
import shutil
import time
import traceback
from pathlib import Path
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir())
OUT = PROJECT / 'Saved/StorageLabelsScoped'
NAMES = ['Bow', 'FieldBow', 'BrewRedPotion', 'SmeltIron', 'CraftSword']
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
editor = u.get_editor_subsystem(u.UnrealEditorSubsystem)
callback = None
started = time.monotonic()
captures = 0


def finish_owners():
    table = u.load_asset(ROOT + 'Data/DT_Recipe')
    handles = [u.DataTableRowHandle(data_table=table, row_name=name) for name in NAMES]
    sub = u.get_engine_subsystem(u.SubobjectDataSubsystem)
    results = []
    for relative in ['Blueprints/Examples/BP_Character', 'Blueprints/Examples/CraftStaton/BP_CraftStation']:
        source = PROJECT / 'Plugins/InventorySystemInvenzaPlugin/Content' / (relative + '.uasset')
        backup = OUT / 'Before' / (relative + '.uasset')
        if not backup.exists():
            backup.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, backup)
        bp = u.load_asset(ROOT + relative)
        bp.modify()
        found = set()
        for handle in sub.k2_gather_subobject_data_for_blueprint(bp):
            component = u.SubobjectDataBlueprintFunctionLibrary.get_object(
                u.SubobjectDataBlueprintFunctionLibrary.get_data(handle))
            if isinstance(component, u.CraftingComponent) and component.get_path_name() not in found:
                found.add(component.get_path_name())
                before = [h.export_text() for h in component.get_editor_property('starting_recipes')]
                component.modify()
                component.set_editor_property('starting_recipes', handles)
                results.append(dict(blueprint=ROOT + relative, before=before, after=NAMES))
        assert found, relative
        u.BlueprintEditorLibrary.compile_blueprint(bp)
        assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False), relative
    (OUT / 'approved-blueprint-changes.json').write_text(json.dumps(results, indent=2), encoding='utf-8')


def setup_preview():
    # A transient blank world keeps the example map out of the preview and out of all saves.
    u.EditorLoadingAndSavingUtils.new_blank_map(False)
    world = editor.get_editor_world()
    table = u.load_asset(ROOT + 'Data/DT_ItemData')
    rows = {row['Name']: row for row in json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(table))}
    names = ['RedPotion', 'YellowPotion', 'Vine', 'Rope', 'Money', 'Paper',
             'Armor_1', 'Sword_1', 'Bow', 'Board', 'Thread', 'IronIngot']
    for i, name in enumerate(names):
        meta = u.ItemMetaData()
        assert meta.import_text(rows[name]['ItemMetaData'])
        mesh = meta.get_editor_property('item_storage_data').get_editor_property('storage_mesh')
        actor = actors.spawn_actor_from_class(u.StaticMeshActor, u.Vector((i % 6) * 105, (i // 6) * 145, 0))
        actor.set_actor_label('Preview_' + name)
        actor.static_mesh_component.set_static_mesh(mesh)
        if name in ['Money', 'Paper']:
            for j in range(1, 4):
                stacked = actors.spawn_actor_from_class(u.StaticMeshActor,
                    actor.get_actor_location() + u.Vector((j % 2) * 22, (j // 2) * 22, 0))
                stacked.static_mesh_component.set_static_mesh(mesh)
    floor = actors.spawn_actor_from_class(u.StaticMeshActor, u.Vector(250, 70, -6))
    floor.static_mesh_component.set_static_mesh(u.load_asset('/Engine/BasicShapes/Cube'))
    floor.set_actor_scale3d(u.Vector(8, 4.5, 0.1))
    floor.static_mesh_component.set_material(0, u.load_asset('/Engine/BasicShapes/BasicShapeMaterial'))
    sun = actors.spawn_actor_from_class(u.DirectionalLight, u.Vector(0, 0, 500), u.Rotator(-55, -35, 0))
    sun.light_component.set_editor_property('intensity', 4.0)
    fill = actors.spawn_actor_from_class(u.DirectionalLight, u.Vector(0, 0, 500), u.Rotator(-35, 140, 0))
    fill.light_component.set_editor_property('intensity', 2.0)
    # Soft preview fill makes the vertical labels legible without editing materials.
    for yaw in [0, 90, 180, 270]:
        soft = actors.spawn_actor_from_class(u.DirectionalLight, u.Vector(0, 0, 500),
            u.Rotator(pitch=-25, yaw=yaw, roll=0))
        soft.light_component.set_editor_properties(dict(intensity=1.8, cast_shadows=False))
    target_position = u.Vector(255, 72, 5)
    position = target_position + u.Vector(150, -620, 660)
    camera = actors.spawn_actor_from_class(u.SceneCapture2D, position,
        u.MathLibrary.find_look_at_rotation(position, target_position))
    capture = camera.get_component_by_class(u.SceneCaptureComponent2D)
    target = u.RenderingLibrary.create_render_target2d(world, 1500, 850, u.TextureRenderTargetFormat.RTF_RGBA8)
    capture.set_editor_properties(dict(texture_target=target, capture_source=u.SceneCaptureSource.SCS_FINAL_COLOR_LDR,
        capture_every_frame=False, capture_on_movement=False, projection_type=u.CameraProjectionMode.ORTHOGRAPHIC,
        ortho_width=730.0))
    settings = capture.get_editor_property('post_process_settings')
    assert settings.import_text('(bOverride_AutoExposureMethod=True,AutoExposureMethod=AEM_Manual,'
        'bOverride_AutoExposureBias=True,AutoExposureBias=0,'
        'bOverride_AutoExposureApplyPhysicalCameraExposure=True,AutoExposureApplyPhysicalCameraExposure=False)')
    capture.set_editor_property('post_process_settings', settings)
    return world, capture, target


def tick(delta):
    global captures
    try:
        if time.monotonic() - started < 18:
            return
        capture.capture_scene()
        captures += 1
        if captures == 4:
            u.RenderingLibrary.export_render_target(world, target, str(OUT), 'storage-labels-unreal.png')
            u.unregister_slate_post_tick_callback(callback)
            u.log('APPROVED_OWNERS_AND_STORAGE_PREVIEW_OK')
            u.SystemLibrary.quit_editor()
    except Exception:
        u.log_error(traceback.format_exc())
        u.unregister_slate_post_tick_callback(callback)
        u.SystemLibrary.quit_editor()


if __name__ == '__main__':
    try:
        finish_owners()
        world, capture, target = setup_preview()
        u.EditorPythonScripting.set_keep_python_script_alive(True)
        callback = u.register_slate_post_tick_callback(tick)
    except Exception:
        u.log_error(traceback.format_exc())
        u.SystemLibrary.quit_editor()
