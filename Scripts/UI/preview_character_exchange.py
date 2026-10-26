#  Nublin Studio 2026 All Rights Reserved.

"""Render the native character exchange window with existing Core/inventory templates. No asset saves."""

from pathlib import Path
import traceback
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
OUT = Path(u.Paths.project_saved_dir())/'CharacterExchange'
OUT.mkdir(parents=True, exist_ok=True)
editor = u.get_editor_subsystem(u.UnrealEditorSubsystem)
world = editor.get_editor_world()
edit = u.InvenzaWidgetEditingLibrary
window = edit.create_preview(world, u.load_class(None, ROOT+'UI/Inventory/Character/WBP_CharacterInventoryExchange.WBP_CharacterInventoryExchange_C'))
containers = []
for index, title in enumerate(['Companion equipment', 'Companion inventory', 'Player equipment', 'Player inventory']):
    container = edit.create_preview(world, u.load_class(None, ROOT+'UI/InventoryContainer/WBP_InvContainer.WBP_InvContainer_C'))
    container.set_editor_property('title', u.Text(title))
    container.set_editor_property('is_show_total_money', False)
    container.set_editor_property('is_show_weight', False)
    path = 'UI/Core/SimpleInvGrid/WBP_EqGrid' if index % 2 == 0 else 'UI/Core/SimpleInvGrid/WBP_SimpleGrid_5x4'
    grid = edit.create_preview(world, u.load_class(None, ROOT+path+'.'+path.rsplit('/',1)[1]+'_C'))
    container.get_editor_property('container_slot').add_child(grid)
    containers.append(container)
window.set_inventories(u.Text('Companion'), containers[:2], containers[2:])
assert edit.render_widget(window, u.Vector2D(1280,800), str(OUT/'exchange.png'))
frame = 0
callback = None


def tick(delta):
    global frame
    try:
        frame += 1
        if frame < 25:
            return
        assert edit.render_widget(window, u.Vector2D(1280,800), str(OUT/'exchange.png'))
        u.log('CHARACTER_EXCHANGE_PREVIEW_OK')
        window.clear_inventories()
        edit.release_preview()
    except Exception:
        u.log_error(traceback.format_exc())
    u.unregister_slate_post_tick_callback(callback)
    u.SystemLibrary.quit_editor()


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_post_tick_callback(tick)
