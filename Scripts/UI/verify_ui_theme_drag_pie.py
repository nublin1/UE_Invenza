# Nublin Studio 2026 All Rights Reserved.
"""Real PIE drag into a reference hotbar, then start dragging its new entry out. No saves."""
from pathlib import Path
import json
import time
import traceback
import unreal as u

OUT = Path(u.Paths.project_saved_dir()) / 'UITheme20261031'
EDIT = u.InvenzaWidgetEditingLibrary
for name in ('bEnableInPIE', 'bAutoLoad', 'bAutoSave', 'bSaveOnExit'):
    u.get_default_object(u.InvenzaSaveSettings).set_editor_property(name, False)
assert u.get_editor_subsystem(u.LevelEditorSubsystem).load_level('/InventorySystemInvenzaPlugin/Maps/Map_Ex')
assert EDIT.start_standalone_preview()
start = phase_start = time.monotonic()
phase = 0
callback = None
done = False
report = {'checks': [], 'assets_saved': False}
layer = manager = bag = source = target = source_cell = target_cell = None
original_contents = None
before_visual_count = 0


def check(condition, message):
    assert condition, message
    report['checks'].append(message)


def contents():
    return {str(e.item_id): e.amount for e in manager.get_editor_property('ItemCollectionRef').collect_items_aggregated(
                str(bag.get_editor_property('InventoryContainerID')))}


def component(owner, name):
    return u.InvenzaTakeAmountValidationLibrary.find_preview_widget(owner, name)


def ghost_for(owner):
    children = component(owner, 'HighlightVisualsPanel').get_all_children()
    return children[0] if children else None


def finish(error=None):
    global done
    report['passed'] = error is None
    if error:
        report['error'] = error
        u.log_error(error)
    (OUT / 'drag-pie.json').write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding='utf-8')
    u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
    done = True


def tick(delta):
    global phase, phase_start, layer, manager, bag, source, target, source_cell, target_cell
    global original_contents, before_visual_count
    if done:
        u.unregister_slate_pre_tick_callback(callback)
        u.SystemLibrary.quit_editor()
        return
    now = time.monotonic()
    try:
        assert now - phase_start < 120, 'Timeout in phase ' + str(phase)
        if phase == 0:
            candidates = [w for w in u.ObjectIterator(u.GameMenuLayerInv) if w.get_owning_player_pawn()
                          and 'UEDPIE_' in w.get_owning_player_pawn().get_path_name()]
            if not candidates or now - start < 8:
                return
            layer = candidates[0]
            pawn = layer.get_owning_player_pawn()
            manager = pawn.get_component_by_class(u.IInventoryManager)
            bag = manager.get_editor_property('MainPawnInventoryRef')
            if not bag:
                return
            check(EDIT.toggle_preview_inventory(pawn), 'Open inventory through its input handler')
            phase, phase_start = 1, now
        elif phase == 1 and now - phase_start > 1:
            source = next(c.get_inventory_widget_from_container_slot() for c in layer.get_all_pawn_inv_containers()
                          if c.get_editor_property('InventoryRef') == bag)
            target = next(w for w in u.ObjectIterator(u.SlotbasedInventoryWidget)
                          if w.get_owning_player_pawn() == layer.get_owning_player_pawn()
                          and w.get_class().get_name().startswith('WBP_HotbarGrid'))
            original_contents = contents()
            source_cells = list(component(source, 'SlotsGridPanel').get_all_children())
            target_cell = component(target, 'SlotsGridPanel').get_all_children()[-1]
            before_visual_count = component(target, 'ItemsVisualsPanel').get_children_count()
            def rectangle(w):
                r = EDIT.get_preview_widget_rect(w, layer)
                return [r.x, r.y, r.z, r.w]
            report['geometry'] = {'source': rectangle(source), 'target': rectangle(target),
                'source_cell': rectangle(source_cells[0]), 'target_cell': rectangle(target_cell)}
            if report['geometry']['source_cell'][2] <= 0 or report['geometry']['target_cell'][2] <= 0:
                return
            report['attempts'] = []
            for cell in source_cells:
                handled = EDIT.preview_drag_over(source, cell, target, target_cell, u.Vector2D(.5, .5))
                ghost = ghost_for(target)
                report['attempts'].append({'cell': cell.get_name(), 'handled': handled,
                    'under_mouse': str(source.get_editor_property('SlotUnderMouse')),
                    'ghost': str(ghost), 'state': str(ghost.get_editor_property('CurrentState')) if ghost else '',
                    'visibility': str(ghost.get_visibility()) if ghost else ''})
                if ghost and ghost.get_visibility() != u.SlateVisibility.COLLAPSED and ghost.get_editor_property('CurrentState') == u.HighlightState.ALLOWED:
                    source_cell = cell
                    break
            check(source_cell is not None, 'Dragging a real bag item produces an allowed hotbar preview')
            phase, phase_start = 3, now
        elif phase == 3 and now - phase_start > .25:
            ghost = ghost_for(target)
            color = ghost.get_editor_property('CoreCellWidget').get_editor_property('Left_Border').get_editor_property('brush_color')
            expected = u.InvenzaUIThemeLibrary.theme_color('Drop.Valid', color)
            check(abs(color.r-expected.r) + abs(color.g-expected.g) + abs(color.b-expected.b) < .0001,
                  'Hotbar outline uses the valid-drop theme role')
            rect = EDIT.get_preview_widget_rect(ghost, target)
            cell_rect = EDIT.get_preview_widget_rect(target_cell, target)
            check(abs(rect.z-cell_rect.z) < 1 and abs(rect.w-cell_rect.w) < 1, 'Hotbar ghost matches the authored cell geometry')
            check(EDIT.complete_preview_drop(target, target_cell), 'Drop is handled by the actual inventory widget')
            phase, phase_start = 2, now
        elif phase == 2 and now - phase_start > 1:
            check(component(target, 'ItemsVisualsPanel').get_children_count() == before_visual_count + 1,
                  'The new hotbar reference has an item visual')
            check(contents() == original_contents, 'Reference drop preserves real bag item quantities')
            EDIT.preview_drag_over(target, target_cell, source, source_cell, u.Vector2D(.5, .5))
            ghost = ghost_for(source)
            check(ghost and ghost.get_visibility() != u.SlateVisibility.COLLAPSED, 'The new hotbar entry can start a drag out')
            check(contents() == original_contents, 'Starting a drag out does not duplicate or remove the real item')
            u.log('UI_THEME_DRAG_PIE_PASSED')
            finish()
    except Exception:
        finish(traceback.format_exc())


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_pre_tick_callback(tick)
