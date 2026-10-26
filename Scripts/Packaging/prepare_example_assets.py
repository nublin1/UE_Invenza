"""Nublin Studio 2026 All Rights Reserved.

Prepare migrated example copies in a disposable host project, never in the source project.
"""
from pathlib import Path
import json
import unreal as u

PROJECT = Path(u.Paths.project_dir()).resolve()
SOURCE = Path('G:/UEProjects/UE_Invenza').resolve()
assert PROJECT != SOURCE, 'Example preparation is restricted to the disposable host.'
ROOT = '/InventorySystemInvenzaPlugin/'
OUT = SOURCE / 'Saved/FabExample20261025'
saved = []


def save(asset):
    assert u.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
    saved.append(asset.get_path_name().split('.')[0])


registry = u.AssetRegistryHelpers.get_asset_registry()
registry.search_all_assets(True)
manny = u.load_asset(ROOT + 'Characters/Mannequins/Meshes/SKM_Manny_Simple')
quinn = u.load_asset(ROOT + 'Characters/Mannequins/Meshes/SKM_Quinn')
ue4 = u.load_asset(ROOT + 'Characters/Mannequin_UE4/Meshes/SK_Mannequin')
assert manny and quinn and ue4
for record in registry.get_assets_by_path(ROOT + 'Characters', recursive=True):
    package = str(record.package_name)
    if str(record.asset_class_path.asset_name) not in ['SkeletalMesh', 'Skeleton', 'AnimSequence', 'PoseAsset']:
        continue
    asset = u.load_asset(package)
    preview = ue4 if '/Mannequin_UE4/' in package else quinn if '/Quinn/' in package or 'SKM_Quinn' in package else manny
    assert u.InvenzaExamplePackagingLibrary.refresh_migrated_preview(asset, preview), package
    save(asset)

character = u.load_asset(ROOT + 'Blueprints/Examples/BP_Character')
controller = u.load_asset(ROOT + 'Blueprints/Examples/Controllers/BP_PlayerController')
u.BlueprintEditorLibrary.reparent_blueprint(character, u.InvenzaExampleCharacter)
u.BlueprintEditorLibrary.reparent_blueprint(controller, u.InvenzaExamplePlayerController)
animation = u.load_class(None, ROOT + 'Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C')
channel = u.CollisionChannel.ECC_VISIBILITY
assert u.InvenzaExamplePackagingLibrary.configure_example_character(character, manny, animation, channel)
u.BlueprintEditorLibrary.compile_blueprint(character)
u.BlueprintEditorLibrary.compile_blueprint(controller)
save(character)
save(controller)

levels = u.get_editor_subsystem(u.LevelEditorSubsystem)
assert levels.load_level(ROOT + 'Maps/Map_Ex')
actors = u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors()
assert len([actor for actor in actors if isinstance(actor, u.LinkedStorageChest)]) == 4
for actor in actors:
    if actor.get_component_by_class(u.InteractableComponent):
        for component in actor.get_components_by_class(u.PrimitiveComponent):
            component.set_collision_response_to_channel(channel, u.CollisionResponseType.ECR_BLOCK)
    for component in actor.get_components_by_class(u.StorageGridVisualizer):
        component.set_editor_property('interaction_channel', channel)
assert levels.save_current_level()
saved.append(ROOT + 'Maps/Map_Ex')
(OUT / 'prepared.json').write_text(json.dumps(dict(saved_packages=saved, actors=len(actors),
    trace_channel=str(channel), demo_input_independent_of_project_settings=True), indent=2), encoding='utf-8')
u.log('PLAYABLE_EXAMPLE_PREPARED ' + json.dumps(dict(saved=len(saved), actors=len(actors))))
