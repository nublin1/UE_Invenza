# Nublin Studio 2026 All Rights Reserved.
"""Render the new example in a transient editor map without saving assets."""
from pathlib import Path
import json
import time
import traceback
import unreal as u

OUT = Path(u.Paths.project_saved_dir()) / "MultiPickup20261028"
if "-AuthorScrapBox" in u.SystemLibrary.get_command_line():
    import runpy
    runpy.run_path(str(Path(u.Paths.project_dir()) / "Scripts/Storage/create_scrap_box_example.py"), run_name="invenza_scrap_author")
u.EditorLoadingAndSavingUtils.new_blank_map(False)
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
cls = u.load_asset("/InventorySystemInvenzaPlugin/Blueprints/Examples/Pickups/BP_ScrapBox").generated_class()
box = actors.spawn_actor_from_class(cls, u.Vector(0, 0, 0))
components = []
for mesh in box.get_components_by_class(u.StaticMeshComponent):
    bounds = mesh.static_mesh.get_bounds()
    components.append(dict(name=mesh.get_name(), mesh=mesh.static_mesh.get_path_name(), extent=str(bounds.box_extent),
                           position=str(mesh.get_editor_property("relative_location")), scale=str(mesh.get_editor_property("relative_scale3d"))))
(OUT / "example-components.json").write_text(json.dumps(components, indent=2), encoding="utf-8")
floor = actors.spawn_actor_from_class(u.StaticMeshActor, u.Vector(0, 0, -3))
floor.static_mesh_component.set_static_mesh(u.load_asset("/Engine/BasicShapes/Cube"))
floor.set_actor_scale3d(u.Vector(2, 2, 0.05))
floor.static_mesh_component.set_material(0, u.load_asset("/Engine/BasicShapes/BasicShapeMaterial"))
for rotation, intensity in [(u.Rotator(-55, -30, 0), 4.0), (u.Rotator(-35, 140, 0), 2.0)]:
    light = actors.spawn_actor_from_class(u.DirectionalLight, u.Vector(0, 0, 200), rotation)
    light.light_component.set_editor_property("intensity", intensity)
for yaw in [0, 90, 180, 270]:
    light = actors.spawn_actor_from_class(u.DirectionalLight, u.Vector(0, 0, 200), u.Rotator(-25, yaw, 0))
    light.light_component.set_editor_properties(dict(intensity=1.8, cast_shadows=False))
position = u.Vector(80, -100, 95)
camera = actors.spawn_actor_from_class(u.SceneCapture2D, position,
    u.MathLibrary.find_look_at_rotation(position, u.Vector(0, 0, 14)))
capture = camera.get_component_by_class(u.SceneCaptureComponent2D)
target = u.RenderingLibrary.create_render_target2d(world, 800, 600, u.TextureRenderTargetFormat.RTF_RGBA8)
capture.set_editor_properties(dict(texture_target=target, capture_source=u.SceneCaptureSource.SCS_FINAL_COLOR_LDR,
    capture_every_frame=False, capture_on_movement=False, fov_angle=35))
post = capture.get_editor_property("post_process_settings")
assert post.import_text("(bOverride_AutoExposureMethod=True,AutoExposureMethod=AEM_Manual,"
    "bOverride_AutoExposureBias=True,AutoExposureBias=0,"
    "bOverride_AutoExposureApplyPhysicalCameraExposure=True,AutoExposureApplyPhysicalCameraExposure=False)")
capture.set_editor_property("post_process_settings", post)
started = time.monotonic()
frames = 0
callback = None


def tick(delta):
    global frames
    try:
        if time.monotonic() - started < 8:
            return
        capture.capture_scene()
        frames += 1
        if frames >= 3:
            u.RenderingLibrary.export_render_target(world, target, str(OUT), "scrap-box.png")
            u.log("INVENZA_SCRAP_BOX_PREVIEW_OK")
            u.unregister_slate_post_tick_callback(callback)
            u.SystemLibrary.quit_editor()
    except Exception:
        u.log_error(traceback.format_exc())
        u.unregister_slate_post_tick_callback(callback)
        u.SystemLibrary.quit_editor()


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_post_tick_callback(tick)
