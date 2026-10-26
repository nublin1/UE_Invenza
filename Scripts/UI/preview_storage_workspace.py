"""Offscreen visual check of the shared storage window, with sample panels only."""
from pathlib import Path
import unreal as u
ROOT='/InventorySystemInvenzaPlugin/'
E=u.InvenzaWidgetEditingLibrary
world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
window=E.create_preview(world,u.load_class(None,ROOT+'UI/Inventory/Workspace/WBP_StorageAccess.WBP_StorageAccess_C'))
workspace=window.get_editor_property('workspace')
containers=[]
for name in ['Wood storage','Metal storage','Supplies','Player inventory']:
    container=E.create_preview(world,u.load_class(None,ROOT+'UI/InventoryContainer/WBP_InvContainer.WBP_InvContainer_C'))
    container.set_editor_property('title',u.Text(name))
    container.set_editor_property('is_show_close_button',False)
    container.set_editor_property('is_show_total_money',False)
    container.set_editor_property('is_show_weight',False)
    grid=E.create_preview(world,u.load_class(None,ROOT+'UI/Core/SimpleInvGrid/WBP_SimpleGrid_5x4.WBP_SimpleGrid_5x4_C'))
    container.get_editor_property('container_slot').add_child(grid)
    containers.append(container)
workspace.set_inventory_widgets(containers[:3],[u.Text(t) for t in ['Wood storage','Metal storage','Supplies']])
workspace.set_player_widget(containers[3])
out=Path(u.Paths.project_dir())/'Saved/StorageWorkspace'
out.mkdir(parents=True,exist_ok=True)
assert E.render_widget(window,u.Vector2D(1280,800),str(out/'workspace.png'))
workspace.get_editor_property('search_text').set_text(u.Text('metal'))
workspace.set_inventory_widgets(containers[:3],[u.Text(t) for t in ['Wood storage','Metal storage','Supplies']])
assert E.render_widget(window,u.Vector2D(1280,800),str(out/'search.png'))
workspace.get_editor_property('search_text').set_text(u.Text('missing'))
workspace.set_inventory_widgets(containers[:3],[u.Text(t) for t in ['Wood storage','Metal storage','Supplies']])
assert E.render_widget(window,u.Vector2D(1280,800),str(out/'empty.png'))
u.log('STORAGE_PREVIEW_OK')
E.release_preview()
u.SystemLibrary.quit_editor()

