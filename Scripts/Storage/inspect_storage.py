import unreal as u
import json
from pathlib import Path

root = '/InventorySystemInvenzaPlugin/'
table = u.load_asset(root + 'Data/DT_ItemData')
out = Path(u.Paths.project_dir()) / 'Saved/BulkBins'
out.mkdir(parents=True, exist_ok=True)
u.DataTableFunctionLibrary.export_data_table_to_json_file(table, str(out / 'items_before.json'))
u.get_editor_subsystem(u.LevelEditorSubsystem).load_level(root + 'Maps/Map_Ex')
actors = u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors()
lines = []
for a in actors:
    if 'Grid' in a.get_class().get_name() or a.get_class().get_name() == 'PlayerStart':
        lines.append(f'{a.get_path_name()} | {a.get_actor_label()} | {a.get_actor_location()} | {a.get_actor_rotation()}')
        for c in a.get_components_by_class(u.ActorComponent):
            lines.append(f'  {c.get_class().get_name()} {c.get_name()}')
            for prop in ['startup_inventories', 'main_loot_container_inv_tag', 'inventory_search_tag', 'interaction_channel', 'cell_scale', 'preview_grid_size', 'static_mesh']:
                try: lines.append(f'    {prop}: {c.get_editor_property(prop)}')
                except Exception: pass
(out / 'scene_before.txt').write_text('\n'.join(lines), encoding='utf-8')
u.log('BULK_INSPECT_DONE')
