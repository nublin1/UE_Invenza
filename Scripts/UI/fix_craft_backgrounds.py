"""Fix only the overlay slots introduced by restyle_craft_menu.py."""
from pathlib import Path
import json
import shutil
import unreal as u

project = Path(u.Paths.project_dir()).resolve()
targets = {
    'UI/Craft/WBP_CraftDashboard': ['CraftSurface', 'QueueArea'],
    'UI/Craft/Lists/WBP_QueueCraftList_Dashboard': ['EntrySurface'],
    'UI/Core/Status/WBP_StatusMessage': ['StatusSurface'],
}
report = []
for path, overlays in targets.items():
    disk = project / 'Plugins/InventorySystemInvenzaPlugin/Content' / (path + '.uasset')
    backup = project / 'Saved/CraftUI/BeforeBackgroundFix' / (path + '.uasset')
    if not backup.exists():
        backup.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(disk, backup)
    bp = u.load_asset('/InventorySystemInvenzaPlugin/' + path)
    assert bp, path
    bp.modify()
    tree = u.find_object(bp, 'WidgetTree')
    for name in overlays:
        overlay = u.find_object(tree, name)
        assert isinstance(overlay, u.Overlay), name
        for child in overlay.get_all_children():
            if child.get_name() == 'EmptyQueueLabel':
                continue  # Keep the centered empty-state label.
            slot = child.slot
            slot.modify()
            slot.set_horizontal_alignment(u.HorizontalAlignment.H_ALIGN_FILL)
            slot.set_vertical_alignment(u.VerticalAlignment.V_ALIGN_FILL)
            report.append(path + ':' + name + '/' + child.get_name())
    assert u.InvenzaWidgetEditingLibrary.compile_widget(bp), path
    assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False), path
(project / 'Saved/CraftUI/background-fix.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
u.log('CRAFT_BACKGROUNDS_FIXED')
