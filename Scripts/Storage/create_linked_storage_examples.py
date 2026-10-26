"""Nublin Studio 2026 All Rights Reserved.

Create the new definition and three chest Blueprints; place examples in Map_Ex.
Run in the isolated validation project. Copy only the explicit output list back.
"""
from pathlib import Path
import json
import runpy
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
OUT = Path('G:/UEProjects/UE_Invenza/Saved/LinkedStorage20261024')
TOOLS = u.AssetToolsHelpers.get_asset_tools()
ASSETS = []


def save(asset):
    assert u.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False), asset
    ASSETS.append(asset.get_path_name().split('.')[0])


def create_blueprint(name, parent):
    path = ROOT + 'Blueprints/Examples/Storage/Linked/' + name
    assert not u.EditorAssetLibrary.does_asset_exist(path), path
    factory = u.BlueprintFactory()
    factory.set_editor_property('parent_class', parent)
    return TOOLS.create_asset(name, path.rsplit('/', 1)[0], None, factory)


factory = u.DataAssetFactory()
factory.set_editor_property('data_asset_class', u.LinkedStorageDefinition)
definition = TOOLS.create_asset('DA_LinkedStorageExample', ROOT + 'Data/Storage', None, factory)
assert definition
definition.set_editor_property('storage_group_id', 'ExampleStorage')
template_class = u.load_class(None, ROOT + 'Blueprints/Examples/Containers/BP_LootContainer.BP_LootContainer_C')
actor_subsystem = u.get_editor_subsystem(u.EditorActorSubsystem)
template_actor = actor_subsystem.spawn_actor_from_class(template_class, u.Vector())
template = template_actor.get_component_by_class(u.ContainerComponent)
settings = template.get_editor_property('startup_inventories')[0].settings
settings.set_editor_properties(dict(
    custom_inventory_id='', inventory_type=u.InventoryType.LOOT_CONTAINER,
    container_title=u.Text('Linked storage'), is_reference_container=False,
    allow_item_referencing=False, collect_inv_data_from_widget=False))
# Reuse all existing container/grid/operations widgets and their presentation defaults.
startup = u.InventoryStartupData(settings=settings)
table = u.load_asset(ROOT + 'Data/DT_ItemData')
rows = [str(row) for row in u.DataTableFunctionLibrary.get_data_table_row_names(table)]
starter_row = next(row for row in rows if row.lower() == 'vine')
startup.set_editor_property('start_items', [u.InitItemsEntry(
    item=u.DataTableRowHandle(data_table=table, row_name=starter_row), amount=3)])
definition.set_editor_property('inventory', startup)
save(definition)
actor_subsystem.destroy_actor(template_actor)

base = create_blueprint('BP_LinkedStorageChest', u.LinkedStorageChest)
cdo = u.get_default_object(base.generated_class())
cdo.mesh.set_static_mesh(u.load_asset(ROOT + 'Meshes/Chest/SM_Chest'))
interaction_channel = u.get_default_object(u.InvenzaInventoryDeveloperSettings).get_editor_property('interaction_trace_channel')
cdo.mesh.set_collision_response_to_channel(interaction_channel, u.CollisionResponseType.ECR_BLOCK)
cdo.linked_storage.set_editor_properties(dict(
    storage_definition=definition,
    window_class=u.load_class(None, ROOT + 'UI/Inventory/Workspace/WBP_StorageAccess.WBP_StorageAccess_C')))
u.BlueprintEditorLibrary.compile_blueprint(base)
save(base)

classes = {}
for name, mode, title in [
    ('BP_SharedStorageChest', u.LinkedStorageAccessMode.SHARED, 'Shared storage'),
    ('BP_PersonalStorageChest', u.LinkedStorageAccessMode.PERSONAL, 'Personal storage'),
]:
    bp = create_blueprint(name, base.generated_class())
    defaults = u.get_default_object(bp.generated_class())
    defaults.linked_storage.set_editor_properties(dict(storage_access_mode=mode, window_title=u.Text(title)))
    u.BlueprintEditorLibrary.compile_blueprint(bp)
    save(bp)
    classes[mode] = bp.generated_class()

runpy.run_path(str(Path(__file__).with_name('place_linked_storage_examples.py')))
ASSETS.append(ROOT + 'Maps/Map_Ex')

report = dict(created_assets=ASSETS, starter_row=starter_row, starter_amount=3,
              map_requires_project_game_mode=True)
(OUT / 'created-assets.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
u.log('LINKED_STORAGE_EXAMPLES_CREATED ' + json.dumps(report))
