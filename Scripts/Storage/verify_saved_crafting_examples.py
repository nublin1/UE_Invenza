# Nublin Studio 2026 All Rights Reserved.
"""Reload saved assets, capture storage, and check actual PIE recipe availability.

Read-only: no assets are saved by this verification.
"""
import importlib.util
import itertools
import json
import time
import traceback
from pathlib import Path
import unreal as u

PROJECT = Path(u.Paths.project_dir())
OUT = PROJECT / 'Saved/StorageLabelsScoped'
ROOT = '/InventorySystemInvenzaPlugin/'
NAMES = ['Bow', 'FieldBow', 'BrewRedPotion', 'SmeltIron', 'CraftSword']
level = u.get_editor_subsystem(u.LevelEditorSubsystem)
editor = u.get_editor_subsystem(u.UnrealEditorSubsystem)
spec = importlib.util.spec_from_file_location('invenza_storage_preview',
    PROJECT / 'Scripts/Storage/finish_recipe_owners_and_preview.py')
preview = importlib.util.module_from_spec(spec)
spec.loader.exec_module(preview)
world, capture, target = preview.setup_preview()
started = time.monotonic()
stage = 0
frames = 0
callback = None
report = {}


def check_game():
    game = editor.get_game_world()
    assert game, 'PIE did not start'
    owners = []
    found_player = False
    found_station = False
    native_checks = 0
    for actor in u.GameplayStatics.get_all_actors_of_class(game, u.Actor):
        for component in actor.get_components_by_class(u.CraftingComponent):
            recipes = component.get_available_recipes()
            ids = [str(row.get_editor_property('id')) for row in recipes]
            assert set(ids) == set(NAMES), (actor.get_name(), ids)
            owners.append(dict(actor=actor.get_name(), recipes=ids))
            found_player |= actor.get_class().get_name() == 'BP_Character_C'
            found_station |= actor.get_class().get_name() == 'BP_CraftStation_C'
            if native_checks:
                continue
            for row in recipes:
                name = str(row.get_editor_property('id'))
                if name == 'Bow':
                    continue
                groups = []
                for req in row.get_editor_property('required_items'):
                    options = [(req.get_editor_property('item'), req.get_editor_property('quantity'))]
                    options.extend((alt.get_editor_property('item'), alt.get_editor_property('quantity'))
                        for alt in req.get_editor_property('alternatives'))
                    groups.append(options)
                for selected in itertools.product(*(range(len(group)) for group in groups)):
                    amounts = []
                    for group, index in zip(groups, selected):
                        handle, count = group[index]
                        amounts.append(u.ItemIDEntry(item_id=handle.get_editor_property('row_name'), amount=count))
                    assert u.CraftingComponent.can_craft_with_items_options(row, amounts, list(selected), 1).can_craft, name
                    native_checks += 1
                    for index in range(len(amounts)):
                        short = [u.ItemIDEntry(item_id=value.item_id, amount=value.amount - int(i == index))
                            for i, value in enumerate(amounts)]
                        assert not u.CraftingComponent.can_craft_with_items_options(row, short, list(selected), 1).can_craft, name
                        native_checks += 1
    assert found_player and found_station, (found_player, found_station, owners)
    assert native_checks == 28, native_checks
    return dict(owners=owners, native_crafting_checks=native_checks, player_and_station_verified=True)


def tick(delta):
    global stage, started, frames, report
    try:
        if stage == 0:
            if time.monotonic() - started < 15:
                return
            capture.capture_scene()
            frames += 1
            if frames >= 4:
                u.RenderingLibrary.export_render_target(world, target, str(OUT), 'storage-labels-unreal.png')
                level.load_level(ROOT + 'Maps/Map_Ex')
                level.editor_request_begin_play()
                started = time.monotonic()
                stage = 1
        elif stage == 1:
            if time.monotonic() - started < 5:
                return
            report = check_game()
            # Recheck Bow against the initial native export after loading the actual game.
            table = u.load_asset(ROOT + 'Data/DT_Recipe')
            rows = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(table))
            original = json.loads((OUT / 'DT_Recipe-before.json').read_text(encoding='utf-8'))
            assert next(r for r in rows if r['Name'] == 'Bow') == next(r for r in original if r['Name'] == 'Bow')
            assert len(rows) == 5
            report['preserved_bow'] = True
            (OUT / 'verification.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
            level.editor_request_end_play()
            stage = 2
        elif stage == 2 and not editor.get_game_world():
            u.unregister_slate_post_tick_callback(callback)
            u.log('SAVED_CRAFTING_AND_PIE_VERIFIED_OK')
            u.SystemLibrary.quit_editor()
    except Exception:
        error = traceback.format_exc()
        u.log_error(error)
        (OUT / 'verification-error.txt').write_text(error, encoding='utf-8')
        level.editor_request_end_play()
        u.unregister_slate_post_tick_callback(callback)
        u.SystemLibrary.quit_editor()


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_post_tick_callback(tick)
