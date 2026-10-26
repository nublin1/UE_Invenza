#  Nublin Studio 2026 All Rights Reserved.

"""Place the native NPC inventory example. Saves only Map_Ex; no Blueprint saves."""

from pathlib import Path
import json
import shutil
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir()).resolve()
OUT = PROJECT/'Saved/CharacterExchange'
OUT.mkdir(parents=True, exist_ok=True)
source = PROJECT/'Plugins/InventorySystemInvenzaPlugin/Content/Maps/Map_Ex.umap'
backup = OUT/'Map_Ex-before.umap'
if not backup.exists():
    shutil.copy2(source, backup)

level = u.get_editor_subsystem(u.LevelEditorSubsystem)
assert level.load_level(ROOT+'Maps/Map_Ex')
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
existing = [a for a in actors.get_all_level_actors() if isinstance(a, u.InventoryMannequin)]
assert len(existing) <= 1
actor = existing[0] if existing else actors.spawn_actor_from_class(
    u.InventoryMannequin, u.Vector(2240, 760, 106), u.Rotator(yaw=90))
assert actor
actor.set_actor_label('NPC inventory and equipment - Mannequin')
access = actor.get_editor_property('inventory_access')
access.set_editor_property('character_display_name', u.Text('Companion'))
data = u.load_asset(ROOT+'Data/DT_ItemData')
rows = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(data))
bow = next((r['Name'] for r in rows if r['Name'].lower() == 'bow'), None)
assert bow, 'Missing demonstration Bow definition'
entry = u.InitItemsEntry()
assert entry.import_text('(Item=(DataTable="/Script/Engine.DataTable\''+ROOT+'Data/DT_ItemData.DT_ItemData\'",RowName="'+bow+'"),Amount=1)')
access.set_editor_property('bag_starting_items', [entry])
assert access.get_editor_property('create_default_inventories')
mesh = actor.get_editor_property('mesh')
assert mesh.get_editor_property('skeletal_mesh_asset')
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
assert u.EditorLoadingAndSavingUtils.save_map(world, ROOT+'Maps/Map_Ex')
u.log('INVENTORY_MANNEQUIN_SAVED '+str(actor.get_actor_location()))
