#  Nublin Studio 2026 All Rights Reserved.
"""Assign persistent identities only to instances in Map_Ex; save no Blueprint or Data Asset."""
from pathlib import Path
import shutil
import unreal as u

root = '/InventorySystemInvenzaPlugin/Maps/Map_Ex'
project = Path(u.Paths.project_dir())
backup = project/'Saved/SessionSaveFix/Map_Ex-before-identities.umap'
if not backup.exists():
    shutil.copy2(project/'Plugins/InventorySystemInvenzaPlugin/Content/Maps/Map_Ex.umap', backup)
level = u.get_editor_subsystem(u.LevelEditorSubsystem)
assert level.load_level(root)
zero = u.Guid()
seen = set()
changed = 0
for actor in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors():
    for cls, prop in [(u.InventoryCollectionComponent, 'persistent_owner_id'), (u.PickupComponent, 'persistent_pickup_id')]:
        for component in actor.get_components_by_class(cls):
            value = component.get_editor_property(prop)
            key = value.export_text()
            if value == zero or key in seen:
                value = u.GuidLibrary.new_guid()
                component.set_editor_property(prop, value)
                key = value.export_text()
                changed += 1
            seen.add(key)
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
assert u.EditorLoadingAndSavingUtils.save_map(world, root)
u.log('INVENZA_PERSISTENT_IDENTITIES_SAVED '+str(changed))
