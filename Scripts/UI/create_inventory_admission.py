# Nublin Studio 2026 All Rights Reserved.
"""Author the compact admission tree. Existing operations panels are opt-in only."""
from pathlib import Path
import json
import shutil
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir())
CONTENT = PROJECT / 'Plugins/InventorySystemInvenzaPlugin/Content'
EDIT = u.InvenzaWidgetEditingLibrary
ASSETS = u.AssetToolsHelpers.get_asset_tools()
BACKUP = PROJECT / 'Saved/InventoryAdmissionSimple/AssetsBefore'
UPDATE_OPERATIONS_PANEL = False
WITH_SLOT_BUTTON = u.load_class(None, ROOT + 'UI/Core/Buttons/WBP_UIButtonWithSlot.WBP_UIButtonWithSlot_C')
FOLDER = ROOT + 'UI/Inventory/Admission'
u.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([FOLDER, ROOT + 'Textures/UI/Admission'], force_rescan=True)
LABEL = u.load_class(None, ROOT + 'UI/Core/WBP_LabelBaseText.WBP_LabelBaseText_C')
BUTTON = u.load_class(None, ROOT + 'UI/Core/Buttons/WBP_UIButton.WBP_UIButton_C')
assert LABEL and BUTTON
TABLE = u.load_asset(ROOT + 'Data/Localization/ST_InvenzaRuntimeTexts')
container = u.load_asset(ROOT + 'UI/InventoryContainer/WBP_InvContainer')
FONT = u.find_object(u.find_object(container, 'WidgetTree'), 'InvMoney').get_editor_property('font_info').copy()
GOLD = u.LinearColor(.82, .65, .36, 1)
BACKGROUND = u.LinearColor(.035, .024, .013, 1)


def preserve(relative):
    source = CONTENT / (relative + '.uasset')
    target = BACKUP / (relative + '.uasset')
    if source.exists() and not target.exists():
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)


def save(bp):
    assert EDIT.compile_widget(bp), bp.get_name()
    assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)


def blueprint(name, parent):
    path = FOLDER + '/' + name
    preserve('UI/Inventory/Admission/' + name)
    if u.EditorAssetLibrary.does_asset_exist(path):
        return u.load_asset(path)
    factory = u.WidgetBlueprintFactory()
    factory.set_editor_property('parent_class', parent)
    result = ASSETS.create_asset(name, FOLDER, None, factory)
    assert result
    return result


def make(bp, cls, name):
    result = EDIT.add_template(bp, cls, name)
    assert result
    if isinstance(result, u.PanelWidget):
        result.clear_children()
    return result


def attach(parent, child, padding=3, fill=False):
    slot = parent.add_child(child)
    if hasattr(slot, 'set_padding'):
        slot.set_padding(u.Margin(padding, padding, padding, padding))
    if isinstance(slot, (u.HorizontalBoxSlot, u.VerticalBoxSlot)):
        slot.set_size(u.SlateChildSize(1, u.SlateSizeRule.FILL if fill else u.SlateSizeRule.AUTOMATIC))
        slot.set_vertical_alignment(u.VerticalAlignment.V_ALIGN_CENTER if isinstance(slot, u.HorizontalBoxSlot) else u.VerticalAlignment.V_ALIGN_FILL)
    return slot


def text(key):
    return EDIT.string_table_text(TABLE, 'Invenza.Admission.' + key)


def label(bp, name, key=None, font_size=15):
    cls = u.load_class(None, ROOT + 'UI/Core/WBP_LabelBaseText_Wrapped.WBP_LabelBaseText_Wrapped_C') if name == 'StatusLabel' else LABEL
    result = make(bp, cls, name)
    font = FONT.copy()
    font.size = font_size
    result.set_editor_property('font_info', font)
    result.set_editor_property('color', u.SlateColor(specified_color=GOLD))
    result.set_editor_property('text', text(key) if key else u.Text(''))
    return result


def sized(bp, name, child, width=None, height=None):
    box = make(bp, u.SizeBox, name)
    if width is not None:
        box.set_width_override(width)
    if height is not None:
        box.set_height_override(height)
    attach(box, child, 0)
    return box


def action(bp, name, key, image=None):
    result = make(bp, BUTTON, name)
    result.set_editor_property('is_toggle_button', False)
    result.set_editor_property('default_text', u.Text('') if image else text(key))
    if image:
        result.set_editor_property('default_image', u.load_asset(ROOT + image))
        result.set_editor_property('default_size', u.Vector2D(30, 30))
    result.set_tool_tip_text(text(key))
    return result


preserve('Data/Localization/ST_InvenzaRuntimeTexts')
entries = {
    'Title': 'Storage rules — {0}',
    'Search': 'Find category...',
    'Category': 'Category',
    'AllItems': 'All items',
    'Expand': 'Expand or collapse this category',
    'Allow': 'Allow',
    'Exclude': 'Exclude',
    'State': 'Effective rule',
    'Mixed': 'Partially allowed',
    'Blocked': 'Blocked',
    'AllowedAction': 'Allowed. Click to block this item or the entire category.',
    'BlockedAction': 'Blocked. Click to allow this item or the entire category.',
    'PartialAction': 'Partially allowed. Click to allow the entire category.',
    'Allowed': 'Allowed',
    'AllowedInherited': 'Allowed (inherited/default)',
    'Excluded': 'Excluded',
    'ExcludedInherited': 'Excluded (inherited/default)',
    'Help': 'Select an item or an entire category. A yellow symbol means that only some items are allowed.',
    'ReadOnly': 'View only. These storage rules cannot be changed during play.',
    'Submitting': 'Applying storage rules...',
    'Rejected': 'Rules were not applied. Access may have ended or editing is locked.',
    'Apply': 'Apply',
    'Close': 'Close / discard unapplied changes',
    'CloseLabel': 'Close',
    'RulesAction': 'View or edit the categories accepted by this storage.',
    'PlaceAllowed': 'Move all permitted items from your inventory into this storage, including item types not already present.',
}
for key, value in entries.items():
    EDIT.set_string_table_entry(TABLE, 'Invenza.Admission.' + key, value)
assert u.EditorAssetLibrary.save_loaded_asset(TABLE, only_if_is_dirty=False)

# The status textures contain only simple geometric symbols; they are editable assets.
def import_symbol(name):
    path = ROOT + 'Textures/UI/Admission/' + name
    texture = u.load_asset(path) if u.EditorAssetLibrary.does_asset_exist(path) else None
    if not texture:
        task = u.AssetImportTask()
        task.set_editor_property('filename', str(PROJECT / 'Scripts/UI/Data/AdmissionIcons' / (name + '.png')))
        task.set_editor_property('destination_path', ROOT + 'Textures/UI/Admission')
        task.set_editor_property('destination_name', name)
        task.set_editor_property('automated', True)
        task.set_editor_property('save', False)
        ASSETS.import_asset_tasks([task])
        texture = u.load_asset(path)
    assert texture, name
    texture.set_editor_property('lod_group', u.TextureGroup.TEXTUREGROUP_UI)
    assert u.EditorAssetLibrary.save_loaded_asset(texture, only_if_is_dirty=False)
    return texture


def icon_brush(texture):
    return u.SlateBrush(resource_object=texture)


row_bp = blueprint('WBP_InventoryCategoryTreeRow', u.InventoryCategoryTreeRowWidget)
row = make(row_bp, u.HorizontalBox, 'CategoryRow')
attach(row, sized(row_bp, 'Indentation', make(row_bp, u.Spacer, 'IndentSpace'), 0, 1), 0)
expand = action(row_bp, 'ExpandButton', 'Expand', 'Textures/UI/Arrows/T_Down')
expand.set_editor_property('default_size', u.Vector2D(22, 22))
attach(row, sized(row_bp, 'ExpandButtonSize', expand, 24, 24), 2)
attach(row, label(row_bp, 'CategoryLabel'), 3, True)
state_button = make(row_bp, WITH_SLOT_BUTTON, 'StateButton')
state_button.set_editor_property('is_toggle_button', False)
state_button.set_editor_property('default_text', u.Text(''))
state_icon = make(row_bp, u.Image, 'StateIcon')
state_icon.set_editor_property('brush', icon_brush(import_symbol('T_AdmissionAllowed')))
EDIT.set_slot_content(state_button, 'NamedSlot_21', sized(row_bp, 'StateIconSize', state_icon, 26, 26))
attach(row, sized(row_bp, 'StateButtonSize', state_button, 34, 30), 3)
EDIT.set_root(row_bp, sized(row_bp, 'CategoryRowHeight', row, height=34))
assert EDIT.compile_widget(row_bp)
row_defaults = u.get_default_object(row_bp.generated_class())
for prop, name in [('allowed_state_brush', 'T_AdmissionAllowed'), ('blocked_state_brush', 'T_AdmissionBlocked'), ('partial_state_brush', 'T_AdmissionPartial')]:
    row_defaults.set_editor_property(prop, icon_brush(import_symbol(name)))
row_defaults.set_editor_property('category_expansion_brush', icon_brush(u.load_asset(ROOT + 'Textures/UI/Arrows/T_Down')))
save(row_bp)

bp = blueprint('WBP_InventoryAdmission', u.InventoryAdmissionWidget)
body = make(bp, u.VerticalBox, 'AdmissionBody')
attach(body, label(bp, 'TitleLabel', font_size=22), 7)
search = make(bp, u.EditableTextBox, 'SearchInput')
search.set_editor_property('hint_text', text('Search'))
search_font = FONT.copy()
search_font.size = 16
search_style = search.get_editor_property('widget_style')
search_text_style = search_style.get_editor_property('text_style')
search_text_style.set_editor_property('font', search_font)
search_text_style.set_editor_property('color_and_opacity', u.SlateColor(specified_color=GOLD))
search_style.set_editor_property('text_style', search_text_style)
search_style.set_editor_property('foreground_color', u.SlateColor(specified_color=GOLD))
for brush_name in ['background_image_normal', 'background_image_hovered', 'background_image_focused', 'background_image_read_only']:
    brush = search_style.get_editor_property(brush_name)
    brush.set_editor_property('tint_color', u.SlateColor(specified_color=u.LinearColor(.11, .08, .045, 1)))
    search_style.set_editor_property(brush_name, brush)
search.set_editor_property('widget_style', search_style)
attach(body, search, 7)
tree = make(bp, u.TreeView, 'CategoryTree')
tree.set_editor_property('entry_widget_class', row_bp.generated_class())
tree.set_editor_property('selection_mode', u.SelectionMode.NONE)
attach(body, tree, 7, True)
status = label(bp, 'StatusLabel', 'Help', 13)
attach(body, status, 7)
footer = make(bp, u.HorizontalBox, 'AdmissionFooter')
for name, key in [('ApplyButton', 'Apply'), ('CloseButton', 'CloseLabel')]:
    button = action(bp, name, key)
    if name == 'CloseButton':
        button.set_tool_tip_text(text('Close'))
    attach(footer, sized(bp, name + 'Size', button, 150, 38), 5)
attach(body, footer, 7).set_horizontal_alignment(u.HorizontalAlignment.H_ALIGN_RIGHT)
background = make(bp, u.Border, 'AdmissionBackground')
background.set_brush_color(BACKGROUND)
background.set_padding(u.Margin(14, 14, 14, 14))
attach(background, body, 0)
EDIT.set_root(bp, sized(bp, 'AdmissionWindowSize', background, 640, 560))
assert EDIT.compile_widget(bp)
u.get_default_object(bp.generated_class()).set_editor_property('is_focusable', True)
save(bp)

if UPDATE_OPERATIONS_PANEL:
    relative = 'UI/Core/OperationsPanel/WBP_Inventory_OperationsPanel'
    preserve(relative)
    panel_bp = u.load_asset(ROOT + relative)
    panel_bp.modify()
    widget_tree = u.find_object(panel_bp, 'WidgetTree')
    place = u.find_object(widget_tree, 'Button_PlaceAll')
    panel = place.get_parent()
    panel.modify()


    def snapshot(root_widget):
        result = {}
        def visit(widget):
            result[widget.get_path_name()] = (str(widget.get_visibility()), widget.get_render_opacity())
            if isinstance(widget, u.PanelWidget):
                for index in range(widget.get_children_count()):
                    visit(widget.get_child_at(index))
        visit(panel)
        return result


    before = snapshot(panel)
    for name, key, texture in [
        ('Button_PlaceAllowed', 'PlaceAllowed', 'Textures/UI/Operations/T_PlaceAll'),
        ('Button_AdmissionRules', 'RulesAction', 'Textures/ItemCategory/T_Allcategory'),
    ]:
        existing = u.find_object(widget_tree, name)
        button = existing if existing else action(panel_bp, name, key, texture)
        if not existing:
            attach(panel, sized(panel_bp, name + 'Size', button, 35, 35), 3).set_horizontal_alignment(u.HorizontalAlignment.H_ALIGN_CENTER)
    after = snapshot(panel)
    assert all(after.get(key) == value for key, value in before.items()), 'Existing UI hit testing or opacity changed'
    save(panel_bp)
(PROJECT / 'Saved/InventoryAdmissionSimple/asset_report.json').write_text(json.dumps({
    'saved': [FOLDER + '/WBP_InventoryAdmission', FOLDER + '/WBP_InventoryCategoryTreeRow', TABLE.get_path_name()],
    'operations_panel_unchanged': not UPDATE_OPERATIONS_PANEL,
}, indent=2), encoding='utf-8')
u.log('INVENZA_ADMISSION_SIMPLE_ASSETS_OK')
