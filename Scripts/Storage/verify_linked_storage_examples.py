"""Nublin Studio 2026 All Rights Reserved. Read-only verification of the new examples."""
from pathlib import Path
import json
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
OUT = Path(u.Paths.project_saved_dir()) / 'LinkedStorageExamples'
OUT.mkdir(parents=True, exist_ok=True)
definition = u.load_asset(ROOT + 'Data/Storage/DA_LinkedStorageExample')
interaction_channel = u.get_default_object(u.InvenzaInventoryDeveloperSettings).get_editor_property('interaction_trace_channel')
assert definition.storage_group_id == 'ExampleStorage'
assert len(definition.inventory.start_items) == 1
assert definition.inventory.start_items[0].amount == 3
results = []
for name, mode in [('BP_SharedStorageChest', u.LinkedStorageAccessMode.SHARED),
                   ('BP_PersonalStorageChest', u.LinkedStorageAccessMode.PERSONAL)]:
    cls = u.load_class(None, ROOT + 'Blueprints/Examples/Storage/Linked/' + name + '.' + name + '_C')
    assert cls
    default = u.get_default_object(cls)
    assert default.linked_storage.storage_access_mode == mode
    assert default.linked_storage.storage_definition == definition
    assert default.linked_storage.window_class
    assert default.mesh.static_mesh
    assert default.mesh.get_collision_response_to_channel(interaction_channel) == u.CollisionResponseType.ECR_BLOCK
    results.append(dict(name=name, mode=str(mode), title=str(default.linked_storage.window_title),
                        mesh=default.mesh.static_mesh.get_path_name(), window=default.linked_storage.window_class.get_path_name()))
levels = u.get_editor_subsystem(u.LevelEditorSubsystem)
assert levels.load_level(ROOT + 'Maps/Map_Ex')
actors = u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors()
chests = [actor for actor in actors if isinstance(actor, u.LinkedStorageChest)]
assert len(chests) == 4
for mode in [u.LinkedStorageAccessMode.SHARED, u.LinkedStorageAccessMode.PERSONAL]:
    assert sum(chest.linked_storage.storage_access_mode == mode for chest in chests) == 2
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
assert world.get_world_settings().default_game_mode
placements = []
for chest in chests:
    assert chest.mesh.get_collision_response_to_channel(interaction_channel) == u.CollisionResponseType.ECR_BLOCK
    origin, extent = chest.get_actor_bounds(False)
    placements.append(dict(name=chest.get_actor_label(), floor_clearance_cm=origin.z - extent.z,
                           location=str(chest.get_actor_location())))
report = dict(blueprints=results, map_chests=len(chests), placements=placements,
              map_game_mode=world.get_world_settings().default_game_mode.get_path_name(),
              no_assets_saved=True)
(OUT / 'example-verification.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
u.log('LINKED_STORAGE_EXAMPLES_VERIFIED ' + json.dumps(report))
