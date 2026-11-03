# Nublin Studio 2026 All Rights Reserved.
"""Render the migrated storage rows in a transient editor world. No assets are saved."""
import importlib.util
import time
import traceback
from pathlib import Path
import unreal as u

OUT = Path(u.Paths.project_saved_dir()) / "ItemDataRefactor20261028"
spec = importlib.util.spec_from_file_location("storage_preview", Path(u.Paths.project_dir()) /
    "Scripts/Storage/finish_recipe_owners_and_preview.py")
preview = importlib.util.module_from_spec(spec)
spec.loader.exec_module(preview)
world, capture, target = preview.setup_preview()
started = time.monotonic()
frames = 0
callback = None


def tick(delta):
    global frames
    try:
        if time.monotonic() - started < 12:
            return
        capture.capture_scene()
        frames += 1
        if frames == 4:
            u.RenderingLibrary.export_render_target(world, target, str(OUT), "shared-box-materials.png")
            u.unregister_slate_post_tick_callback(callback)
            u.log("SHARED_BOX_MATERIAL_PREVIEW_OK")
            u.SystemLibrary.quit_editor()
    except Exception:
        u.log_error(traceback.format_exc())
        u.unregister_slate_post_tick_callback(callback)
        u.SystemLibrary.quit_editor()


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_post_tick_callback(tick)
