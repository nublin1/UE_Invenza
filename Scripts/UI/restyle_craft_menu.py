"""Run with Unreal Editor Python after building the InvenzaUIEditor module.

Reuses Core widgets and existing control buttons. Backs up each input asset once
under Saved/CraftUI/Before before saving any changes.
"""
from pathlib import Path
import shutil
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir()).resolve()
EDIT = u.InvenzaWidgetEditingLibrary
u.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([ROOT], force_rescan=True)
H = u.HorizontalAlignment
V = u.VerticalAlignment
VIS = u.SlateVisibility

def asset(path):
    disk = PROJECT / 'Plugins/InventorySystemInvenzaPlugin/Content' / (path + '.uasset')
    backup = PROJECT / 'Saved/CraftUI/Before' / (path + '.uasset')
    if disk.exists() and not backup.exists():
        backup.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(disk, backup)
    result = u.load_asset(ROOT + path)
    assert result, path
    result.modify()
    return result

def widget(bp, name):
    return u.find_object(u.find_object(bp, 'WidgetTree'), name)

def make(bp, cls, name):
    result = EDIT.add_template(bp, cls, name)
    assert result, name
    result.modify()
    result.set_visibility(VIS.SELF_HIT_TEST_INVISIBLE)
    return result

def margin(l=0, t=None, r=None, b=None):
    return u.Margin(l, l if t is None else t, l if r is None else r, l if b is None else b)

def attach(panel, child, pad=None, fill=False):
    child.remove_from_parent()
    slot = panel.add_child(child)
    # Overlay slots default to Left/Top, so an image otherwise stays icon-sized.
    if isinstance(slot, u.OverlaySlot):
        slot.set_horizontal_alignment(H.H_ALIGN_FILL)
        slot.set_vertical_alignment(V.V_ALIGN_FILL)
    if pad is not None: slot.set_padding(pad)
    if isinstance(slot, (u.VerticalBoxSlot, u.HorizontalBoxSlot)):
        slot.set_size(u.SlateChildSize(1.0, u.SlateSizeRule.FILL if fill else u.SlateSizeRule.AUTOMATIC))
    return slot

def save(bp):
    assert EDIT.compile_widget(bp), 'Compile failed: ' + bp.get_name()
    assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)

inventory = u.load_asset(ROOT + 'UI/InventoryContainer/WBP_InvContainer')
BG = widget(inventory, 'Border_27').get_editor_property('brush_color')
GOLD = widget(inventory, 'InvMoney').get_editor_property('color')
GOLD_COLOR = GOLD.specified_color
BASE_FONT = widget(inventory, 'InvMoney').get_editor_property('font_info')
MUTED = u.SlateColor(specified_color=u.LinearColor(0.38, 0.29, 0.18, 1))
PANEL_BG = u.LinearColor(0.035, 0.024, 0.013, 1)
LABEL = u.load_class(None, ROOT + 'UI/Core/WBP_LabelBaseText.WBP_LabelBaseText_C')
IMAGE = u.load_class(None, ROOT + 'UI/Core/WBP_BaseImage.WBP_BaseImage_C')
FRAME = u.load_class(None, ROOT + 'UI/Core/WBP_Border4SideWithSlot.WBP_Border4SideWithSlot_C')

def label(bp, name, text, size=16, bold=False, color=None):
    w = make(bp, LABEL, name)
    f = BASE_FONT.copy()
    f.size = size
    f.typeface_font_name = 'Bold' if bold else 'Regular'
    w.set_editor_properties(dict(text=u.Text(text), color=color or GOLD, font_info=f))
    return w

def background(bp, name, color):
    w = make(bp, IMAGE, name)
    w.set_editor_property('base_material', None)
    brush = u.SlateBrush(draw_as=u.SlateBrushDrawType.BOX, tint_color=u.SlateColor(specified_color=color))
    w.set_editor_property('brush_style', u.UIBrushStyle(brush=brush, brush_color=u.LinearColor.WHITE))
    w.set_visibility(VIS.HIT_TEST_INVISIBLE)
    return w

def frame(bp, name, content, color=None):
    w = make(bp, FRAME, name)
    tint = color or GOLD_COLOR
    style = u.BorderFrameStyle(
        brush=u.SlateBrush(draw_as=u.SlateBrushDrawType.BOX, tint_color=u.SlateColor(specified_color=tint)),
        brush_color=u.LinearColor.WHITE, padding=margin(1), desired_size=u.Vector2D(1, 1))
    for side in ['top', 'bottom', 'left', 'right']:
        w.set_editor_property(side + '_border_style', style)
    content.remove_from_parent()
    EDIT.set_slot_content(w, 'NamedSlot', content)
    return w

def surface(bp, name, content, color=BG, pad=12):
    over = make(bp, u.Overlay, name)
    over.clear_children()
    attach(over, background(bp, name + 'Background', color))
    attach(over, content, margin(pad))
    return over

# General-purpose Core message row, using the existing list-entry C++ class.
status_path = 'UI/Core/Status/WBP_StatusMessage'
if not (PROJECT / 'Plugins/InventorySystemInvenzaPlugin/Content' / (status_path + '.uasset')).exists():
    factory = u.WidgetBlueprintFactory()
    factory.set_editor_property('parent_class', u.SimpleUserObjectListEntry.static_class())
    status = u.AssetToolsHelpers.get_asset_tools().create_asset(
        'WBP_StatusMessage', ROOT + 'UI/Core/Status', u.WidgetBlueprint, factory)
else:
    status = asset(status_path)
status_row = make(status, u.HorizontalBox, 'StatusRow')
status_row.clear_children()
indicator_box = make(status, u.SizeBox, 'IndicatorSize')
indicator_box.set_width_override(4)
indicator_box.set_height_override(18)
indicator_box.clear_children()
attach(indicator_box, background(status, 'ListEntry_Image', GOLD_COLOR))
icon_slot = attach(status_row, indicator_box, margin(0, 2, 10, 0))
icon_slot.set_vertical_alignment(V.V_ALIGN_TOP)
text = label(status, 'ListEntry_Text', 'Production is blocked', 14)
attach(status_row, text, fill=True)
EDIT.set_root(status, frame(status, 'StatusFrame', surface(status, 'StatusSurface', status_row, PANEL_BG, 10), u.LinearColor(0.18, 0.12, 0.055, 1)))
save(status)
STATUS = status.generated_class()

# Keep button instances/classes; only change their public text and layout.
controls = asset('UI/Craft/WBP_DashboardControlPanel')
holder = widget(controls, 'BtnsHolder')
add_button = widget(controls, 'Btn_AddTask')
pause_button = widget(controls, 'Btn_Pause')
holder.clear_children()
for btn, caption in [(add_button, 'Add task'), (pause_button, 'Pause')]:
    btn.modify()
    btn.set_editor_property('default_text', u.Text(caption))
    btn.set_editor_property('is_toggle_button', False)
    btn.set_tool_tip_text(u.Text('Choose a recipe to add to the queue' if btn==add_button else 'Pause or resume production'))
    box = make(controls, u.SizeBox, btn.get_name() + 'Layout')
    box.set_min_desired_height(42)
    box.clear_children()
    attach(box, btn, margin(10, 4, 10, 4))
    attach(holder, frame(controls, btn.get_name() + 'Frame', box, u.LinearColor(0.24, 0.17, 0.08, 1)),
           margin(0, 0, 8 if btn==add_button else 0, 0), fill=True)
save(controls)

# Dashboard: preserve live inventory slots and queue widgets from the existing asset.
dashboard = asset('UI/Craft/WBP_CraftDashboard')
queue = widget(dashboard, 'QueueCraftList')
control = widget(dashboard, 'CraftControlPanel')
input_slot = widget(dashboard, 'InputSlot')
output_slot = widget(dashboard, 'OutputSlot')
interactor_slot = widget(dashboard, 'InteractorSlot')
fuel_slot = widget(dashboard, 'FuelSlot') or make(dashboard, u.NamedSlot, 'FuelSlot')

root = make(dashboard, u.SizeBox, 'CraftRoot')
root.clear_children()
root.set_min_desired_width(1040)
root.set_min_desired_height(600)
body = make(dashboard, u.VerticalBox, 'CraftBody')
body.clear_children()
attach(body, label(dashboard, 'CraftTitle', 'Crafting', 20, True), margin(0, 0, 0, 14))
columns = make(dashboard, u.HorizontalBox, 'CraftColumns')
columns.clear_children()
attach(body, columns, fill=True)

left = make(dashboard, u.VerticalBox, 'VBox_Craft')
left.clear_children()
left_width = make(dashboard, u.SizeBox, 'QueueColumnSize')
left_width.set_width_override(380)
left_width.clear_children()
attach(left_width, left)
attach(columns, left_width, margin(0, 0, 18, 0))
attach(left, label(dashboard, 'QueueTitle', 'Production queue', 17, True), margin(0, 0, 0, 10))
attach(left, control, margin(0, 0, 0, 12))
queue_over = make(dashboard, u.Overlay, 'QueueArea')
queue_over.clear_children()
attach(queue_over, background(dashboard, 'QueueBackground', PANEL_BG))
attach(queue_over, queue, margin(6))
empty = label(dashboard, 'EmptyQueueLabel', 'Queue is empty', 15, color=MUTED)
empty.set_visibility(VIS.COLLAPSED)
empty_slot = attach(queue_over, empty, margin(12))
empty_slot.set_horizontal_alignment(H.H_ALIGN_CENTER)
empty_slot.set_vertical_alignment(V.V_ALIGN_CENTER)
attach(left, frame(dashboard, 'QueueFrame', queue_over, u.LinearColor(0.16, 0.11, 0.05, 1)), fill=True)

section = make(dashboard, u.VerticalBox, 'BlockReasonsSection')
section.clear_children()
attach(section, label(dashboard, 'StatusTitle', 'Production status', 15, True), margin(0, 12, 0, 8))
scroll_size = make(dashboard, u.SizeBox, 'StatusMaxHeight')
scroll_size.set_max_desired_height(164)
scroll_size.clear_children()
scroll = make(dashboard, u.ScrollBox, 'StatusScroll')
scroll.clear_children()
panel = make(dashboard, u.VerticalBox, 'BlockReasonsPanel')
panel.clear_children()
# Designer-only sample content; NativeConstruct replaces it from the actual state.
for i, message in enumerate(['Not enough resources', 'Paused by user']):
    row = make(dashboard, STATUS, 'StatusPreview' + str(i))
    attach(panel, row, margin(0, 0, 0, 6))
attach(scroll, panel)
attach(scroll_size, scroll)
attach(section, scroll_size)
attach(left, section)

station = make(dashboard, u.VerticalBox, 'VBox_StationInv')
station.clear_children()
attach(columns, station, margin(0, 0, 18, 0), fill=True)
for name, caption, slot in [('InputTitle','Resources',input_slot), ('FuelTitle','Fuel',fuel_slot), ('OutputTitle','Output',output_slot)]:
    attach(station, label(dashboard, name, caption, 16, True), margin(0, 0, 0, 8))
    attach(station, slot, margin(0, 0, 0, 16))

player = make(dashboard, u.VerticalBox, 'VBox_Interactor')
player.clear_children()
attach(columns, player, fill=True)
attach(player, label(dashboard, 'InventoryTitle', 'Inventory', 16, True), margin(0, 0, 0, 8))
attach(player, interactor_slot)
attach(root, frame(dashboard, 'CraftFrame', surface(dashboard, 'CraftSurface', body, BG, 16)))
EDIT.set_root(dashboard, root)
assert EDIT.compile_widget(dashboard)
u.get_default_object(dashboard.generated_class()).set_editor_property('block_reason_widget_class', STATUS)
save(dashboard)

# Queue row and its existing Core progress labels: compact typography and spacing.
entry = asset('UI/Craft/Lists/WBP_QueueCraftList_Dashboard')
entry_root = widget(entry, 'SizeBox_0')
entry_root.clear_width_override()
entry_root.set_min_desired_height(84)
entry_root.set_padding(margin(0)) if hasattr(entry_root,'set_padding') else None
entry_content = widget(entry, 'HorizontalBox_34')
entry_root.clear_children()
attach(entry_root, frame(entry, 'EntryFrame', surface(entry, 'EntrySurface', entry_content, PANEL_BG, 8), u.LinearColor(0.12, 0.085, 0.04, 1)))
item_name = widget(entry, 'QueueItemName')
font = BASE_FONT.copy()
font.size = 15
font.typeface_font_name = 'Bold'
item_name.set_editor_properties(dict(font_info=font, color=GOLD))
widget(entry,'VBox_QueueItemInfo').slot.set_size(u.SlateChildSize(1, u.SlateSizeRule.FILL))
widget(entry,'Spacer_259').set_size(u.Vector2D(10,0))
save(entry)

for path in ['UI/Core/Progress/WBP_RemainingCounter_CraftDashboard', 'UI/Core/Progress/WBP_RemainingCounter_CraftDashboard_WorkAmount']:
    bp = asset(path)
    tree = u.find_object(bp,'WidgetTree')
    for w in u.ObjectIterator(u.LabelBaseText):
        if w.get_outer()!=tree: continue
        font = w.get_editor_property('font_info')
        font.size = 13
        w.set_editor_properties(dict(font_info=font,color=MUTED))
    save(bp)

# Adjust the embedded dashboard in the requested game-menu asset.
menu = asset('UI/Layouts/Layout/GameMenu/WBP_GameMenuInvenza')
embedded = widget(menu, 'WBP_CraftDashboard')
embedded.slot.set_size(u.Vector2D(1160, 680))
embedded.slot.set_alignment(u.Vector2D(0.5,0.5))
save(menu)
# Apply the current local palette and entry spacing after initial construction.
polish = PROJECT / 'Scripts/UI/polish_craft_menu.py'
exec(compile(polish.read_text(encoding='utf-8-sig'), str(polish), 'exec'), {})
u.log('CRAFT_MENU_RESTYLE_COMPLETE')
