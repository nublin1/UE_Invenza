"""Restyle the recipe picker with project Core widgets. Run in Unreal Python.

Existing assets are backed up under Saved/CraftUI/BeforeChoose before saving.
No shared button assets are changed. Local variants keep other menus intact.
"""
from pathlib import Path
import shutil
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir()).resolve()
EDIT = u.InvenzaWidgetEditingLibrary
u.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([ROOT], force_rescan=True)
H, V, VIS = u.HorizontalAlignment, u.VerticalAlignment, u.SlateVisibility
WHITE = u.LinearColor.WHITE
BG = u.LinearColor(.024, .016, .009, 1)
PANEL = u.LinearColor(.035, .024, .013, 1)
CARD = u.LinearColor(.065, .045, .025, 1)
LINE = u.LinearColor(.20, .14, .075, 1)
GOLD = u.LinearColor(.82, .65, .36, 1)
MUTED = u.LinearColor(.48, .38, .25, 1)

def backup(path):
    disk = PROJECT/'Plugins/InventorySystemInvenzaPlugin/Content'/(path+'.uasset')
    dest = PROJECT/'Saved/CraftUI/BeforeChoose'/(path+'.uasset')
    if disk.exists() and not dest.exists():
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(disk, dest)

def asset(path):
    backup(path)
    bp = u.load_asset(ROOT+path)
    assert bp, path
    bp.modify()
    return bp

def variant(source, target):
    if not u.EditorAssetLibrary.does_asset_exist(ROOT+target):
        assert u.EditorAssetLibrary.duplicate_asset(ROOT+source, ROOT+target)
    return asset(target)

def widget(bp, name):
    result = u.find_object(u.find_object(bp, 'WidgetTree'), name)
    assert result, bp.get_name()+': '+name
    result.modify()
    return result

def make(bp, cls, name):
    w = EDIT.add_template(bp, cls, name)
    assert w, name
    w.modify()
    w.set_visibility(VIS.SELF_HIT_TEST_INVISIBLE)
    return w

def margin(l=0, t=None, r=None, b=None):
    return u.Margin(l, l if t is None else t, l if r is None else r, l if b is None else b)

def attach(parent, child, pad=0, fill=False):
    child.remove_from_parent()
    slot = parent.add_child(child)
    if hasattr(slot, 'set_padding'): slot.set_padding(pad if isinstance(pad,u.Margin) else margin(pad))
    if hasattr(slot, 'set_horizontal_alignment'): slot.set_horizontal_alignment(H.H_ALIGN_FILL)
    if hasattr(slot, 'set_vertical_alignment'): slot.set_vertical_alignment(V.V_ALIGN_FILL)
    if isinstance(slot,(u.VerticalBoxSlot,u.HorizontalBoxSlot)):
        slot.set_size(u.SlateChildSize(1.,u.SlateSizeRule.FILL if fill else u.SlateSizeRule.AUTOMATIC))
    return slot

def panel(bp, cls, name):
    w = make(bp,cls,name)
    w.clear_children()
    return w

def size(bp,name,child,width=None,height=None,min_height=None):
    box=panel(bp,u.SizeBox,name)
    box.clear_width_override(); box.clear_height_override()
    if width is not None: box.set_width_override(width)
    if height is not None: box.set_height_override(height)
    if min_height is not None: box.set_min_desired_height(min_height)
    attach(box,child)
    return box

def save(bp):
    assert EDIT.compile_widget(bp), bp.get_name()
    assert u.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)

LABEL=u.load_class(None,ROOT+'UI/Core/WBP_LabelBaseText.WBP_LabelBaseText_C')
IMAGE=u.load_class(None,ROOT+'UI/Core/WBP_BaseImage.WBP_BaseImage_C')
FRAME=u.load_class(None,ROOT+'UI/Core/WBP_Border4SideWithSlot.WBP_Border4SideWithSlot_C')
BASE_FONT=widget(u.load_asset(ROOT+'UI/InventoryContainer/WBP_InvContainer'),'InvMoney').get_editor_property('font_info')

def style_label(w,text=None,font_size=16,bold=False,color=GOLD):
    font=BASE_FONT.copy(); font.size=font_size; font.typeface_font_name='Bold' if bold else 'Regular'
    w.set_editor_properties(dict(font_info=font,color=u.SlateColor(specified_color=color)))
    if text is not None: w.set_editor_property('text',u.Text(text))
    w.set_visibility(VIS.HIT_TEST_INVISIBLE)
    return w

def label(bp,name,text,font_size=16,bold=False,color=GOLD,wrapped=False):
    return style_label(make(bp,WRAPPED if wrapped else LABEL,name),text,font_size,bold,color)

def background(bp,name,color):
    w=make(bp,IMAGE,name)
    w.set_editor_property('base_material',None)
    w.set_editor_property('brush_style',u.UIBrushStyle(
        brush=u.SlateBrush(draw_as=u.SlateBrushDrawType.BOX,tint_color=u.SlateColor(specified_color=WHITE)),brush_color=color))
    w.set_visibility(VIS.HIT_TEST_INVISIBLE)
    return w

def surface(bp,name,child,color=PANEL,pad=12):
    over=panel(bp,u.Overlay,name)
    attach(over,background(bp,name+'Background',color))
    attach(over,child,pad)
    return over

def frame(bp,name,child):
    w=make(bp,FRAME,name)
    style=u.BorderFrameStyle(brush=u.SlateBrush(draw_as=u.SlateBrushDrawType.BOX,tint_color=u.SlateColor(specified_color=WHITE)),
        brush_color=LINE,padding=margin(1),desired_size=u.Vector2D(1,1))
    for side in ['top','bottom','left','right']: w.set_editor_property(side+'_border_style',style)
    child.remove_from_parent(); EDIT.set_slot_content(w,'NamedSlot',child)
    return w

# A reusable wrapping label based on the existing Core Blueprint.
bp=variant('UI/Core/WBP_LabelBaseText','UI/Core/WBP_LabelBaseText_Wrapped')
widget(bp,'MainTextBlock').set_auto_wrap_text(True)
save(bp); WRAPPED=bp.generated_class()

# Explicit wrapping gives ListView the correct row height on its first layout pass.
bp=variant('UI/Core/WBP_LabelBaseText_Wrapped','UI/Craft/Lists/WBP_RecipeNameText')
widget(bp,'MainTextBlock').set_auto_wrap_text(False)
widget(bp,'MainTextBlock').set_editor_property('wrap_text_at',150.)
save(bp); RECIPE_NAME=bp.generated_class()

# Local title bar retains the existing close button and dragging behavior.
bp=variant('UI/Core/TitleBar/WBP_MovableTitleBar','UI/Craft/WBP_CraftChooseTitleBar')
row=panel(bp,u.HorizontalBox,'HeaderCanvasPanel')
title=style_label(widget(bp,'TitleName'),'Add crafting task',22,True)
attach(row,title,margin(0,4,16,4),True).set_vertical_alignment(V.V_ALIGN_CENTER)
close=widget(bp,'Button_Close')
attach(row,size(bp,'CloseSize',close,32,32)).set_vertical_alignment(V.V_ALIGN_CENTER)
EDIT.set_root(bp,row); save(bp); TITLE=bp.generated_class()

# Craft-only filter panel: inventory categories do not filter recipes yet.
bp=variant('UI/Core/ItemFiltersPanel/WBP_ItemFiltersPanel','UI/Craft/Lists/WBP_RecipeSearch')
search=widget(bp,'SearchText')
search.set_hint_text(u.Text('Find recipe'))
font=BASE_FONT.copy(); font.size=15; font.typeface_font_name='Regular'
search.set_font(font)
search_style=search.get_editor_property('widget_style')
search_style.color_and_opacity=u.SlateColor(specified_color=GOLD)
search.set_editor_property('widget_style',search_style)
row=panel(bp,u.HorizontalBox,'RecipeSearchRow')
attach(row,size(bp,'SearchIconSize',widget(bp,'WBP_BaseImage'),18,18),margin(0,0,8,0)).set_vertical_alignment(V.V_ALIGN_CENTER)
attach(row,search,fill=True).set_vertical_alignment(V.V_ALIGN_CENTER)
attach(row,size(bp,'ClearSearchSize',widget(bp,'ClearSearchTextButton'),22,22),margin(6,0,0,0)).set_vertical_alignment(V.V_ALIGN_CENTER)
EDIT.set_root(bp,frame(bp,'SearchFrame',surface(bp,'SearchSurface',row,BG,10)))
save(bp)
cdo=u.get_default_object(bp.generated_class())
cdo.set_editor_property('is_show_filters_buttons',False)
cdo.set_editor_property('search_in_filtered_slots',False)
u.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
SEARCH=bp.generated_class()

# Recipe rows: a subtle background, fixed icon, wrapping title and selection stripe.
bp=asset('UI/Craft/Lists/WBP_RecipeListEntry')
row=panel(bp,u.HorizontalBox,'RecipeRow')
icon=widget(bp,'ListEntry_Image')
attach(row,frame(bp,'IconFrame',surface(bp,'IconSurface',size(bp,'RecipeIconSize',icon,42,42),BG,4)),margin(0,0,12,0)).set_vertical_alignment(V.V_ALIGN_CENTER)
title=style_label(make(bp,RECIPE_NAME,'ListEntry_Text'),'Recipe name',16,True)
title.set_visibility(VIS.SELF_HIT_TEST_INVISIBLE)
attach(row,title,fill=True).set_vertical_alignment(V.V_ALIGN_CENTER)
over=panel(bp,u.Overlay,'RecipeRowLayers')
# Give the table row a hit-testable surface, including empty padding.
over.set_visibility(VIS.VISIBLE)
attach(over,background(bp,'RecipeRowBackground',CARD))
for name,color in [('HoverHighlight',u.LinearColor(.10,.073,.035,.55)),('SelectionHighlight',u.LinearColor(.18,.12,.05,.60))]:
    highlight=background(bp,name,color); highlight.set_render_opacity(0); attach(over,highlight)
attach(over,row,margin(14,10,12,10))
stripe=background(bp,'SelectionIndicator',GOLD); stripe.set_render_opacity(0)
attach(over,size(bp,'SelectionStripeSize',stripe,3)).set_horizontal_alignment(H.H_ALIGN_LEFT)
EDIT.set_root(bp,frame(bp,'RecipeRowFrame',over)); save(bp)
u.get_default_object(bp.generated_class()).set_padding(margin(0,0,0,8))
u.get_default_object(bp.generated_class()).set_visibility(VIS.SELF_HIT_TEST_INVISIBLE)
u.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)

bp=asset('UI/Craft/Lists/WBP_CraftRecipesList')
body=panel(bp,u.VerticalBox,'RecipeListBody')
attach(body,make(bp,SEARCH,'ItemFiltersPanel'),margin(0,0,0,12))
over=panel(bp,u.Overlay,'RecipeListArea')
attach(over,widget(bp,'ObjectList'),margin(0,0,4,0))
empty=label(bp,'EmptyRecipesText','No recipes available',15,color=MUTED,wrapped=True)
slot=attach(over,empty,20); slot.set_vertical_alignment(V.V_ALIGN_CENTER)
attach(body,over,fill=True)
EDIT.set_root(bp,body); save(bp)

# Local quantity selector keeps all existing button instances and their icons.
bp=variant('UI/Core/QuantitySelectors/WBP_CraftingQuantitySelector','UI/Craft/WBP_CraftChooseQuantity')
row=panel(bp,u.HorizontalBox,'QuantityRow')
for name,tip in [('Btn_SetMin','Minimum'),('Btn_Decrease','Decrease quantity'),('CurrentQuantityText',None),('Btn_Increase','Increase quantity'),('Btn_SetMax','Maximum from available resources')]:
    w=widget(bp,name)
    if tip:
        w.set_tool_tip_text(u.Text(tip))
        # Existing arrow textures face left; mirror the increase/max instances.
        w.set_render_scale(u.Vector2D(-1 if name in ['Btn_Increase','Btn_SetMax'] else 1,1))
    else:
        font=BASE_FONT.copy(); font.size=18
        w.set_editor_properties(dict(font_info=font,text=u.Text('1')))
    attach(row,size(bp,name+'Size',w,72 if not tip else 32,36),margin(3,0,3,0))
EDIT.set_root(bp,frame(bp,'QuantityFrame',surface(bp,'QuantitySurface',row,BG,6)))
save(bp); QUANTITY=bp.generated_class()

bp=asset('UI/Craft/WBP_CraftMenuActionBtns')
btn=make(bp,u.load_class(None,ROOT+'UI/Core/Buttons/WBP_UIButton.WBP_UIButton_C'),'Btn_Craft')
btn.set_visibility(VIS.SELF_HIT_TEST_INVISIBLE)
btn.set_editor_property('default_text',u.Text('Add to queue'))
btn.set_tool_tip_text(u.Text('Add the selected recipe and quantity to the production queue'))
EDIT.set_root(bp,frame(bp,'CraftActionFrame',surface(bp,'CraftActionSurface',size(bp,'CraftActionSize',btn,min_height=40),CARD,8)))
save(bp)

# A scoped counter preserves the existing Need / Have convention used by C++.
bp=variant('UI/Core/Progress/WBP_RemainingCounter','UI/Craft/Ingredient/WBP_IngredientAmounts')
row=panel(bp,u.HorizontalBox,'AmountRow')
for name,text in [('CurrentValue','1'),('ValueSeparator',' / '),('MaxValue','0')]:
    w=style_label(widget(bp,name),text,14) if name!='ValueSeparator' else label(bp,name,text,14,color=MUTED)
    attach(row,w)
EDIT.set_root(bp,row); save(bp); COUNTER=bp.generated_class()

for path,alternative in [('UI/Craft/Ingredient/WBP_RequirementOptionEntry',False),('UI/Craft/Ingredient/WBP_RequirementOptionEntryAlt',True)]:
    bp=asset(path)
    row=panel(bp,u.HorizontalBox,'IngredientRow')
    if alternative: attach(row,label(bp,'AlternativeLabel','OR',11,True,color=MUTED),margin(0,0,8,0)).set_vertical_alignment(V.V_ALIGN_CENTER)
    attach(row,size(bp,'IngredientIconSize',widget(bp,'IngredientIcon'),32,32),margin(0,0,10,0)).set_vertical_alignment(V.V_ALIGN_CENTER)
    info=panel(bp,u.VerticalBox,'IngredientInfo')
    attach(info,label(bp,'RequiredItemName','Ingredient',15,wrapped=True))
    attach(info,label(bp,'MissingResourcesText','Not enough resources',11,color=u.LinearColor(.64,.23,.14,1)),margin(0,3,0,0))
    attach(row,info,fill=True).set_vertical_alignment(V.V_ALIGN_CENTER)
    counter=make(bp,COUNTER,'RemainingCounter')
    counter.set_visibility(VIS.HIT_TEST_INVISIBLE)
    attach(row,size(bp,'AmountSize',counter,96),margin(12,0,0,0)).set_vertical_alignment(V.V_ALIGN_CENTER)
    # Retain the existing button class, with its named-slot content and toggle behavior.
    button=widget(bp,'MainButton')
    content=surface(bp,'IngredientSurface',row,PANEL,10)
    row.remove_from_parent()
    selected=background(bp,'SelectionBackground',CARD); selected.set_render_opacity(0)
    attach(content,selected)
    attach(content,row,10)
    marker=background(bp,'SelectedMarker',GOLD); marker.set_render_opacity(0)
    attach(content,size(bp,'SelectedMarkerSize',marker,3)).set_horizontal_alignment(H.H_ALIGN_LEFT)
    EDIT.set_slot_content(button,'NamedSlot_21',content)
    EDIT.set_root(bp,button); save(bp)

bp=asset('UI/Craft/Lists/WBP_ReceptIDetailListEntryWidget')
EDIT.set_root(bp,frame(bp,'RequirementGroupFrame',widget(bp,'RequirementsContainer')))
save(bp)
u.get_default_object(bp.generated_class()).set_padding(margin(0,0,0,10))
u.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
bp=asset('UI/Craft/Lists/WBP_ReceptDetailRequiredListSimple')
widget(bp,'SizeBox_0').clear_width_override(); save(bp)

# Detail panel. Only requirements scroll; quantity/action footer remains visible.
bp=asset('UI/Craft/WBP_CraftMenuDetail')
body=panel(bp,u.VerticalBox,'DetailBody')
header=panel(bp,u.HorizontalBox,'RecipeHeader')
attach(header,frame(bp,'PreviewFrame',surface(bp,'PreviewSurface',size(bp,'PreviewSize',widget(bp,'RecipeImage'),72,72),BG,10)),margin(0,0,16,0)).set_vertical_alignment(V.V_ALIGN_TOP)
info=panel(bp,u.VerticalBox,'RecipeHeading')
attach(info,label(bp,'RecipeName','Recipe name',24,True,wrapped=True),margin(0,0,0,6))
description_scroll=panel(bp,u.ScrollBox,'DescriptionScroll')
attach(description_scroll,label(bp,'RecipeDescription','Select a recipe to view its description.',14,color=MUTED,wrapped=True))
description_size=size(bp,'DescriptionMaxSize',description_scroll)
description_size.set_max_desired_height(80)
attach(info,description_size)
attach(header,info,fill=True)
attach(body,header,margin(0,0,0,16))
output=panel(bp,u.VerticalBox,'OutputSummary')
attach(output,label(bp,'OutputHeading','RESULT',11,True,color=MUTED),margin(0,0,0,5))
attach(output,label(bp,'RecipeOutput','Recipe output',16,True,wrapped=True),margin(0,0,0,8))
attach(output,label(bp,'RecipeWorkAmount','Work amount: 0',13,color=MUTED))
output_scroll=panel(bp,u.ScrollBox,'OutputScroll'); attach(output_scroll,output)
attach(body,frame(bp,'OutputFrame',surface(bp,'OutputSurface',size(bp,'OutputMaxSize',output_scroll),BG,12)),margin(0,0,0,16))
widget(bp,'OutputMaxSize').set_max_desired_height(130)
head=panel(bp,u.HorizontalBox,'RequirementsHeading')
attach(head,label(bp,'RequirementsTitle','Required resources',16,True),fill=True)
attach(head,size(bp,'RequirementsCountHeading',label(bp,'AmountsHeading','Need / Have',12,color=MUTED),108))
attach(body,head,margin(0,0,0,10))
switcher=widget(bp,'RecipeTabsSwitcher'); switcher.clear_children()
attach(switcher,widget(bp,'RecipeDetailRequiredListSimple'))
area=panel(bp,u.Overlay,'RequirementsArea'); attach(area,switcher)
slot=attach(area,label(bp,'NoRequirementsText','No resources required',15,color=MUTED,wrapped=True),12)
slot.set_vertical_alignment(V.V_ALIGN_CENTER)
attach(body,area,fill=True)
footer=panel(bp,u.HorizontalBox,'DetailFooter')
quantity=panel(bp,u.VerticalBox,'QuantitySection')
attach(quantity,label(bp,'QuantityHeading','Quantity',13,True),margin(0,0,0,6))
attach(quantity,make(bp,QUANTITY,'CraftingQuantitySelector'))
attach(footer,quantity,margin(0,0,16,0))
slot=attach(footer,widget(bp,'CraftMenuActionButtons'),fill=True); slot.set_vertical_alignment(V.V_ALIGN_BOTTOM)
attach(body,footer,margin(0,16,0,0))
EDIT.set_root(bp,body); save(bp)

# Main picker uses the same outer size as the dashboard.
bp=asset('UI/Craft/WBP_CraftMenuChoose')
body=panel(bp,u.VerticalBox,'ChooseBody')
title=make(bp,TITLE,'MovableTitleBar')
title.set_editor_property('allow_dragging',True)
# Preserve the restored title-bar hit-test behavior.
title.set_visibility(VIS.SELF_HIT_TEST_INVISIBLE)
attach(body,title,margin(0,0,0,18))
columns=panel(bp,u.HorizontalBox,'ChooseColumns')
left=panel(bp,u.VerticalBox,'RecipeColumn')
attach(left,label(bp,'RecipesHeading','Recipes',16,True),margin(0,0,0,10))
attach(left,widget(bp,'CraftRecipesList'),fill=True)
attach(columns,size(bp,'RecipeColumnSize',frame(bp,'RecipeColumnFrame',surface(bp,'RecipeColumnSurface',left,PANEL,14)),316),margin(0,0,16,0))
switcher=widget(bp,'WidgetSwitcher'); switcher.clear_children()
attach(switcher,widget(bp,'CraftMenuDetail'))
empty=style_label(widget(bp,'EmptySelectionText'),'Select a recipe to view its requirements',17,color=MUTED)
empty.set_visibility(VIS.SELF_HIT_TEST_INVISIBLE)
slot=attach(switcher,empty,24); slot.set_vertical_alignment(V.V_ALIGN_CENTER); slot.set_horizontal_alignment(H.H_ALIGN_CENTER)
attach(columns,frame(bp,'DetailFrame',surface(bp,'DetailSurface',switcher,PANEL,18)),fill=True)
attach(body,columns,fill=True)
root=size(bp,'ChooseRoot',frame(bp,'ChooseFrame',surface(bp,'ChooseSurface',body,BG,20)))
root.set_min_desired_width(1040); root.set_min_desired_height(600)
EDIT.set_root(bp,root); save(bp)
bp=asset('UI/Layouts/Layout/GameMenu/WBP_GameMenuInvenza')
embedded=widget(bp,'WBP_CraftMenuChoose')
dashboard=widget(bp,'WBP_CraftDashboard')
embedded.slot.set_anchors(dashboard.slot.get_anchors())
embedded.slot.set_position(dashboard.slot.get_position())
embedded.slot.set_size(u.Vector2D(1160,680))
embedded.slot.set_alignment(dashboard.slot.get_alignment())
save(bp)
u.log('CRAFT_CHOOSE_RESTYLE_COMPLETE')
