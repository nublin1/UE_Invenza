#  Nublin Studio 2026 All Rights Reserved.

"""Read-only PIE trace check and scene capture of the saved stand in Map_Ex."""

from pathlib import Path
import time
import traceback
import unreal as u

OUT = Path(u.Paths.project_saved_dir())/'StorageAccessMesh'
level = u.get_editor_subsystem(u.LevelEditorSubsystem)
editor = u.get_editor_subsystem(u.UnrealEditorSubsystem)
level.load_level('/InventorySystemInvenzaPlugin/Maps/Map_Ex')
editor_actors = u.get_editor_subsystem(u.EditorActorSubsystem)
editor_actor = next(a for a in editor_actors.get_all_level_actors() if isinstance(a,u.StorageAccessPoint))
editor_center = editor_actor.get_actor_location()+u.Vector(0,0,5)
editor_location = editor_center+u.Vector(-155,205,145)
editor_camera = editor_actors.spawn_actor_from_class(u.SceneCapture2D, editor_location,
    u.MathLibrary.find_look_at_rotation(editor_location,editor_center))
editor_camera.set_actor_label('StorageDeskPreviewCamera')
stage = 0
frame = 0
started = time.monotonic()
callback = None
capture = None
target = None
camera = None
world = None


def setup_preview():
    global capture, target, camera, world
    world = editor.get_game_world()
    actor = u.GameplayStatics.get_all_actors_of_class(world, u.StorageAccessPoint)[0]
    assert actor.mesh.static_mesh.get_name() == 'SM_StorageAccessDesk'
    assert str(actor.mesh.get_collision_profile_name()) == 'BlockAll'
    scale = actor.get_actor_scale3d()
    assert all(abs(v-1.) < .001 for v in (scale.x,scale.y,scale.z))
    center = actor.get_actor_location()+u.Vector(0,0,5)
    hit = u.SystemLibrary.line_trace_single(world, center+u.Vector(0,-100,0), center,
        u.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], u.DrawDebugTrace.NONE, True)
    results = hit if isinstance(hit, tuple) else (hit,)
    result = next((r for r in results if isinstance(r,u.HitResult)), None)
    assert result is not None, 'Interaction trace missed the desk: '+str(hit)
    assert actor in result.to_tuple(), 'Interaction trace hit a different actor'
    u.log('STORAGE_DESK_INTERACTION_TRACE_OK')
    camera = next(a for a in u.GameplayStatics.get_all_actors_of_class(world,u.SceneCapture2D)
        if a.get_actor_label() == 'StorageDeskPreviewCamera')
    capture = camera.get_component_by_class(u.SceneCaptureComponent2D)
    target = u.RenderingLibrary.create_render_target2d(world, 900, 900, u.TextureRenderTargetFormat.RTF_RGBA8)
    capture.set_editor_properties(dict(texture_target=target, capture_source=u.SceneCaptureSource.SCS_FINAL_COLOR_LDR,
        capture_every_frame=False, capture_on_movement=False, fov_angle=45.))
    settings = capture.get_editor_property('post_process_settings')
    assert settings.import_text('(bOverride_AutoExposureMethod=True,AutoExposureMethod=AEM_Manual,bOverride_AutoExposureBias=True,AutoExposureBias=0,bOverride_AutoExposureApplyPhysicalCameraExposure=True,AutoExposureApplyPhysicalCameraExposure=False)')
    capture.set_editor_property('post_process_settings',settings)


def tick(delta):
    global stage, frame
    try:
        if stage == 0:
            if time.monotonic()-started < 3.: return
            assert editor.get_game_world(), 'PIE world not ready'
            setup_preview()
            stage = 1
        elif stage == 1:
            frame += 1
            if frame in (20,21,22):
                capture.capture_scene()
                u.RenderingLibrary.export_render_target(world,target,str(OUT),'unreal-preview.png')
            elif frame == 23:
                u.log('STORAGE_DESK_PREVIEW_OK')
                level.editor_request_end_play()
                stage = 2
        elif stage == 2 and not editor.get_game_world():
            u.unregister_slate_post_tick_callback(callback)
            editor_actors.destroy_actor(editor_camera)
            u.SystemLibrary.quit_editor()
    except Exception:
        u.log_error(traceback.format_exc())
        level.editor_request_end_play()
        stage = 2


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_post_tick_callback(tick)
level.editor_play_simulate()
