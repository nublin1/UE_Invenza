"""Adjust the crafting menu's local styling without replacing widget trees."""
from pathlib import Path
import shutil
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir()).resolve()
EDIT = u.InvenzaWidgetEditingLibrary

def asset(path):
    disk = PROJECT / 'Plugins/InventorySystemInvenzaPlugin/Content' / (path + '.uasset')
    backup = PROJECT / 'Saved/CraftUI/BeforePolish' / (path + '.uasset')
    if not backup.exists():
        backup.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(disk, backup)
    bp = u.load_asset(ROOT + path)
    assert bp, path
    bp.modify()
    return bp

def widget(bp, name):
    w = u.find_object(u.find_object(bp, 'WidgetTree'), name)
    assert w, name
    w.modify()
    return w

def image_color(w, color=None):
    style = w.get_editor_property('brush_style')
    old = style.brush.tint_color.specified_color
    mult = style.brush_color
    style.brush_color = color or u.LinearColor(old.r*mult.r, old.g*mult.g, old.b*mult.b, old.a*mult.a)
    brush = style.brush
    brush.tint_color = u.SlateColor(specified_color=u.LinearColor.WHITE)
    style.brush = brush
    w.set_editor_property('brush_style', style)

def frame(w, color=None):
    for side in ['top', 'bottom', 'left', 'right']:
        key = side + '_border_style'
        style = w.get_editor_property(key)
        old, mult = style.brush.tint_color.specified_color, style.brush_color
        style.brush_color = color or u.LinearColor(old.r*mult.r, old.g*mult.g, old.b*mult.b, old.a*mult.a)
        brush = style.brush
        brush.tint_color = u.SlateColor(specified_color=u.LinearColor.WHITE)
        style.brush = brush
        # Empty Border desired size is the sum of its opposing padding values.
        # Two Slate units remain visible at the game's ~0.67 DPI scale.
        style.padding = u.Margin(1, 1, 1, 1)
        style.desired_size = u.Vector2D(1, 1)
        w.set_editor_property(key, style)

def save(bp):
    assert EDIT.compile_widget(bp), bp.get_name()
    assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)

controls = asset('UI/Craft/WBP_DashboardControlPanel')
for name in ['Btn_AddTaskFrame', 'Btn_PauseFrame']:
    w = widget(controls, name)
    frame(w)
    pad = w.slot.get_editor_property('padding')
    pad.top = pad.bottom = 2
    w.slot.set_padding(pad)
save(controls)

dashboard = asset('UI/Craft/WBP_CraftDashboard')
for name in ['CraftFrame', 'QueueFrame']:
    frame(widget(dashboard, name))
for name in ['CraftSurfaceBackground', 'QueueBackground']:
    image_color(widget(dashboard, name))
widget(dashboard, 'QueueCraftList').slot.set_padding(u.Margin(8, 8, 8, 8))
save(dashboard)

entry = asset('UI/Craft/Lists/WBP_QueueCraftList_Dashboard')
image_color(widget(entry, 'EntrySurfaceBackground'), u.LinearColor(0.065, 0.045, 0.025, 1))
frame(widget(entry, 'EntryFrame'), u.LinearColor(0.20, 0.14, 0.075, 1))
widget(entry, 'QueueItemName').set_editor_property('color', u.SlateColor(specified_color=u.LinearColor(0.82, 0.65, 0.36, 1)))
assert EDIT.compile_widget(entry)
# ListView adds the entry CDO padding to its own spacing. Scope this to the
# dashboard entry so other queue presentations retain their existing spacing.
u.get_default_object(entry.generated_class()).set_padding(u.Margin(0, 0, 0, 8))
assert u.EditorAssetLibrary.save_loaded_asset(entry, only_if_is_dirty=False)

for path in ['UI/Core/Progress/WBP_RemainingCounter_CraftDashboard', 'UI/Core/Progress/WBP_RemainingCounter_CraftDashboard_WorkAmount']:
    bp = asset(path)
    tree = u.find_object(bp, 'WidgetTree')
    for w in u.ObjectIterator(u.LabelBaseText):
        if w.get_outer() == tree:
            w.modify()
            w.set_editor_property('color', u.SlateColor(specified_color=u.LinearColor(0.48, 0.38, 0.25, 1)))
    save(bp)

status = asset('UI/Core/Status/WBP_StatusMessage')
frame(widget(status, 'StatusFrame'))
for name in ['StatusSurfaceBackground', 'ListEntry_Image']:
    image_color(widget(status, name))
save(status)
u.log('CRAFT_MENU_POLISH_COMPLETE')
