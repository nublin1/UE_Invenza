# Nublin Studio 2026 All Rights Reserved.
"""Read-only style/graph audit with scoped backups before the full UI theme migration."""
from pathlib import Path
import json
import shutil
import unreal as u

PROJECT = Path(u.Paths.project_dir())
OUT = PROJECT/'Saved/UITheme20261031'
OUT.mkdir(parents=True, exist_ok=True)
registry = u.AssetRegistryHelpers.get_asset_registry()
roots = ['/InventorySystemInvenzaPlugin/UI', '/Game/Frontend/UI']
registry.scan_paths_synchronous(roots, force_rescan=True)
result = []
for root in roots:
    for data in registry.get_assets_by_path(root, recursive=True):
        if str(data.asset_class_path.asset_name) != 'WidgetBlueprint':
            continue
        asset = u.load_asset(str(data.package_name))
        if not isinstance(asset, u.WidgetBlueprint):
            continue
        name = str(data.package_name)
        relative = (Path('Plugins/InventorySystemInvenzaPlugin/Content') / name.removeprefix('/InventorySystemInvenzaPlugin/')
                    if name.startswith('/InventorySystemInvenzaPlugin/') else Path('Content') / name.removeprefix('/Game/'))
        source = PROJECT / (str(relative) + '.uasset')
        backup = OUT/'before'/source.relative_to(PROJECT)
        if not backup.exists():
            backup.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, backup)
        record = json.loads(u.InvenzaUIThemeEditingLibrary.inspect_blueprint(asset))
        record['package'] = name
        record['file'] = str(source.relative_to(PROJECT)).replace('\\', '/')
        result.append(record)
        u.log('UI_THEME_AUDIT ' + name)
(OUT/'audit-before.json').write_text(json.dumps(result, indent=2, ensure_ascii=False), encoding='utf-8')
(OUT/'asset-scope.md').write_text('# UI theme migration scope\n\n' + '\n'.join('- `' + entry['package'] + '`' for entry in result)
                              + '\n\nNew assets: `Data/UI/DA_UITheme_Default`, `UI/Theme/WBP_UIThemePreview`.\n', encoding='utf-8')
u.log('UI_THEME_AUDIT_COMPLETE ' + str(len(result)))
