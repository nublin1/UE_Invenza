# Nublin Studio 2026 All Rights Reserved.
"""Exercise the real drop, ground interaction and inventory-key handlers without saving assets.

Use -BackpackReequipClient or -BackpackReequipListen for two-player PIE.
The default runs standalone PIE. Reports are written under Saved/BackpackReequipFix20261030.
"""
from pathlib import Path
import json
import time
import traceback
import unreal as u

EDIT = u.InvenzaWidgetEditingLibrary
COMMAND_LINE = u.SystemLibrary.get_command_line()
MODE = 'client' if '-BackpackReequipClient' in COMMAND_LINE else (
    'listen' if '-BackpackReequipListen' in COMMAND_LINE else 'standalone')
OUT = Path(u.Paths.project_saved_dir()) / 'BackpackReequipFix20261030'
OUT.mkdir(parents=True, exist_ok=True)
for setting in ['bEnableInPIE', 'bAutoLoad', 'bAutoSave', 'bSaveOnExit']:
    u.get_default_object(u.InvenzaSaveSettings).set_editor_property(setting, False)
assert u.get_editor_subsystem(u.LevelEditorSubsystem).load_level('/InventorySystemInvenzaPlugin/Maps/Map_Ex')
assert (EDIT.start_standalone_preview() if MODE == 'standalone'
        else u.InvenzaTakeAmountValidationLibrary.start_network_preview())

started = phase_at = time.monotonic()
phase = cycle = 0
last_tick = 0.0
done = False
callback = None
layer = manager = pawn = world = server_world = server_pawn = server_manager = None
pickup = server_pickup = original_id = original_contents = original_weight = None
report = {'mode': MODE, 'assets_saved': False, 'save_disabled': True, 'checks': [], 'snapshots': []}
# Fast re-equip, re-equip after the client state becomes irrelevant, and reuse of the ground-view panel.
CASES = [(0.5, False), (8.0, False), (0.5, True), (8.0, True)]


def check(condition, message):
    assert condition, message
    report['checks'].append(message)


def inventory_id(inventory):
    return str(inventory.get_editor_property('InventoryContainerID')) if inventory else None


def contents(inventory):
    collection = inventory.get_editor_property('ItemCollectionLinked')
    return {str(entry.item_id): entry.amount for entry in collection.collect_items_aggregated(inventory_id(inventory))}


def panels_for(inventory):
    return [container for container in layer.get_all_pawn_inv_containers()
            if container.get_editor_property('InventoryRef') == inventory]


def record(label):
    inventory = manager.get_editor_property('MainPawnInventoryRef')
    report['snapshots'].append({
        'label': label, 'cycle': cycle, 'main': str(inventory), 'main_id': inventory_id(inventory),
        'panels': [{'name': container.get_path_name(), 'inventory': str(container.get_editor_property('InventoryRef')),
                    'parent': str(container.get_parent()), 'visibility': str(container.get_visibility())}
                   for container in layer.get_all_pawn_inv_containers()]})
    (OUT / (MODE + '-progress.json')).write_text(json.dumps(report, indent=2), encoding='utf-8')


def next_phase(value):
    global phase, phase_at
    phase, phase_at = value, time.monotonic()


def finish(error=None):
    global done
    report['passed'] = error is None
    if error:
        report['error'] = error
        u.log_error(error)
    (OUT / (MODE + '-results.json')).write_text(json.dumps(report, indent=2), encoding='utf-8')
    u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
    done = True


def tick(delta):
    global last_tick, done, layer, manager, pawn, world, server_world, server_pawn, server_manager
    global pickup, server_pickup, original_id, original_contents, original_weight, cycle
    if done:
        u.unregister_slate_pre_tick_callback(callback)
        u.SystemLibrary.quit_editor()
        return
    now = time.monotonic()
    if now - last_tick < 0.2:
        return
    last_tick = now
    try:
        assert now - phase_at < 35, 'Timeout in phase ' + str(phase) + ', cycle ' + str(cycle)
        if phase == 0:
            layers = [candidate for candidate in u.ObjectIterator(u.GameMenuLayerInv)
                      if candidate.get_owning_player_pawn() and
                      'UEDPIE_' in candidate.get_owning_player_pawn().get_path_name()]
            layer = next((candidate for candidate in layers if
                          candidate.get_owning_player_pawn().has_authority() == (MODE != 'client')), None)
            if not layer or now - started < 8:
                return
            pawn = layer.get_owning_player_pawn()
            world = pawn.get_outer().get_outer()
            manager = pawn.get_component_by_class(u.IInventoryManager)
            if MODE == 'client':
                host = next((candidate.get_owning_player_pawn() for candidate in layers
                             if candidate.get_owning_player_pawn().has_authority()), None)
                if not host:
                    return
                server_world = host.get_outer().get_outer()
                server_pawn = next((candidate for candidate in u.GameplayStatics.get_all_actors_of_class(server_world, u.Pawn)
                                    if candidate.is_player_controlled() and not candidate.is_locally_controlled()), None)
                if not server_pawn:
                    return
            else:
                server_pawn, server_world = pawn, world
            server_manager = server_pawn.get_component_by_class(u.IInventoryManager)
            inventory = manager.get_editor_property('MainPawnInventoryRef')
            if not inventory or not panels_for(inventory):
                return
            original_id = inventory_id(inventory)
            original_contents = contents(inventory)
            original_weight = inventory.get_editor_property('InventoryTotalWeight')
            check(bool(original_contents), 'Starting bag contains real items')
            next_phase(1)
        elif phase == 1:
            if not layer.get_editor_property('bInventoryOpen'):
                check(EDIT.toggle_preview_inventory(pawn), 'Inventory input handler opens before drop')
            record('before_drop')
            check(EDIT.drop_preview_equipped_backpack(pawn), 'Drop sent through normal inventory request')
            next_phase(2)
        elif phase == 2:
            if server_manager.get_editor_property('MainPawnInventoryRef') or manager.get_editor_property('MainPawnInventoryRef'):
                return
            check(not [container for container in layer.get_all_pawn_inv_containers()
                       if inventory_id(container.get_editor_property('InventoryRef')) == original_id],
                  'Dropped bag is removed from player panels')
            # Close through the inventory input handler; no direct visibility writes in the probe.
            if layer.get_editor_property('bInventoryOpen'):
                EDIT.toggle_preview_inventory(pawn)
            server_pickup = next((actor.get_component_by_class(u.PickupComponent)
                for actor in u.GameplayStatics.get_all_actors_of_class(server_world, u.Actor)
                if actor.get_owner() == server_pawn and actor.get_component_by_class(u.PickupComponent)
                and any(item.is_portable_inventory() for item in actor.get_component_by_class(u.PickupComponent).get_pickup_items())), None)
            if not server_pickup:
                return
            record('dropped')
            next_phase(3)
        elif phase == 3 and now - phase_at >= CASES[cycle][0]:
            actor = next((candidate for candidate in u.GameplayStatics.get_all_actors_of_class(world, u.Actor)
                          if candidate.get_name() == server_pickup.get_owner().get_name()), None)
            pickup = actor.get_component_by_class(u.PickupComponent) if actor else None
            if not pickup or not pickup.get_pickup_items() or not pickup.get_pickup_items()[0].is_portable_inventory():
                return
            if CASES[cycle][1]:
                check(EDIT.preview_pickup_input(pawn, pickup, True, u.InteractionType.PRIMARY),
                      'Open dropped bag through Primary input')
                next_phase(4)
            else:
                check(EDIT.preview_pickup_input(pawn, pickup, True, u.InteractionType.SECONDARY),
                      'Equip ground bag through Secondary input')
                next_phase(5)
        elif phase == 4:
            links = manager.get_editor_property('ItemCollectionRef').get_editor_property('LinkedInventories')
            if not links.external_inventory or inventory_id(links.external_inventory) != original_id:
                return
            external = next((candidate for candidate in u.ObjectIterator(u.InventoryContainerWidget)
                             if candidate.get_owning_player_pawn() == pawn
                             and candidate.get_editor_property('InventoryRef') == links.external_inventory
                             and candidate.get_parent()), None)
            if not external or not external.get_parent() or now - phase_at < 1:
                return
            record('ground_view_open')
            check(EDIT.preview_pickup_input(pawn, pickup, True, u.InteractionType.SECONDARY),
                  'Equip the bag while its ground view is open')
            next_phase(5)
        elif phase == 5:
            inventory = manager.get_editor_property('MainPawnInventoryRef')
            if not inventory or not server_manager.get_editor_property('MainPawnInventoryRef') or now - phase_at < 2:
                return
            record('reequipped')
            check(inventory_id(inventory) == original_id, 'Re-equipped bag preserves inventory identity')
            check(len(panels_for(inventory)) == 1, 'Exactly one current bag panel is attached to player layer')
            if layer.get_editor_property('bInventoryOpen'):
                EDIT.toggle_preview_inventory(pawn)
            check(EDIT.toggle_preview_inventory(pawn), 'Actual inventory input handler invoked after re-equip')
            next_phase(6)
        elif phase == 6:
            inventory = manager.get_editor_property('MainPawnInventoryRef')
            panels = panels_for(inventory)
            check(len(panels) == 1 and panels[0].get_visibility() == u.SlateVisibility.VISIBLE,
                  'Inventory key displays the re-equipped bag')
            check(contents(inventory) == original_contents, 'Contents and quantities survive drop and re-equip')
            check(abs(inventory.get_editor_property('InventoryTotalWeight') - original_weight) < 0.01,
                  'Contents weight survives drop and re-equip')
            grid = panels[0].get_inventory_widget_from_container_slot()
            visuals = next(candidate for candidate in u.ObjectIterator(u.CanvasPanel)
                           if candidate.get_name() == 'ItemsVisualsPanel'
                           and candidate.get_outer() and candidate.get_outer().get_outer() == grid)
            check(visuals.get_children_count() > 0, 'Reopened panel actually contains item visuals')
            record('reopened')
            EDIT.toggle_preview_inventory(pawn)
            check(panels[0].get_visibility() == u.SlateVisibility.COLLAPSED,
                  'Second inventory key closes the re-equipped bag')
            cycle += 1
            if cycle == len(CASES):
                u.log('INVENZA_BACKPACK_ROUNDTRIP_PASSED ' + MODE)
                finish()
            else:
                next_phase(1)
    except Exception:
        finish(traceback.format_exc())


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_pre_tick_callback(tick)
