# Nublin Studio 2026 All Rights Reserved.
"""Author only the three newly scoped UI templates. Run with Unreal Python."""
from pathlib import Path
import unreal as u

source=(Path(u.Paths.project_dir())/'Scripts/UI/restyle_craft_choose.py').read_text(encoding='utf-8')
exec(source.split('# A reusable wrapping label')[0])
BUTTON=u.load_class(None,ROOT+'UI/Core/Buttons/WBP_UIButtonWithSlot.WBP_UIButtonWithSlot_C')
WRAPPED=u.load_class(None,ROOT+'UI/Core/WBP_LabelBaseText_Wrapped.WBP_LabelBaseText_Wrapped_C')
TABLE=u.load_asset(ROOT+'Data/Localization/ST_InvenzaRuntimeTexts')
assert TABLE

def text(key):
    return EDIT.string_table_text(TABLE,key)

def new_widget(relative, parent):
    if u.EditorAssetLibrary.does_asset_exist(ROOT+relative):
        assert relative in ['UI/Controls/WBP_ControlsHelpRow','UI/Inventory/Workspace/WBP_StorageInventoryRow','UI/Inventory/Character/WBP_CharacterInventoryExchange']
        return u.load_asset(ROOT+relative)
    folder,name=relative.rsplit('/',1)
    factory=u.WidgetBlueprintFactory()
    factory.set_editor_property('parent_class',parent)
    bp=u.AssetToolsHelpers.get_asset_tools().create_asset(name,ROOT+folder,None,factory)
    assert bp
    return bp

def caption(bp,name,key,points=17,bold=False):
    result=label(bp,name,None,points,bold)
    result.set_editor_property('text',text(key))
    return result

bp=new_widget('UI/Controls/WBP_ControlsHelpRow',u.ControlsHelpRowWidget)
body=panel(bp,u.HorizontalBox,'HelpRow')
for name,justify,padding in [('ActionLabel',u.TextJustify.LEFT,margin(0,0,24,0)),('KeysLabel',u.TextJustify.RIGHT,margin(0))]:
    w=style_label(make(bp,WRAPPED,name),None,15)
    w.set_editor_property('text',u.Text(''))
    # Reuse the existing wrapping Core label without changing its Blueprint.
    slot=attach(body,w,padding,True)
    if name=='KeysLabel':
        slot.set_horizontal_alignment(H.H_ALIGN_RIGHT)
EDIT.set_root(bp,body)
save(bp)

bp=new_widget('UI/Inventory/Workspace/WBP_StorageInventoryRow',u.InventoryWorkspaceRowWidget)
body=panel(bp,u.VerticalBox,'StorageRow')
button=make(bp,BUTTON,'ToggleButton')
button.set_visibility(VIS.VISIBLE)
header=panel(bp,u.HorizontalBox,'StorageHeader')
arrow=make(bp,u.Image,'Arrow')
attach(header,size(bp,'ArrowSize',arrow,12,12),margin(4,0,8,0)).set_vertical_alignment(V.V_ALIGN_CENTER)
attach(header,label(bp,'TitleLabel',None,17,True)).set_vertical_alignment(V.V_ALIGN_CENTER)
EDIT.set_slot_content(button,'NamedSlot_21',header)
attach(body,size(bp,'HeaderSize',button,min_height=32))
attach(body,make(bp,u.NamedSlot,'InventoryContent'),margin(0,0,0,0))
EDIT.set_root(bp,body)
assert EDIT.compile_widget(bp)
cdo=u.get_default_object(bp.generated_class())
for prop,brush_name in [('expanded_brush','TreeArrow_Expanded'),('collapsed_brush','TreeArrow_Collapsed')]:
    brush=EDIT.get_core_style_brush(brush_name)
    brush.set_editor_property('tint_color',u.SlateColor(specified_color=GOLD))
    cdo.set_editor_property(prop,brush)
save(bp)

bp=new_widget('UI/Inventory/Character/WBP_CharacterInventoryExchange',u.CharacterInventoryExchangeWidget)
body=panel(bp,u.VerticalBox,'ExchangeBody')
header=panel(bp,u.HorizontalBox,'ExchangeHeader')
attach(header,caption(bp,'CharacterLabel','Invenza.InventoryMannequin',24,True),fill=True).set_vertical_alignment(V.V_ALIGN_CENTER)
button=make(bp,BUTTON,'CloseButton')
button.set_visibility(VIS.VISIBLE)
EDIT.set_slot_content(button,'NamedSlot_21',caption(bp,'CloseCaption','Invenza.CloseCharacterInventory',17,True))
attach(header,size(bp,'CloseSize',button,100,38))
attach(body,header,margin(0,0,0,12))
columns=panel(bp,u.HorizontalBox,'ExchangeColumns')
for name,heading_key in [('Other','Invenza.OtherCharacterEquipment'),('Player','Invenza.PlayerCharacterEquipment')]:
    column=panel(bp,u.VerticalBox,name+'Column')
    attach(column,caption(bp,name+'Heading',heading_key,17,True),margin(0,0,0,10))
    inventories=panel(bp,u.HorizontalBox,name+'Inventories')
    scale=panel(bp,u.ScaleBox,name+'InventoryScale')
    scale.set_stretch(u.Stretch.SCALE_TO_FIT)
    scale.set_stretch_direction(u.StretchDirection.DOWN_ONLY)
    attach(scale,inventories)
    scroll=panel(bp,u.ScrollBox,name+'Scroll')
    scroll.set_visibility(VIS.VISIBLE)
    attach(scroll,size(bp,name+'ContentSize',scale,596,570))
    attach(column,scroll,fill=True)
    attach(columns,column,margin(0,0,12 if name=='Other' else 0,0),True)
attach(body,columns,fill=True)
content=surface(bp,'ExchangeSurface',body,BG,18)
fixed=size(bp,'ExchangeSize',content,1280,720)
scale=panel(bp,u.ScaleBox,'ExchangeScale')
scale.set_stretch(u.Stretch.SCALE_TO_FIT)
scale.set_stretch_direction(u.StretchDirection.DOWN_ONLY)
attach(scale,fixed)
EDIT.set_root(bp,scale)
assert EDIT.compile_widget(bp)
u.get_default_object(bp.generated_class()).set_editor_property('is_focusable',True)
save(bp)
u.log('INVENZA_DESIGNER_TEMPLATES_OK')

