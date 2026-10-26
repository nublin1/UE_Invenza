import unreal as u
from pathlib import Path
import shutil
path='/InventorySystemInvenzaPlugin/UI/Inventory/Workspace/WBP_InventoryWorkspace'
f=Path(u.Paths.project_dir())/'Plugins/InventorySystemInvenzaPlugin/Content/UI/Inventory/Workspace/WBP_InventoryWorkspace.uasset'
b=Path(u.Paths.project_dir())/'Saved/StorageRepair/WBP_InventoryWorkspace.uasset'
b.parent.mkdir(parents=True,exist_ok=True)
if not b.exists(): shutil.copy2(f,b)
bp=u.load_asset(path); tree=u.find_object(bp,'WidgetTree')
for name in ['Inventories','PlayerScroll']:
 w=u.find_object(tree,name); w.modify(); w.set_visibility(u.SlateVisibility.VISIBLE)
assert u.InvenzaWidgetEditingLibrary.compile_widget(bp)
assert u.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
