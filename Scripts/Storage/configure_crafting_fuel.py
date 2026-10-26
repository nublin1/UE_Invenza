# Nublin Studio 2026 All Rights Reserved.
"""Configure only the approved fuel examples and block reason.

Updates InvenzaSettings, Log's traits in DT_ItemData, BP_CraftStation's two
fuel properties, and the existing runtime String Table. No UI assets are saved.
"""
import hashlib
import json
from pathlib import Path
import shutil
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir())
CONTENT = PROJECT / 'Plugins/InventorySystemInvenzaPlugin/Content'
OUT = PROJECT / 'Saved/CraftFuel'
EDIT = u.InvenzaWidgetEditingLibrary
DATA = u.InvenzaDataTableEditingLibrary
PACKAGES = ['Data/Settings/InvenzaSettings', 'Data/DT_ItemData',
    'Blueprints/Examples/CraftStaton/BP_CraftStation', 'Data/Localization/ST_InvenzaRuntimeTexts']


def save(asset):
    assert u.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False), asset


def tag(name):
    value = u.GameplayTag()
    assert value.import_text('(TagName="' + name + '")')
    assert name in value.export_text()
    return value


def run():
    OUT.mkdir(parents=True, exist_ok=True)
    baseline = OUT / 'asset-hashes-before.json'
    if not baseline.exists():
        baseline.write_text(json.dumps({str(p.relative_to(CONTENT)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in CONTENT.rglob('*') if p.suffix in ['.uasset', '.umap']}, indent=2), encoding='utf-8')
    for relative in PACKAGES:
        source = CONTENT / (relative + '.uasset')
        backup = OUT / 'Before' / (relative + '.uasset')
        if not backup.exists():
            backup.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, backup)

    strings = u.load_asset(ROOT + PACKAGES[3])
    EDIT.set_string_table_entry(strings, 'InvenzaCraft.NoFuel', 'Not enough fuel')
    save(strings)
    settings = u.load_asset(ROOT + PACKAGES[0])
    fuel_block = tag('CraftSystem.Blocks.NoFuel')
    settings.set_editor_property('block_no_fuel', fuel_block)
    reasons = list(settings.get_editor_property('available_block_reasons'))
    reasons = [r for r in reasons if r.tag.export_text() != fuel_block.export_text()]
    reasons.append(u.BlockReasonData(tag=fuel_block,
        message=EDIT.string_table_text(strings, 'InvenzaCraft.NoFuel')))
    settings.set_editor_property('available_block_reasons', reasons)
    save(settings)

    items = u.load_asset(ROOT + PACKAGES[1])
    before = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(items))
    (OUT / 'items-before.json').write_text(json.dumps(before, indent=2), encoding='utf-8')
    log = next(row for row in before if row['Name'] == 'Log')
    metadata = u.ItemMetaData()
    assert metadata.import_text(log['ItemMetaData'])
    fuel_tags = u.GameplayTagContainer()
    assert fuel_tags.import_text('(GameplayTags=((TagName="Item.Trait.Fuel")))')
    traits = u.GameplayTagLibrary.append_gameplay_tag_containers(metadata.item_tags, fuel_tags)
    metadata.set_editor_property('item_tags', traits)
    row_text = '(ID="' + log['ID'] + '",ItemMetaData=' + metadata.export_text() + ')'
    assert DATA.set_row_from_text(items, 'Log', row_text)
    after = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(items))
    assert [r for r in before if r['Name'] != 'Log'] == [r for r in after if r['Name'] != 'Log']
    check = next(r for r in after if r['Name'] == 'Log')
    parsed = u.ItemMetaData()
    assert parsed.import_text(check['ItemMetaData'])
    assert 'Item.Trait.Fuel' in parsed.item_tags.export_text()
    parsed.set_editor_property('item_tags', u.ItemMetaData().item_tags)
    original = u.ItemMetaData()
    assert original.import_text(log['ItemMetaData'])
    original.set_editor_property('item_tags', u.ItemMetaData().item_tags)
    assert parsed.export_text() == original.export_text(), 'Unrelated Log metadata changed'
    save(items)

    station = u.load_asset(ROOT + PACKAGES[2])
    subsystem = u.get_engine_subsystem(u.SubobjectDataSubsystem)
    components = []
    for handle in subsystem.k2_gather_subobject_data_for_blueprint(station):
        value = u.SubobjectDataBlueprintFunctionLibrary.get_object(
            u.SubobjectDataBlueprintFunctionLibrary.get_data(handle))
        if isinstance(value, u.CraftingComponent):
            value.modify()
            value.set_editor_properties(dict(requires_fuel=True, fuel_item_tag=tag('Item.Trait.Fuel')))
            components.append(value.get_path_name())
    assert components
    u.BlueprintEditorLibrary.compile_blueprint(station)
    save(station)
    old = json.loads(baseline.read_text(encoding='utf-8'))
    changed = [Path(name).as_posix() for name, digest in old.items()
        if hashlib.sha256((CONTENT / name).read_bytes()).hexdigest() != digest]
    assert set(changed) <= {p + '.uasset' for p in PACKAGES}, changed
    (OUT / 'asset-changes.json').write_text(json.dumps(dict(changed=changed,
        station_components=components, only_log_traits_changed=True), indent=2), encoding='utf-8')
    u.log('CRAFT_FUEL_ASSETS_OK')


try:
    run()
finally:
    u.SystemLibrary.quit_editor()
