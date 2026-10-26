# Nublin Studio 2026 All Rights Reserved.
"""Apply the four agreed recipes; preserve Bow and all unrelated item data.

No Blueprint, UI or level assets are modified by this script.
"""
import itertools
import json
import shutil
from pathlib import Path
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir())
OUT = PROJECT / 'Saved/StorageLabelsScoped'
TABLE_PATH = ROOT + 'Data/DT_ItemData'
RECIPE_PATH = ROOT + 'Data/DT_Recipe'
TEXT_PATH = ROOT + 'Data/Localization/ST_CraftingExamples'
EDIT = u.InvenzaDataTableEditingLibrary
TEXT = u.InvenzaWidgetEditingLibrary


def save(asset):
    assert u.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False), asset


def handle(name):
    return u.DataTableRowHandle(data_table=items, row_name=name)


def requirement(name, count, alternatives):
    value = u.RecipeItemRequirement()
    value.set_editor_properties(dict(item=handle(name), quantity=count,
        alternatives=[u.AlternativeItem(item=handle(n), quantity=q) for n, q in alternatives]))
    return value


def localized(key, source):
    TEXT.set_string_table_entry(strings, key, source)
    return TEXT.string_table_text(strings, key)


def inventory(resources):
    return [u.ItemIDEntry(item_id=item_rows[name]['ID'], amount=count) for name, count in resources]


try:
    OUT.mkdir(parents=True, exist_ok=True)
    for package in [RECIPE_PATH, TEXT_PATH]:
        relative = package.removeprefix(ROOT) + '.uasset'
        source = PROJECT / 'Plugins/InventorySystemInvenzaPlugin/Content' / relative
        backup = OUT / 'Before' / relative
        if source.exists() and not backup.exists():
            backup.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, backup)
    strings = u.load_asset(TEXT_PATH) if u.EditorAssetLibrary.does_asset_exist(TEXT_PATH) else (
        u.AssetToolsHelpers.get_asset_tools().create_asset('ST_CraftingExamples',
            ROOT + 'Data/Localization', u.StringTable, u.StringTableFactory()))
    assert strings
    # The existing item rows already reference these entries, but the table was missing.
    for key, source in {
        'Item.IronIngot.Name': 'Iron ingot',
        'Item.IronIngot.Description': 'Smelted from iron ore. Used to forge a sword.',
        'Item.HerbalExtract.Name': 'Herbal extract',
        'Item.HerbalExtract.Description': 'A concentrated plant reagent.',
    }.items():
        localized(key, source)
    save(strings)
    items = u.load_asset(TABLE_PATH)
    recipes = u.load_asset(RECIPE_PATH)
    item_before = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(items))
    recipe_before = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(recipes))
    item_rows = {row['Name']: row for row in item_before}
    original_bow = next(row for row in recipe_before if row['Name'] == 'Bow')

    # Storage geometry was already imported. Check every label against the item's own icon.
    storage_report = []
    for name, entry in item_rows.items():
        assert '(INVALID)' not in entry['ItemMetaData'], name
        meta = u.ItemMetaData()
        assert meta.import_text(entry['ItemMetaData']), name
        storage = meta.get_editor_property('item_storage_data')
        mesh = storage.get_editor_property('storage_mesh')
        assert mesh, name
        if name not in ['Log', 'IronOreBulk', 'Rope']:
            material = mesh.get_material(1)
            assert isinstance(material, u.MaterialInstanceConstant), name
            icon = meta.get_editor_property('item_asset_data').get_editor_property('icon')
            label_icon = u.MaterialEditingLibrary.get_material_instance_texture_parameter_value(material, 'ItemIcon')
            assert label_icon == icon, name
        if name in ['Paper', 'Money']:
            assert storage.get_editor_property('storage_method') == u.StorageMethod.SMALL_BOXES, name
        storage_report.append(dict(item=name, mesh=mesh.get_path_name(),
            method=str(storage.get_editor_property('storage_method'))))

    plans = [
        ('FieldBow', 'Field bow', 'Choose one wood option and one bowstring option. Both groups are required.',
            [('Board', 2, [('Log', 1)]), ('Rope', 1, [('Thread', 2), ('Vine', 3)])], ('Bow', 1), 6),
        ('BrewRedPotion', 'Brew red potion', 'Combine yellow potion and vines, then filter through paper.',
            [('YellowPotion', 1, []), ('Vine', 3, []), ('Paper', 1, [])], ('RedPotion', 2), 4),
        ('SmeltIron', 'Smelt iron ingot', 'Refine three units of iron ore into one iron ingot.',
            [('IronOreBulk', 3, [])], ('IronIngot', 1), 6),
        ('CraftSword', 'Forge sword', 'Use two iron ingots, a board and thread to make a sword.',
            [('IronIngot', 2, []), ('Board', 1, []), ('Thread', 1, [])], ('Sword_1', 1), 8),
    ]
    built = {}
    check_count = 0
    for name, title, description, inputs, output, duration in plans:
        assert name not in item_rows or name == 'FieldBow', name
        row = u.ItemRecipeRow()
        row.set_editor_properties(dict(id=name, display_name=localized('Recipe.' + name + '.Name', title),
            description=localized('Recipe.' + name + '.Description', description),
            required_items=[requirement(n, q, alternatives) for n, q, alternatives in inputs],
            output_items=[u.InitItemsEntry(item=handle(output[0]), amount=output[1])],
            craft_volume=float(duration)))
        meta = u.ItemMetaData()
        assert meta.import_text(item_rows[output[0]]['ItemMetaData'])
        row.set_editor_property('recipe_icon', meta.get_editor_property('item_asset_data').get_editor_property('icon'))
        groups = [[(n, q)] + alternatives for n, q, alternatives in inputs]
        for indices in itertools.product(*(range(len(group)) for group in groups)):
            resources = [group[index] for group, index in zip(groups, indices)]
            result = u.CraftingComponent.can_craft_with_items_options(row, inventory(resources), list(indices), 1)
            assert result.get_editor_property('can_craft'), (name, indices, result)
            check_count += 1
            # Every group is mandatory; insufficient and absent ingredients must block crafting.
            for missing in range(len(resources)):
                short = list(resources)
                n, q = short[missing]
                short[missing] = (n, q - 1)
                result = u.CraftingComponent.can_craft_with_items_options(row, inventory(short), list(indices), 1)
                assert not result.get_editor_property('can_craft'), (name, indices, missing)
                check_count += 1
        built[name] = row

    placeholders = []
    for row in recipe_before:
        if row['Name'].startswith('TestRecipe'):
            assert row['OutputItems'] and all('DataTable=None' in value and 'Amount=0' in value
                for value in row['OutputItems']), row['Name']
            placeholders.append(row['Name'])
    assert EDIT.remove_rows(recipes, placeholders) == len(placeholders)
    for name, row in built.items():
        assert EDIT.set_row_from_text(recipes, name, row.export_text()), name
    final = json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(recipes))
    assert next(row for row in final if row['Name'] == 'Bow') == original_bow
    assert {row['Name'] for row in final} == {'Bow'} | set(built)
    assert len({row['ID'] for row in final}) == len(final)
    assert json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(items)) == item_before
    save(strings)
    save(recipes)
    (OUT / 'recipes-after.json').write_text(json.dumps(final, indent=2), encoding='utf-8')
    (OUT / 'result.json').write_text(json.dumps(dict(removed=placeholders, added=list(built),
        preserved_bow=True, item_table_unchanged=True, storage=storage_report,
        native_crafting_checks=check_count, no_blueprint_or_level_saves=True), indent=2), encoding='utf-8')
    u.log('AGREED_CRAFTING_EXAMPLES_OK')
finally:
    u.SystemLibrary.quit_editor()
