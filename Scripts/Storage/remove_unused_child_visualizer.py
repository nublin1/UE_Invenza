# Nublin Studio 2026 All Rights Reserved.
"""Remove only the confirmed unused local component; save only its named Blueprint."""
import json
from pathlib import Path
import unreal as u

OUT = Path('G:/UEProjects/UE_Invenza/Saved/FabRelease20261022-Final')
ASSET = '/InventorySystemInvenzaPlugin/Blueprints/Examples/Containers/BP_LootContainer_Child'
bp = u.load_asset(ASSET)
assert bp is not None
subsystem = u.get_engine_subsystem(u.SubobjectDataSubsystem)
library = u.SubobjectDataBlueprintFunctionLibrary
fields = ['track_all_invs', 'default_search_tag', 'display_mode', 'socket_keywords',
          'occupancy_meshes', 'auto_activate', 'component_tags', 'is_editor_only', 'replicates']

def serialize(value):
    if isinstance(value, u.Object):
        return value.get_path_name()
    if isinstance(value, (str, bool, int, float)) or value is None:
        return value
    if hasattr(value, 'items'):
        return {str(key): serialize(item) for key, item in value.items()}
    if hasattr(value, 'export_text'):
        return value.export_text()
    try:
        return [serialize(item) for item in value]
    except TypeError:
        return str(value)

def settings(obj):
    return {field: serialize(obj.get_editor_property(field)) for field in fields}

def components():
    handles = subsystem.k2_gather_subobject_data_for_blueprint(bp)
    entries = []
    for handle in handles:
        data = library.get_data(handle)
        obj = library.get_associated_object(data)
        if isinstance(obj, u.InventoryVisualizer):
            effective = library.get_object_for_blueprint(data, bp) or obj
            entries.append(dict(handle=handle, inherited=library.is_inherited_component(data),
                                name=str(library.get_variable_name(data)), properties=settings(effective)))
    return handles, entries

handles, before = components()
local = [entry for entry in before if not entry['inherited']]
inherited = [entry for entry in before if entry['inherited']]
assert len(local) == len(inherited) == 1, 'Expected exactly one local and one inherited visualizer'
assert local[0]['properties'] == settings(u.get_default_object(u.InventoryVisualizer)), 'Local component settings changed since inspection'
deleted = subsystem.delete_subobject(handles[0], local[0]['handle'], bp)
assert deleted == 1, f'Expected one deleted subobject, got {deleted}'
u.BlueprintEditorLibrary.compile_blueprint(bp)
_, after = components()
assert len(after) == 1 and after[0]['inherited'], 'Inherited visualizer must be the only remaining visualizer'
assert after[0]['properties'] == inherited[0]['properties'], 'Inherited settings changed'
assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False), 'Saving the named Blueprint failed'
result = dict(asset=ASSET, removed=local[0]['name'], deleted_count=deleted,
              inherited_settings_preserved=True, saved_assets=[ASSET],
              before=[{key: value for key, value in entry.items() if key != 'handle'} for entry in before],
              after=[{key: value for key, value in entry.items() if key != 'handle'} for entry in after])
(OUT / 'component-removal.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
u.log('VISUALIZER_REMOVAL ' + json.dumps(result))
