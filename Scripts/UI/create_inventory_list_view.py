# Nublin Studio 2026 All Rights Reserved.
"""Author only the two read-only list view templates. Run with Unreal Python.

Requires the new native view classes and the project-only InvenzaUIEditor module.
Existing inventory widgets, Core widgets, settings and maps are never saved.
"""
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
EDIT = u.InvenzaWidgetEditingLibrary
ASSETS = u.AssetToolsHelpers.get_asset_tools()
FOLDER = ROOT + "UI/Inventory/Views"
u.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([FOLDER], force_rescan=True)
LABEL = u.load_class(None, ROOT + "UI/Core/WBP_LabelBaseText.WBP_LabelBaseText_C")
BUTTON = u.load_class(None, ROOT + "UI/Core/Buttons/WBP_UIButtonWithSlot.WBP_UIButtonWithSlot_C")
TOOLTIP = u.load_class(None, ROOT + "UI/HelpersWidgets/WBP_ItemTooltip.WBP_ItemTooltip_C")
assert LABEL and BUTTON and TOOLTIP
container_template = u.load_asset(ROOT + "UI/InventoryContainer/WBP_InvContainer")
money_label = u.find_object(u.find_object(container_template, "WidgetTree"), "InvMoney")
BASE_FONT = money_label.get_editor_property("font_info").copy()

GOLD = u.LinearColor(.82, .65, .36, 1)
PANEL = u.LinearColor(.035, .024, .013, 1)
ROW = u.LinearColor(.065, .045, .025, 1)


def new_blueprint(name, parent):
    path = FOLDER + "/" + name
    assert name in ("WBP_InventoryListView", "WBP_InventoryListViewRow")
    if u.EditorAssetLibrary.does_asset_exist(path):
        result = u.load_asset(path)
        assert isinstance(u.get_default_object(result.generated_class()), parent)
        return result
    factory = u.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    result = ASSETS.create_asset(name, FOLDER, None, factory)
    assert result
    return result


def make(bp, cls, name):
    result = EDIT.add_template(bp, cls, name)
    assert result
    if isinstance(result, u.PanelWidget):
        result.clear_children()
    return result


def attach(parent, child, padding=0, fill=False):
    slot = parent.add_child(child)
    if hasattr(slot, "set_padding"):
        slot.set_padding(u.Margin(padding, padding, padding, padding))
    if hasattr(slot, "set_vertical_alignment"):
        slot.set_vertical_alignment(u.VerticalAlignment.V_ALIGN_CENTER)
    if isinstance(slot, (u.HorizontalBoxSlot, u.VerticalBoxSlot)):
        slot.set_size(u.SlateChildSize(1, u.SlateSizeRule.FILL if fill else u.SlateSizeRule.AUTOMATIC))
    return slot


def sized(bp, name, child, width=None, height=None):
    result = make(bp, u.SizeBox, name)
    if width is not None:
        result.set_width_override(width)
    if height is not None:
        result.set_height_override(height)
    slot = attach(result, child)
    slot.set_vertical_alignment(u.VerticalAlignment.V_ALIGN_FILL)
    return result


def label(bp, name, value, size=16):
    result = make(bp, LABEL, name)
    font = BASE_FONT.copy()
    font.size = size
    font.typeface_font_name = "Regular"
    result.set_editor_property("font_info", font)
    result.set_editor_property("text", u.Text(value))
    result.set_editor_property("color", u.SlateColor(specified_color=GOLD))
    return result


def frame(bp, name, child, color):
    result = make(bp, u.Border, name)
    result.set_brush_color(color)
    result.set_padding(u.Margin(8, 6, 8, 6))
    slot = attach(result, child)
    slot.set_vertical_alignment(u.VerticalAlignment.V_ALIGN_FILL)
    return result


def save(bp):
    assert EDIT.compile_widget(bp), bp.get_name()
    assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)


bp = new_blueprint("WBP_InventoryListViewRow", u.InventoryListViewRowWidget)
row = make(bp, u.HorizontalBox, "RowContent")
icon = make(bp, u.Image, "ItemIcon")
attach(row, sized(bp, "IconSize", icon, 36, 36), 4)
attach(row, label(bp, "ItemName", ""), 6, True)
attach(row, sized(bp, "QuantitySize", label(bp, "ItemQuantity", ""), 72), 6)
EDIT.set_root(bp, frame(bp, "RowFrame", row, ROW))
assert EDIT.compile_widget(bp)
u.get_default_object(bp.generated_class()).set_editor_property("item_tooltip_widget_class", TOOLTIP)
save(bp)
ROW_CLASS = bp.generated_class()

bp = new_blueprint("WBP_InventoryListView", u.InventoryListViewWidget)
body = make(bp, u.VerticalBox, "ViewContent")
toolbar = make(bp, u.HorizontalBox, "ViewToolbar")
search_icon = make(bp, u.Image, "SearchIcon")
search_icon.set_brush_from_texture(u.load_asset(ROOT + "Textures/UI/Search/T_Icon_find"))
attach(toolbar, sized(bp, "SearchIconSize", search_icon, 18, 18), 6)
search = make(bp, u.EditableText, "SearchText")
search.set_hint_text(u.Text("Find item"))
search_font = BASE_FONT.copy()
search_font.size = 15
search_font.typeface_font_name = "Regular"
search.set_font(search_font)
attach(toolbar, search, 6, True)
for name, texture in [("SortByNameButton", "Operations/T_Sort_AZ"), ("SortByQuantityButton", "Arrows/T_Down")]:
    button = make(bp, BUTTON, name)
    button.set_editor_property("default_text", u.Text(""))
    image = make(bp, u.Image, name + "Icon")
    image.set_brush_from_texture(u.load_asset(ROOT + "Textures/UI/" + texture))
    EDIT.set_slot_content(button, "NamedSlot_21", sized(bp, name + "IconSize", image, 22, 22))
    attach(toolbar, sized(bp, name + "Size", button, 36, 36), 3)
attach(body, toolbar, 2)
items = make(bp, u.ListView, "ItemsList")
items.set_editor_property("entry_widget_class", ROW_CLASS)
items.set_editor_property("selection_mode", u.SelectionMode.SINGLE)
slot = attach(body, items, 2, True)
slot.set_vertical_alignment(u.VerticalAlignment.V_ALIGN_FILL)
EDIT.set_root(bp, sized(bp, "DefaultViewSize", frame(bp, "ViewFrame", body, PANEL), 480, 480))
assert EDIT.compile_widget(bp)
cdo = u.get_default_object(bp.generated_class())
for prop, text in {
    "sort_by_name_tooltip": "Sort by name, ascending",
    "name_ascending_active_tooltip": "Sorted by name, ascending. Click for descending.",
    "name_descending_active_tooltip": "Sorted by name, descending. Click for ascending.",
    "sort_by_quantity_tooltip": "Sort by quantity, largest first",
    "quantity_sort_active_tooltip": "Sorted by quantity, largest first.",
}.items():
    cdo.set_editor_property(prop, u.Text(text))
save(bp)
u.log("INVENZA_INVENTORY_LIST_VIEW_ASSETS_OK")
