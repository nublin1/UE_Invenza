#  Nublin Studio 2026 All Rights Reserved.

"""Update only the dashboard queue row and world craft widget; create local Core/world components.

Run in Unreal Python after building. Existing assets are backed up under
Saved/CraftProgress/Before. Does not save the crafting station or shared widgets.
"""

from pathlib import Path
import shutil
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir()).resolve()
EDIT = u.InvenzaWidgetEditingLibrary
u.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([ROOT], force_rescan=True)
H, V, VIS = u.HorizontalAlignment, u.VerticalAlignment, u.SlateVisibility
GOLD = u.LinearColor(.82, .65, .36, 1)


def asset(path):
    disk = PROJECT / 'Plugins/InventorySystemInvenzaPlugin/Content' / (path + '.uasset')
    before = PROJECT / 'Saved/CraftProgress/Before' / (path + '.uasset')
    if disk.exists() and not before.exists():
        before.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(disk, before)
    bp = u.load_asset(ROOT + path)
    assert bp, path
    bp.modify()
    return bp


def widget(bp, name):
    w = u.find_object(u.find_object(bp, 'WidgetTree'), name)
    assert w, bp.get_name() + ': ' + name
    w.modify()
    return w


def make(bp, cls, name):
    w = EDIT.add_template(bp, cls, name)
    assert w, name
    w.modify()
    w.set_visibility(VIS.SELF_HIT_TEST_INVISIBLE)
    return w


def panel(bp, cls, name):
    w = make(bp, cls, name)
    w.clear_children()
    return w


def attach(parent, child, padding=None, fill=False):
    child.remove_from_parent()
    slot = parent.add_child(child)
    if hasattr(slot, 'set_padding'):
        slot.set_padding(padding or u.Margin())
    if hasattr(slot, 'set_horizontal_alignment'):
        slot.set_horizontal_alignment(H.H_ALIGN_FILL)
    if hasattr(slot, 'set_vertical_alignment'):
        slot.set_vertical_alignment(V.V_ALIGN_CENTER)
    if isinstance(slot, (u.HorizontalBoxSlot, u.VerticalBoxSlot)):
        slot.set_size(u.SlateChildSize(1., u.SlateSizeRule.FILL if fill else u.SlateSizeRule.AUTOMATIC))
    return slot


def save(bp):
    assert EDIT.compile_widget(bp), bp.get_name()
    assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)


LABEL = u.load_class(None, ROOT + 'UI/Core/WBP_LabelBaseText.WBP_LabelBaseText_C')
inventory = u.load_asset(ROOT + 'UI/InventoryContainer/WBP_InvContainer')
FONT = u.find_object(u.find_object(inventory, 'WidgetTree'), 'InvMoney').get_editor_property('font_info').copy()


def label(bp, name, text, size=13, bold=False):
    w = make(bp, LABEL, name)
    font = FONT.copy()
    font.size = size
    font.typeface_font_name = 'Bold' if bold else 'Regular'
    w.set_editor_properties(dict(text=u.Text(text), color=u.SlateColor(specified_color=GOLD), font_info=font))
    return w


# New reusable Core widget. Existing counters remain unchanged.
core_path = ROOT + 'UI/Core/Progress/WBP_CraftProgressDisplay'
if not u.EditorAssetLibrary.does_asset_exist(core_path):
    factory = u.WidgetBlueprintFactory()
    factory.set_editor_property('parent_class', u.CraftProgressDisplay.static_class())
    bp = u.AssetToolsHelpers.get_asset_tools().create_asset(
        'WBP_CraftProgressDisplay', ROOT + 'UI/Core/Progress', u.WidgetBlueprint, factory)
else:
    bp = asset('UI/Core/Progress/WBP_CraftProgressDisplay')
root = panel(bp, u.HorizontalBox, 'ProgressRow')
work = panel(bp, u.HorizontalBox, 'WorkAmount')
for name, text in [('CurrentValue', '0'), ('SeparatorSymbol', '/'), ('MaxValue', '5')]:
    attach(work, label(bp, name, text), u.Margin(0, 0, 6 if name != 'MaxValue' else 0, 0))
attach(root, work)
bar_size = panel(bp, u.SizeBox, 'BarContainer')
bar_size.set_height_override(6)
bar_size.set_min_desired_width(24)
bar = make(bp, u.ProgressBar, 'ProgressBar')
brush = u.SlateBrush(draw_as=u.SlateBrushDrawType.IMAGE, tint_color=u.SlateColor(specified_color=u.LinearColor.WHITE))
bar.set_editor_property('widget_style', u.ProgressBarStyle(background_image=brush, fill_image=brush, marquee_image=brush))
bar.set_editor_property('bar_fill_style', u.ProgressBarFillStyle.SCALE)
bar.set_editor_property('border_padding', u.Vector2D(0, 0))
attach(bar_size, bar)
attach(root, bar_size, fill=True)
attach(root, label(bp, 'PercentageLabel', '0%'), u.Margin(8, 0, 0, 0))
EDIT.set_root(bp, root)
save(bp)
PROGRESS = bp.generated_class()


def replace_progress(entry):
    old = widget(entry, 'CraftProgress')
    parent = old.get_parent()
    index = parent.get_child_index(old)
    children = list(parent.get_all_children())
    pads = [child.slot.get_editor_property('padding') for child in children]
    new = make(entry, PROGRESS, 'CraftProgress')
    children[index] = new
    parent.clear_children()
    for child, pad in zip(children, pads):
        attach(parent, child, pad)
    new.slot.set_padding(u.Margin(0, 4, 0, 0))
    assert EDIT.compile_widget(entry)
    u.get_default_object(entry.generated_class()).set_editor_property(
        'progress_settings', u.CraftProgressDisplaySettings(use_progress_bar=True, show_percentage=True))
    assert u.EditorAssetLibrary.save_loaded_asset(entry, only_if_is_dirty=False)


dashboard = asset('UI/Craft/Lists/WBP_QueueCraftList_Dashboard')
replace_progress(dashboard)

# A local world entry avoids changing ItemBasic everywhere it is reused.
world_entry_path = ROOT + 'UI/World/Craft/WBP_WorldCraftEntry'
if not u.EditorAssetLibrary.does_asset_exist(world_entry_path):
    assert u.EditorAssetLibrary.duplicate_asset(
        ROOT + 'UI/Craft/Lists/WBP_QueueCraftList_ItemBasic', world_entry_path)
world_entry = asset('UI/World/Craft/WBP_WorldCraftEntry')
replace_progress(world_entry)
entry_size = widget(world_entry, 'SizeBox_0')
entry_size.clear_width_override()
entry_size.set_min_desired_height(72)
widget(world_entry, 'VBox_QueueItemInfo').slot.set_size(u.SlateChildSize(1., u.SlateSizeRule.FILL))
name = widget(world_entry, 'QueueItemName')
font = FONT.copy()
font.size = 20
font.typeface_font_name = 'Bold'
name.set_editor_properties(dict(font_info=font, color=u.SlateColor(specified_color=GOLD)))
progress = widget(world_entry, 'CraftProgress')
style = progress.get_editor_property('text_style')
font = FONT.copy()
font.size = 16
font.typeface_font_name = 'Regular'
style.font_info = font
progress.set_editor_properties(dict(text_style=style, bar_height=8.))
save(world_entry)

world = asset('UI/World/Craft/WBP_WorldCraftState')
size = widget(world, 'SizeBox_28')
size.clear_children()
size.set_width_override(250)
size.set_min_desired_height(100)
frame = panel(world, u.Border, 'WorldFrame')
frame.set_brush(brush)
frame.set_padding(u.Margin(1))
surface = panel(world, u.Border, 'WorldSurface')
surface.set_brush(brush)
surface.set_padding(u.Margin(16, 12, 16, 12))
entry = make(world, world_entry.generated_class(), 'QueueCraftListEntryWidget')
attach(surface, entry)
attach(frame, surface)
attach(size, frame)
layout = panel(world, u.Overlay, 'WorldCraftLayout')
slot = attach(layout, size)
slot.set_horizontal_alignment(H.H_ALIGN_CENTER)
slot.set_vertical_alignment(V.V_ALIGN_CENTER)
EDIT.set_root(world, layout)
save(world)
u.log('CRAFT_PROGRESS_ASSETS_UPDATED')
