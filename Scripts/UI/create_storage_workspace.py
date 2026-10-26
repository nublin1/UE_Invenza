"""Create the shared storage workspace and migrate CraftDashboard. Run in Unreal Python."""
from pathlib import Path
import unreal as u
# Reuse the established Core styling helpers, without running the recipe migration.
source = (Path(u.Paths.project_dir())/'Scripts/UI/restyle_craft_choose.py').read_text(encoding='utf-8')
exec(source.split('# A reusable wrapping label')[0])
WRAPPED = LABEL
BUTTON = u.load_class(None, ROOT+'UI/Core/Buttons/WBP_UIButtonWithSlot.WBP_UIButtonWithSlot_C')

def create(path, parent, widget_bp=True):
    if u.EditorAssetLibrary.does_asset_exist(ROOT+path): return asset(path)
    factory = u.WidgetBlueprintFactory() if widget_bp else u.BlueprintFactory()
    factory.set_editor_property('parent_class', parent)
    directory, name = (ROOT+path).rsplit('/',1)
    bp = u.AssetToolsHelpers.get_asset_tools().create_asset(name, directory, None, factory)
    assert bp, path
    return bp

def button(bp,name,text,width=100):
    w=make(bp,BUTTON,name)
    w.set_visibility(VIS.VISIBLE)
    content=label(bp,name+'Label',text,15,True)
    EDIT.set_slot_content(w,'NamedSlot_21',size(bp,name+'ContentSize',content,width,32))
    return w

heading=variant('UI/Core/WBP_LabelBaseText','UI/Inventory/Workspace/WBP_StorageInventoryTitle')
style_label(u.get_default_object(heading.generated_class()),'Storage',17,True)
save(heading); HEADING=heading.generated_class()

bp=create('UI/Inventory/Workspace/WBP_InventoryWorkspace',u.InventoryWorkspaceWidget)
root=panel(bp,u.HorizontalBox,'WorkspaceColumns')
left=panel(bp,u.VerticalBox,'StorageColumn')
attach(root,left,margin(0,0,20,0),True)
searchrow=panel(bp,u.HorizontalBox,'SearchRow')
search=make(bp,u.EditableTextBox,'SearchText'); search.set_visibility(VIS.VISIBLE)
search.set_hint_text(u.Text('Find storage by name'))
# Copy the existing Core inventory search style.
filters=u.load_asset(ROOT+'UI/Core/ItemFiltersPanel/WBP_ItemFiltersPanel')
style=search.get_editor_property('widget_style')
textstyle=style.get_editor_property('text_style')
textstyle.set_editor_property('font',BASE_FONT)
textstyle.set_editor_property('color_and_opacity',u.SlateColor(specified_color=GOLD))
style.set_editor_property('text_style',textstyle)
for field in ['background_image_normal','background_image_hovered','background_image_focused']:
    style.set_editor_property(field,u.SlateBrush(draw_as=u.SlateBrushDrawType.BOX,tint_color=u.SlateColor(specified_color=PANEL)))
style.set_editor_property('foreground_color',u.SlateColor(specified_color=GOLD))
search.set_editor_property('widget_style',style)
attach(searchrow,search,margin(0,0,8,0),True)
attach(searchrow,button(bp,'ClearSearch','Clear',62))
attach(left,searchrow,margin(4,4,4,12))
attach(left,label(bp,'EmptyLabel','No storages available',15,color=MUTED),margin(8))
scroll=panel(bp,u.ScrollBox,'Inventories')
scroll.set_visibility(VIS.VISIBLE)
attach(left,scroll,0,True)
player=panel(bp,u.VerticalBox,'PlayerColumn')
attach(player,label(bp,'PlayerHeading','Inventory',17,True),margin(0,4,0,12))
playerscroll=panel(bp,u.ScrollBox,'PlayerScroll')
playerscroll.set_visibility(VIS.VISIBLE)
attach(playerscroll,make(bp,u.NamedSlot,'PlayerSlot'))
attach(player,playerscroll,0,True)
attach(root,size(bp,'PlayerColumnSize',player,width=330))
EDIT.set_root(bp,root); assert EDIT.compile_widget(bp)
u.get_default_object(bp.generated_class()).set_editor_property('label_class',HEADING)
save(bp); WORKSPACE=bp.generated_class()

bp=create('UI/Inventory/Workspace/WBP_StorageAccess',u.StorageAccessWidget)
body=panel(bp,u.VerticalBox,'StorageBody')
header=panel(bp,u.HorizontalBox,'StorageHeader')
attach(header,label(bp,'TitleLabel','Base storage',22,True),margin(0,0,16,0),True)
attach(header,button(bp,'RefreshButton','Refresh',100),margin(0,0,12,0))
attach(header,button(bp,'CloseButton','Close',70))
attach(body,header,margin(0,0,0,16))
attach(body,make(bp,WORKSPACE,'Workspace'),0,True)
content=frame(bp,'StorageFrame',surface(bp,'StorageSurface',body,BG,16))
content=size(bp,'StorageWindowSize',content,1040,620)
scale=panel(bp,u.ScaleBox,'ScreenFit'); scale.set_stretch(u.Stretch.SCALE_TO_FIT); attach(scale,content)
canvas=panel(bp,u.CanvasPanel,'StorageCanvas')
slot=canvas.add_child(scale); slot.set_anchors(u.Anchors(u.Vector2D(0,0),u.Vector2D(1,1))); slot.set_offsets(u.Margin(28,28,28,28))
EDIT.set_root(bp,canvas); save(bp); WINDOW=bp.generated_class()

# Preserve queue and control widgets. Replace only the station/player columns.
bp=asset('UI/Craft/WBP_CraftDashboard')
columns=widget(bp,'CraftColumns')
for child in list(columns.get_all_children())[1:]: child.remove_from_parent()
attach(columns,make(bp,WORKSPACE,'InventoryWorkspace'),0,True)
save(bp)

bp=create('Blueprints/Examples/Storage/BP_StorageAccessPoint',u.StorageAccessPoint,False)
u.BlueprintEditorLibrary.compile_blueprint(bp)
cdo=u.get_default_object(bp.generated_class())
access=cdo.get_editor_property('storage_access')
access.set_editor_property('window_class',WINDOW)
cdo.get_editor_property('mesh').set_relative_scale3d(u.Vector(.65,.45,1.0))
u.BlueprintEditorLibrary.compile_blueprint(bp)
assert u.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
u.log('STORAGE_WORKSPACE_ASSETS_OK')




