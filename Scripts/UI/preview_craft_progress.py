#  Nublin Studio 2026 All Rights Reserved.

"""Read-only, offscreen preview of the actual crafting widgets and all display modes."""

from pathlib import Path
import json
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
EDIT = u.InvenzaWidgetEditingLibrary
OUT = Path(u.Paths.project_saved_dir()) / 'CraftProgress'
OUT.mkdir(parents=True, exist_ok=True)
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()


def preview(path):
    return EDIT.create_preview(world, u.load_class(None, ROOT + path + '.' + path.split('/')[-1] + '_C'))


row = preview('UI/Craft/Lists/WBP_QueueCraftList_Dashboard')
station = preview('UI/World/Craft/WBP_WorldCraftState')
canvas = preview('UI/Core/WBP_Border4SideWithSlot')
body = u.new_object(u.VerticalBox, outer=canvas)
for child, height in [(row, 100), (station, 110)]:
    size = u.new_object(u.SizeBox, outer=canvas)
    size.set_width_override(385)
    size.set_height_override(height)
    size.add_child(child)
    body.add_child_to_vertical_box(size).set_padding(u.Margin(12))
EDIT.set_slot_content(canvas, 'NamedSlot', body)
recipe = u.QueuedRecipe()
data = recipe.get_editor_property('item_recipe_row')
data.set_editor_properties(dict(id='Bow', display_name=u.Text('Bow'), craft_volume=5.))
table = u.load_asset(ROOT + 'Data/DT_Recipe')
rows = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(table))
for entry in rows:
    if entry.get('Name') == 'Bow' and entry.get('RecipeIcon'):
        data.set_editor_property('recipe_icon', u.load_asset(entry['RecipeIcon']))
recipe.set_editor_property('item_recipe_row', data)
recipe.set_editor_property('count', 3)
# Import test data without exposing gameplay's read-only progress to Blueprint writes.
assert recipe.import_text('(CurrentProgress=2)')
EDIT.render_widget(canvas, u.Vector2D(409, 258), str(OUT / 'bar-percent.png'))
row.update_data(recipe)
station.queue_craft_list_entry_widget.update_data(recipe)
station.queue_craft_list_entry_widget.set_visibility(u.SlateVisibility.HIT_TEST_INVISIBLE)
station.world_frame.set_visibility(u.SlateVisibility.HIT_TEST_INVISIBLE)

frame = 0
callback = None


def tick(delta):
    global frame, callback
    frame += 1
    if frame in (20, 21):
        assert EDIT.render_widget(canvas, u.Vector2D(409, 258), str(OUT / 'bar-percent.png'))
    elif frame == 22:
        settings = u.CraftProgressDisplaySettings(use_progress_bar=True, show_percentage=False)
        row.set_progress_display_settings(settings)
        station.set_progress_display_settings(settings)
    elif frame == 23:
        assert EDIT.render_widget(canvas, u.Vector2D(409, 258), str(OUT / 'bar.png'))
    elif frame == 24:
        settings = u.CraftProgressDisplaySettings(use_progress_bar=False, show_percentage=False)
        row.set_progress_display_settings(settings)
        station.set_progress_display_settings(settings)
    elif frame == 25:
        assert EDIT.render_widget(canvas, u.Vector2D(409, 258), str(OUT / 'numbers.png'))
    elif frame == 26:
        settings = u.CraftProgressDisplaySettings(use_progress_bar=False, show_percentage=True)
        row.set_progress_display_settings(settings)
        station.set_progress_display_settings(settings)
    elif frame == 27:
        assert EDIT.render_widget(canvas, u.Vector2D(409, 258), str(OUT / 'percent.png'))
    elif frame == 28:
        u.unregister_slate_post_tick_callback(callback)
        EDIT.release_preview()
        u.log('CRAFT_PROGRESS_PREVIEW_OK')
        u.SystemLibrary.quit_editor()


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_post_tick_callback(tick)
