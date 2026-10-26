"""Nublin Studio 2026 All Rights Reserved.

Place the four linked-storage examples in Map_Ex. Save only that level.
"""
from pathlib import Path
import json
import shutil
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir()).resolve()
OUT = PROJECT / 'Saved/LinkedStorageExamples'
OUT.mkdir(parents=True, exist_ok=True)
MAP = ROOT + 'Maps/Map_Ex'
source = PROJECT / 'Plugins/InventorySystemInvenzaPlugin/Content/Maps/Map_Ex.umap'
backup = OUT / 'Map_Ex-before-linked-storage.umap'
if not backup.exists():
    shutil.copy2(source, backup)

levels = u.get_editor_subsystem(u.LevelEditorSubsystem)
assert levels.load_level(MAP)
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
assert not any(isinstance(actor, u.LinkedStorageChest) for actor in actors.get_all_level_actors()), \
    'Map_Ex already contains linked chests; preserve existing placements.'

placed = []
for name, title, mode, positions in [
    ('BP_SharedStorageChest', 'SHARED', u.LinkedStorageAccessMode.SHARED, [(1750, 1900), (2450, 1900)]),
    ('BP_PersonalStorageChest', 'PERSONAL', u.LinkedStorageAccessMode.PERSONAL, [(2000, 1900), (2700, 1900)]),
]:
    cls = u.load_class(None, ROOT + 'Blueprints/Examples/Storage/Linked/' + name + '.' + name + '_C')
    assert cls
    for index, (x, y) in enumerate(positions, 1):
        # Face the central aisle; both pairs share their existing definition asset.
        chest = actors.spawn_actor_from_class(cls, u.Vector(x, y, 0), u.Rotator(yaw=180))
        assert chest
        chest.set_actor_label('Linked storage - ' + title.title() + ' ' + str(index))
        chest.set_folder_path('Examples/Linked Storage')
        assert chest.linked_storage.storage_access_mode == mode
        origin, extent = chest.get_actor_bounds(False)
        # Put the mesh base exactly on the existing floor at Z=0.
        chest.add_actor_world_offset(u.Vector(0, 0, extent.z - origin.z), False, False)
        label = actors.spawn_actor_from_class(u.TextRenderActor, u.Vector(x, y - 5, 155), u.Rotator(yaw=-90))
        assert label
        label.set_actor_label('Linked storage - ' + title.title() + ' ' + str(index) + ' label')
        label.set_folder_path('Examples/Linked Storage')
        label.text_render.set_text(u.Text(title + ' ' + str(index) + '\nLINKED STORAGE'))
        label.text_render.set_world_size(18)
        label.text_render.set_horizontal_alignment(u.HorizTextAligment.EHTA_CENTER)
        placed.append(dict(name=chest.get_name(), label=chest.get_actor_label(),
                           mode=str(mode), location=chest.get_actor_location().to_tuple()))

assert levels.save_current_level()
report = dict(map=MAP, chests=placed, saved_packages=[MAP], blueprint_assets_saved=[])
(OUT / 'placement.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
u.log('LINKED_STORAGE_PLACED_IN_MAP_EX ' + json.dumps(report))
