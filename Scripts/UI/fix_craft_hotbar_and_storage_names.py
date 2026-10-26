# Nublin Studio 2026 All Rights Reserved.
"""Fix two UI layouts and the example level's inventory display names only."""
import hashlib
import json
from pathlib import Path
import shutil
import traceback
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir())
CONTENT = PROJECT / 'Plugins/InventorySystemInvenzaPlugin/Content'
OUT = PROJECT / 'Saved/InventoryPolish'
EDIT = u.InvenzaWidgetEditingLibrary
DASHBOARD = 'UI/Craft/WBP_CraftDashboard'
HOTBAR = 'UI/Core/SimpleInvGrid/WBP_HotbarGrid'
TEXTS = 'Data/Localization/ST_InvenzaRuntimeTexts'
LEVEL = 'Maps/Map_Ex'


def widget(bp, name):
    result = u.find_object(u.find_object(bp, 'WidgetTree'), name)
    assert result is not None, name
    return result


def input_settings(bp):
    tree = u.find_object(bp, 'WidgetTree')
    return {w.get_name(): [str(w.get_visibility()), w.get_render_opacity()]
            for w in u.ObjectIterator(u.Widget) if w.get_outer() == tree}


def backup(relative, extension):
    source = CONTENT / (relative + extension)
    target = OUT / 'Before' / (relative + extension)
    if not target.exists():
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)


def update_title(component, index, text):
    rows = list(component.get_editor_property('startup_inventories'))
    before = [row.export_text() for row in rows]
    settings = rows[index].get_editor_property('settings')
    settings.set_editor_property('container_title', text)
    rows[index].set_editor_property('settings', settings)
    component.modify()
    component.set_editor_property('startup_inventories', rows,
                                  notify_mode=u.PropertyAccessChangeNotifyMode.NEVER)
    actual = list(component.get_editor_property('startup_inventories'))
    assert str(actual[index].settings.container_title) == str(text)
    for row, original in zip(actual, before):
        restored = type(row)()
        assert restored.import_text(original)
        candidate = type(row)()
        assert candidate.import_text(row.export_text())
        row_settings = candidate.get_editor_property('settings')
        row_settings.set_editor_property('container_title', restored.settings.container_title)
        candidate.set_editor_property('settings', row_settings)
        assert candidate.export_text() == restored.export_text(), 'A non-title inventory property changed'


def run():
    OUT.mkdir(parents=True, exist_ok=True)
    baseline_path = OUT / 'asset-hashes-before.json'
    if not baseline_path.exists():
        hashes = {str(p.relative_to(CONTENT)): hashlib.sha256(p.read_bytes()).hexdigest()
                  for p in CONTENT.rglob('*') if p.suffix in ['.uasset', '.umap']}
        baseline_path.write_text(json.dumps(hashes, indent=2), encoding='utf-8')
    for relative in [DASHBOARD, HOTBAR, TEXTS]:
        backup(relative, '.uasset')
    backup(LEVEL, '.umap')

    dashboard = u.load_asset(ROOT + DASHBOARD)
    hotbar = u.load_asset(ROOT + HOTBAR)
    before_dashboard = input_settings(dashboard)
    before_hotbar = input_settings(hotbar)
    workspace = widget(dashboard, 'InventoryWorkspace')
    display = workspace.get_editor_property('display_settings')
    display.set_editor_property('player_column_min_width', 420.)
    display.set_editor_property('fit_player_column_to_content', True)
    workspace.set_editor_property('display_settings', display)
    # Keep the existing window footprint; allocate its spare storage-column space to the player.
    widget(dashboard, 'CraftRoot').set_min_desired_width(1040.)

    # An auto-sized, point-anchored child contributes its content size to the canvas.
    # Stretching in an automatically sized canvas produced a zero-height item layer.
    overlay_slot = widget(hotbar, 'Overlay_604').slot
    assert isinstance(overlay_slot, u.CanvasPanelSlot)
    overlay_slot.set_anchors(u.Anchors(minimum=u.Vector2D(0., 0.), maximum=u.Vector2D(0., 0.)))
    overlay_slot.set_auto_size(True)
    overlay_slot.set_offsets(u.Margin(0.))

    assert input_settings(dashboard) == before_dashboard
    assert input_settings(hotbar) == before_hotbar
    assert EDIT.compile_widget(dashboard)
    assert EDIT.compile_widget(hotbar)
    assert input_settings(dashboard) == before_dashboard
    assert input_settings(hotbar) == before_hotbar

    strings = u.load_asset(ROOT + TEXTS)
    captions = {
        'ExampleStorage.Chest': 'Chest - hold to open',
        'ExampleStorage.RackBinary': 'Rack - empty / full',
        'ExampleStorage.RackGradual': 'Rack - gradual fill',
        'ExampleStorage.Grid': 'Grid storage',
        'ExampleStorage.Trader': 'Trader stock',
        'ExampleStorage.StationInput': 'Station input',
        'ExampleStorage.StationOutput': 'Station output',
        'ExampleStorage.StationFuel': 'Station fuel',
    }
    for key, text in captions.items():
        EDIT.set_string_table_entry(strings, key, text)
    level = u.get_editor_subsystem(u.LevelEditorSubsystem)
    assert level.load_level(ROOT + LEVEL)
    actors = {a.get_name(): a for a in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors()}
    titles = []
    assignments = {
        'BP_Container_Hold_C_1': ['ExampleStorage.Chest'],
        'BP_Grid_C_3': ['ExampleStorage.Grid'],
        'BP_Vendor_C_2': ['ExampleStorage.Trader'],
        'BP_CraftStation_C_4': ['ExampleStorage.StationInput', 'ExampleStorage.StationOutput', 'ExampleStorage.StationFuel'],
    }
    for name in ['BP_Container_C_0', 'BP_Container_C_1']:
        actor = actors[name]
        visual = next(c for c in actor.get_components_by_class(u.ActorComponent)
                      if c.get_class().get_name() == 'InventoryVisualizer')
        thresholds = visual.get_editor_property('occupancy_meshes')
        assignments[name] = ['ExampleStorage.RackGradual' if len(thresholds) > 2 else 'ExampleStorage.RackBinary']
    for name, keys in assignments.items():
        actor = actors[name]
        actor.modify()
        component_name = 'InvManager' if name == 'BP_Vendor_C_2' else 'Container'
        collection = next(c for c in actor.get_components_by_class(u.ActorComponent)
                          if c.get_name() == component_name)
        assert collection is not None, name
        for index, key in enumerate(keys):
            text = EDIT.string_table_text(strings, key)
            update_title(collection, index, text)
            titles.append(dict(actor=name, index=index, title=str(text), key=key))

    access = actors['BP_StorageAccessPoint_C_1'].get_component_by_class(u.StorageAccessComponent)
    blacklist = list(access.get_editor_property('excluded_inventory_names'))
    for key in ['ExampleStorage.StationInput', 'ExampleStorage.StationOutput', 'ExampleStorage.StationFuel']:
        if captions[key] not in blacklist:
            blacklist.append(captions[key])
    # Existing session saves retain these previous station titles.
    for legacy_title in ['Input', 'Output', 'Fuel']:
        if legacy_title not in blacklist:
            blacklist.append(legacy_title)
    access.modify()
    access.set_editor_property('excluded_inventory_names', blacklist,
                              notify_mode=u.PropertyAccessChangeNotifyMode.NEVER)

    for name, keys in assignments.items():
        component_name = 'InvManager' if name == 'BP_Vendor_C_2' else 'Container'
        component = next(c for c in actors[name].get_components_by_class(u.ActorComponent)
                         if c.get_name() == component_name)
        actual_titles = [str(row.settings.container_title)
                         for row in component.get_editor_property('startup_inventories')]
        assert actual_titles == [captions[key] for key in keys], (name, actual_titles)

    for asset in [strings, dashboard, hotbar]:
        assert u.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
    assert level.save_current_level()
    baseline = json.loads(baseline_path.read_text(encoding='utf-8'))
    changed = [Path(name).as_posix() for name, digest in baseline.items()
               if hashlib.sha256((CONTENT / name).read_bytes()).hexdigest() != digest]
    expected = {DASHBOARD + '.uasset', HOTBAR + '.uasset', TEXTS + '.uasset', LEVEL + '.umap'}
    assert set(changed) == expected, changed
    (OUT / 'asset-changes.json').write_text(json.dumps(dict(changed=changed, titles=titles,
        blacklist=blacklist, input_settings_preserved=True), indent=2), encoding='utf-8')
    u.log('INVENTORY_POLISH_ASSETS_OK')


try:
    run()
except Exception:
    (OUT / 'asset-error.txt').write_text(traceback.format_exc(), encoding='utf-8')
    raise
finally:
    u.SystemLibrary.quit_editor()
