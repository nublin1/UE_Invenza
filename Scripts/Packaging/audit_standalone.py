# Nublin Studio 2026 All Rights Reserved.
"""Read-only dependency validation for the packaged plugin in an empty host project."""

from pathlib import Path
import json

import unreal as u


registry = u.AssetRegistryHelpers.get_asset_registry()
registry.search_all_assets(True)
options = u.AssetRegistryDependencyOptions(
    include_soft_package_references=True,
    include_hard_package_references=True,
    include_searchable_names=True,
    include_soft_management_references=True,
    include_hard_management_references=True,
)
root = "/InventorySystemInvenzaPlugin/"
packages = sorted({str(asset.package_name) for asset in registry.get_assets_by_path(root, recursive=True, include_only_on_disk_assets=True)})
external = []
for package in packages:
    dependencies = [str(value) for value in registry.get_dependencies(package, options)]
    forbidden = [value for value in dependencies if value.startswith(("/Game/", "/Script/UE_GridInventory", "/Script/InvenzaUIEditor"))]
    if forbidden:
        external.append({"package": package, "dependencies": forbidden})
assert not external, json.dumps(external, indent=2)
for package in [root + "Data/DT_ItemData", root + "Data/Settings/InvenzaSettings", root + "Data/Localization/ST_InvenzaSaveTexts", root + "Data/Localization/ST_InvenzaMenuPages", root + "Data/Localization/ST_InvenzaRuntimeTexts"]:
    assert u.load_asset(package), package
for package in [root + "UI/SaveLoad/WBP_InvenzaSaveMenu", root + "UI/SaveLoad/WBP_SaveGamePage", root + "UI/SaveLoad/WBP_LoadGamePage", root + "UI/Inventory/Character/WBP_CharacterInventoryExchange", root + "UI/Inventory/Workspace/WBP_StorageInventoryRow", root + "UI/Controls/WBP_ControlsHelpRow"]:
    assert u.load_class(None, package + "." + package.rsplit("/", 1)[1] + "_C"), package
assert root + "Maps/Map_Ex" not in packages
assert root + "Blueprints/Examples/BP_Character" not in packages
result = {"packages": len(packages), "forbidden_dependencies": external, "runtime_roots_loaded": True, "example_project_packages_excluded": True}
output = Path(u.Paths.project_saved_dir()) / "standalone_audit.json"
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(result, indent=2), encoding="utf-8")
u.log("INVENZA_STANDALONE_DEPENDENCIES_OK " + json.dumps(result))
