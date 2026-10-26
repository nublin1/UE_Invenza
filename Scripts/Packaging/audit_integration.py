# Nublin Studio 2026 All Rights Reserved.
"""Read-only configuration probe; run with Unreal's PythonScript commandlet.

Example script arguments: --channel ECC_CAMERA --profile IntegrationProbe
--font-size 23. No project assets are modified or saved.
"""

import argparse
import json
import re
from pathlib import Path

import unreal as u


parser = argparse.ArgumentParser()
parser.add_argument("--channel", default="ECC_VISIBILITY")
parser.add_argument("--profile", default="Default")
parser.add_argument("--font-size", type=int, default=15)
parser.add_argument("--tag-source", type=Path, default=Path(__file__).resolve().parents[2] /
                    "Plugins/InventorySystemInvenzaPlugin/Config/Tags/InvenzaTags.ini")
args = parser.parse_args()

settings = u.get_default_object(u.InvenzaInventoryDeveloperSettings)
saves = u.get_default_object(u.InvenzaSaveSettings)
expected_channel = getattr(u.CollisionChannel, args.channel)
assert settings.get_editor_property("interaction_trace_channel") == expected_channel
assert saves.get_editor_property("profile") == args.profile
assert settings.get_editor_property("help_row_font_size") == args.font_size
assert u.get_default_object(u.InteractionComponent).get_editor_property("trace_channel") == expected_channel
assert u.get_default_object(u.StorageGridVisualizer).get_editor_property("interaction_channel") == expected_channel

loaded = []
for owner, names in [
    (settings, ["inventory_settings_asset", "runtime_text_table", "example_equipment_table",
                "character_inventory_window_class", "storage_inventory_row_class",
                "controls_help_row_class", "default_ui_font", "default_storage_wall_mesh",
                "default_access_point_mesh"]),
    (saves, ["save_menu_class", "text_table"]),
]:
    for name in names:
        asset = owner.get_editor_property(name)
        assert asset, name
        loaded.append(asset.get_path_name())

for template_name in ["character_bag_template", "character_equipment_template"]:
    template = settings.get_editor_property(template_name)
    for name in ["inventory_class", "container_widget_class", "inventory_widget_class"]:
        assert template.get_editor_property(name), (template_name, name)
    slots = template.get_editor_property("inventory_slot_based_settings")
    assert slots.get_editor_property("slotbased_inventory_slot_class"), template_name

assert str(settings.get_text("StorageAccess.Title"))
tag_source = args.tag_source.read_text(encoding="utf-8-sig")
tag_names = re.findall(r'GameplayTagList=\(Tag="([^"]+)"', tag_source)
assert tag_names, args.tag_source
for name in tag_names:
    tag = u.GameplayTag()
    tag.import_text('(TagName="' + name + '")')
    assert str(u.GameplayTagLibrary.get_tag_name(tag)) == name, name
legacy_tag = u.GameplayTag()
legacy_tag.import_text('(TagName="Item.Category.Ingredients")')
assert str(u.GameplayTagLibrary.get_tag_name(legacy_tag)) == "Item.Category.Ingredient"
result = {"channel": args.channel, "profile": args.profile, "font_size": args.font_size,
          "native_component_channels_match": True, "loaded_config_assets": loaded,
          "registered_tags": len(tag_names), "legacy_tag_redirect": True}
output = Path(u.Paths.project_saved_dir()) / "integration_audit.json"
output.write_text(json.dumps(result, indent=2), encoding="utf-8")
u.log("INVENZA_INTEGRATION_CONFIG_OK " + json.dumps(result))
