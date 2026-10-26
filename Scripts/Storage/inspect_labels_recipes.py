# Nublin Studio 2026 All Rights Reserved.
"""Export the current tables and recipe owners without saving any assets."""
import json
from pathlib import Path
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
OUT = Path(u.Paths.project_dir()) / 'Saved/StorageLabelsScoped'
OUT.mkdir(parents=True, exist_ok=True)
try:
    for name in ['DT_ItemData', 'DT_Recipe']:
        table = u.load_asset(ROOT + 'Data/' + name)
        assert table, name
        (OUT / (name + '-before.json')).write_text(
            u.DataTableFunctionLibrary.export_data_table_to_json_string(table), encoding='utf-8')
    sub = u.get_engine_subsystem(u.SubobjectDataSubsystem)
    owners = []
    for path in ['Blueprints/Examples/BP_Character', 'Blueprints/Examples/CraftStaton/BP_CraftStation']:
        bp = u.load_asset(ROOT + path)
        for handle in sub.k2_gather_subobject_data_for_blueprint(bp):
            data = u.SubobjectDataBlueprintFunctionLibrary.get_data(handle)
            component = u.SubobjectDataBlueprintFunctionLibrary.get_object(data)
            if isinstance(component, u.CraftingComponent):
                owners.append({'blueprint': ROOT + path, 'component': component.get_path_name(),
                    'recipes': [entry.export_text() for entry in component.get_editor_property('starting_recipes')]})
    (OUT / 'recipe-owners.json').write_text(json.dumps(owners, indent=2), encoding='utf-8')
    u.log('SCOPED_STORAGE_INSPECTION_OK')
finally:
    u.SystemLibrary.quit_editor()
