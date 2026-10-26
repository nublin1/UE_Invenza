"""Author only the save-menu assets in Unreal Editor; no runtime UMG construction.

Texts are imported from the adjacent Data/InvenzaSaveTexts.csv source.
Existing shared Core widgets are referenced without modifying or saving them.
"""
from pathlib import Path
import csv
import unreal as u

PROJECT = Path(u.Paths.project_dir())
ROOT = '/InventorySystemInvenzaPlugin/'
# Read helper definitions only: do not run the recipe migration below this marker.
helpers = (PROJECT/'Scripts/UI/restyle_craft_choose.py').read_text(encoding='utf-8')
exec(helpers.split('# A reusable wrapping label')[0])
BUTTON = u.load_class(None, ROOT+'UI/Core/Buttons/WBP_UIButtonWithSlot.WBP_UIButtonWithSlot_C')


def create(path, parent):
    full = ROOT+path
    if u.EditorAssetLibrary.does_asset_exist(full):
        return u.load_asset(full)
    factory = u.WidgetBlueprintFactory()
    factory.set_editor_property('parent_class', parent)
    directory, name = full.rsplit('/', 1)
    result = u.AssetToolsHelpers.get_asset_tools().create_asset(name, directory, None, factory)
    assert result, full
    return result


table_path = ROOT+'Data/Localization/ST_InvenzaSaveTexts'
table = u.load_asset(table_path) if u.EditorAssetLibrary.does_asset_exist(table_path) else None
if table is None:
    directory, name = table_path.rsplit('/', 1)
    table = u.AssetToolsHelpers.get_asset_tools().create_asset(name, directory, None, u.StringTableFactory())
assert table
with (PROJECT/'Scripts/SaveLoad/Data/InvenzaSaveTexts.csv').open(encoding='utf-8', newline='') as stream:
    for row in csv.DictReader(stream):
        EDIT.set_string_table_entry(table, row['Key'], row['SourceString'])
assert u.EditorAssetLibrary.save_loaded_asset(table, only_if_is_dirty=False)

# Local Core-label variant: automatic desired height instead of the inherited canvas.
label_path = ROOT+'UI/SaveLoad/WBP_SaveLabel'
if not u.EditorAssetLibrary.does_asset_exist(label_path):
    assert u.EditorAssetLibrary.duplicate_asset(ROOT+'UI/Core/WBP_LabelBaseText', label_path)
lbp = u.load_asset(label_path)
text_block = make(lbp, u.TextBlock, 'MainTextBlock')
text_block.set_auto_wrap_text(False)
text_block.set_editor_property('wrap_text_at', 520.)
text_block.remove_from_parent()
EDIT.set_root(lbp, text_block)
assert EDIT.compile_widget(lbp)
style_label(u.get_default_object(lbp.generated_class()), None, 16)
save(lbp)
LABEL = WRAPPED = lbp.generated_class()


def text(bp, name, key=None, font_size=16, bold=False):
    result = label(bp, name, None, font_size, bold)
    result.set_editor_property('text', EDIT.string_table_text(table, key) if key else u.Text(''))
    return result


def action(bp, name, key, font_size=16):
    result = make(bp, BUTTON, name)
    result.set_visibility(VIS.VISIBLE)
    EDIT.set_slot_content(result, 'NamedSlot_21', text(bp, name+'Label', key, font_size))
    return size(bp, name+'Size', result, min_height=36)


bp = create('UI/SaveLoad/WBP_InvenzaSaveMenu', u.InvenzaSaveWidget)
body = panel(bp, u.VerticalBox, 'SaveBody')
header = panel(bp, u.HorizontalBox, 'SaveHeader')
attach(header, text(bp, 'TitleLabel', 'Title', 24, True), margin(0, 0, 12, 0), True)
attach(header, action(bp, 'CloseButton', 'Close', 15))
attach(body, header, margin(0, 0, 0, 12))
attach(body, text(bp, 'ScopeLabel', 'Scope', 15), margin(0, 0, 0, 14))
attach(body, action(bp, 'ContinueButton', 'Continue'), margin(0, 0, 0, 5))
attach(body, action(bp, 'QuickSaveButton', 'QuickSave'), margin(0, 0, 0, 5))
manual = panel(bp, u.HorizontalBox, 'ManualSaveButtons')
for index in range(1, 4):
    attach(manual, action(bp, 'ManualSave%dButton'%index, 'ManualSave%d'%index, 14), margin(0, 0, 8 if index<3 else 0, 0), True)
attach(body, manual, margin(0, 0, 0, 14))
slots = make(bp, u.ComboBoxString, 'Slots')
slots.set_visibility(VIS.VISIBLE)
slots.set_editor_property('font', BASE_FONT)
attach(body, size(bp, 'SlotsSize', slots, min_height=34), margin(0, 0, 0, 5))
attach(body, action(bp, 'LoadButton', 'Load'), margin(0, 0, 0, 14))
trustrow = panel(bp, u.HorizontalBox, 'TrustRow')
trust = make(bp, u.CheckBox, 'Trust'); trust.set_visibility(VIS.VISIBLE)
attach(trustrow, trust, margin(0, 0, 8, 0)).set_vertical_alignment(V.V_ALIGN_CENTER)
attach(trustrow, text(bp, 'TrustLabel', 'Trust', 15), 0, True)
attach(body, trustrow, margin(0, 0, 0, 5))
attach(body, text(bp, 'TrustDescriptionLabel', 'TrustDescription', 13), margin(0, 0, 0, 14))
attach(body, action(bp, 'RecoverButton', 'Recover'), margin(0, 0, 0, 5))
attach(body, action(bp, 'ResetButton', 'Reset'), margin(0, 0, 0, 8))
confirmation = panel(bp, u.VerticalBox, 'Confirmation')
attach(confirmation, text(bp, 'ConfirmationLabel', 'ConfirmReset', 14), margin(0, 0, 0, 6))
confirmrow = panel(bp, u.HorizontalBox, 'ConfirmationButtons')
attach(confirmrow, action(bp, 'ConfirmResetButton', 'Confirm', 15), margin(0, 0, 8, 0), True)
attach(confirmrow, action(bp, 'CancelResetButton', 'Cancel', 15), 0, True)
attach(confirmation, confirmrow)
attach(body, confirmation, margin(0, 0, 0, 12))
confirmation.set_visibility(VIS.COLLAPSED)
attach(body, text(bp, 'StatusLabel', None, 14), margin(0, 12, 0, 4))
scroll = panel(bp, u.ScrollBox, 'SaveScroll'); scroll.set_visibility(VIS.VISIBLE)
attach(scroll, body)
content = frame(bp, 'SaveFrame', surface(bp, 'SaveSurface', scroll, BG, 20))
content = size(bp, 'SaveWindowSize', content, 600, 730)
fit = panel(bp, u.ScaleBox, 'ScreenFit')
fit.set_stretch(u.Stretch.SCALE_TO_FIT)
fit.set_stretch_direction(u.StretchDirection.DOWN_ONLY)
fit_slot = attach(fit, content)
fit_slot.set_horizontal_alignment(H.H_ALIGN_CENTER)
fit_slot.set_vertical_alignment(V.V_ALIGN_CENTER)
canvas = panel(bp, u.CanvasPanel, 'SaveCanvas')
slot = canvas.add_child(fit)
slot.set_anchors(u.Anchors(u.Vector2D(0, 0), u.Vector2D(1, 1)))
slot.set_offsets(margin(16))
EDIT.set_root(bp, canvas)
assert EDIT.compile_widget(bp)
u.get_default_object(bp.generated_class()).set_editor_property('is_focusable', True)
save(bp)
u.log('INVENZA_SAVE_MENU_ASSETS_OK')

