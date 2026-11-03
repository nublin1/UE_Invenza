# Nublin Studio 2026 All Rights Reserved.
"""Author the amount header, settings references and String Table entries.

No existing Widget Blueprint, map, animation or inventory definition is saved.
Requires the project-only InvenzaUIEditor bridge and the new native modal classes.
"""
from pathlib import Path
import shutil
import unreal as u

ROOT = "/InventorySystemInvenzaPlugin/"
PROJECT = Path(u.Paths.project_dir())
CONTENT = PROJECT / "Plugins/InventorySystemInvenzaPlugin/Content"
BACKUP = PROJECT / "Saved/TakeAmount20261027/Before"
EDIT = u.InvenzaWidgetEditingLibrary
ASSETS = u.AssetToolsHelpers.get_asset_tools()
u.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous([ROOT], force_rescan=True)
LABEL = u.load_class(None, ROOT + "UI/Core/WBP_LabelBaseText.WBP_LabelBaseText_C")
WRAPPED_LABEL = u.load_class(None, ROOT + "UI/Core/WBP_LabelBaseText_Wrapped.WBP_LabelBaseText_Wrapped_C")
SETTINGS = u.load_asset(ROOT + "Data/Settings/InvenzaSettings")
TABLE = u.load_asset(ROOT + "Data/Localization/ST_InvenzaRuntimeTexts")
container = u.load_asset(ROOT + "UI/InventoryContainer/WBP_InvContainer")
font_source = u.find_object(u.find_object(container, "WidgetTree"), "InvMoney")
BASE_FONT = font_source.get_editor_property("font_info").copy()
GOLD = u.LinearColor(.82, .65, .36, 1)
assert LABEL and SETTINGS and TABLE


def preserve(relative):
    original = CONTENT / (relative + ".uasset")
    target = BACKUP / (relative + ".uasset")
    if original.exists() and not target.exists():
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(original, target)


def save(bp):
    assert EDIT.compile_widget(bp), bp.get_name()
    assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)


def widget(relative, parent):
    preserve(relative)
    if u.EditorAssetLibrary.does_asset_exist(ROOT + relative):
        bp = u.load_asset(ROOT + relative)
        assert isinstance(u.get_default_object(bp.generated_class()), parent)
        return bp
    folder, name = relative.rsplit("/", 1)
    factory = u.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    result = ASSETS.create_asset(name, ROOT + folder, None, factory)
    assert result
    return result


def make(bp, cls, name):
    result = EDIT.add_template(bp, cls, name)
    assert result
    if isinstance(result, u.PanelWidget):
        result.clear_children()
    return result


def attach(parent, child, padding=4, fill=False):
    slot = parent.add_child(child)
    if hasattr(slot, "set_padding"):
        slot.set_padding(u.Margin(padding, padding, padding, padding))
    if isinstance(slot, (u.HorizontalBoxSlot, u.VerticalBoxSlot)):
        slot.set_size(u.SlateChildSize(1, u.SlateSizeRule.FILL if fill else u.SlateSizeRule.AUTOMATIC))
    return slot


def label(bp, name, size=16, wrapped=False):
    result = make(bp, WRAPPED_LABEL if wrapped else LABEL, name)
    font = BASE_FONT.copy()
    font.size = size
    font.typeface_font_name = "Regular"
    result.set_editor_property("font_info", font)
    result.set_editor_property("text", u.Text(""))
    result.set_editor_property("color", u.SlateColor(specified_color=GOLD))
    return result


def text(key):
    return EDIT.string_table_text(TABLE, "Invenza.TakeAmount." + key)


preserve("Data/Localization/ST_InvenzaRuntimeTexts")
texts = {
    "Action": "Take amount...",
    "Title": "Take items from storage",
    "Take": "Take:",
    "Remaining": "Remaining: {0}",
    "Empty": "No unreserved items are available.",
    "Submitting": "Transferring items...",
    "Invalid": "Choose a valid quantity. No items were moved.",
    "Access": "Storage access is no longer available. No items were moved.",
    "Changed": "The stack changed. Review the updated quantity and confirm again.",
    "Reserved": "The requested quantity is reserved or no longer available.",
    "Capacity": "Not enough inventory space or weight capacity. Choose a smaller quantity.",
    "Unsupported": "This item cannot be transferred with this action.",
    "Busy": "Another inventory operation is in progress. Please try again.",
}
for key, value in texts.items():
    EDIT.set_string_table_entry(TABLE, "Invenza.TakeAmount." + key, value)
assert u.EditorAssetLibrary.save_loaded_asset(TABLE, only_if_is_dirty=False)

header_bp = widget("UI/Core/Modals/ModalHeaderParts/WBP_TakeAmountHeader", u.InventoryTakeAmountHeaderWidget)
body = make(header_bp, u.VerticalBox, "TakeAmountBody")
attach(body, label(header_bp, "TitleLabel", 20), 6)
row = make(header_bp, u.HorizontalBox, "TakeAmountRow")
attach(row, label(header_bp, "TakeLabel"), 6, True)
amount = make(header_bp, u.SpinBox, "AmountInput")
amount.set_editor_property("delta", 1.0)
amount.set_editor_property("min_fractional_digits", 0)
amount.set_editor_property("max_fractional_digits", 0)
amount_font = BASE_FONT.copy()
amount_font.size = 17
amount_font.typeface_font_name = "Regular"
amount.set_editor_property("font", amount_font)
amount_size = make(header_bp, u.SizeBox, "AmountInputSize")
amount_size.set_width_override(140)
amount_size.set_height_override(36)
attach(amount_size, amount, 0)
attach(row, amount_size, 6)
attach(body, row, 4)
attach(body, label(header_bp, "RemainingLabel"), 6)
feedback = label(header_bp, "FeedbackLabel", 14, True)
feedback.set_editor_property("text", u.Text(""))
attach(body, feedback, 6)
root = make(header_bp, u.SizeBox, "TakeAmountHeaderSize")
root.set_width_override(540)
attach(root, body, 4)
EDIT.set_root(header_bp, root)
assert EDIT.compile_widget(header_bp)
cdo = u.get_default_object(header_bp.generated_class())
for prop, key in [("take_format", "Take"), ("remaining_format", "Remaining"),
                  ("no_available_amount_text", "Empty"), ("submitting_text", "Submitting")]:
    cdo.set_editor_property(prop, text(key))
save(header_bp)

preserve("Data/Settings/InvenzaSettings")
actions = dict(SETTINGS.get_editor_property("inv_context_modal_actions"))
actions[u.ObjectInteractionType.TAKE_AMOUNT] = u.ModalActionConfig(
    header_text=text("Title"), step_requirement=u.ModalStepRequirement.REQUIRES_AMOUNT)
SETTINGS.set_editor_property("inv_context_modal_actions", actions)
display = dict(SETTINGS.get_editor_property("modal_actions"))
display[u.ObjectInteractionType.TAKE_AMOUNT] = u.ModalAction(display_text=text("Action"))
SETTINGS.set_editor_property("modal_actions", display)
headers = dict(SETTINGS.get_editor_property("modal_header_widgets"))
headers[u.ModalHeaderType.INVENTORY_TAKE_AMOUNT] = header_bp.generated_class()
SETTINGS.set_editor_property("modal_header_widgets", headers)
footers = dict(SETTINGS.get_editor_property("modal_footer_widgets"))
footers.pop(u.ModalFooterType.INVENTORY_SELECTION, None)
SETTINGS.set_editor_property("modal_footer_widgets", footers)
messages = dict(SETTINGS.get_editor_property("take_amount_messages"))
for result, key in [(u.InventoryTakeAmountResult.INVALID_REQUEST, "Invalid"),
                    (u.InventoryTakeAmountResult.ACCESS_DENIED, "Access"),
                    (u.InventoryTakeAmountResult.SOURCE_CHANGED, "Changed"),
                    (u.InventoryTakeAmountResult.AMOUNT_RESERVED, "Reserved"),
                    (u.InventoryTakeAmountResult.NO_CAPACITY, "Capacity"),
                    (u.InventoryTakeAmountResult.UNSUPPORTED_ITEM, "Unsupported"),
                    (u.InventoryTakeAmountResult.BUSY, "Busy")]:
    messages[result] = text(key)
SETTINGS.set_editor_property("take_amount_messages", messages)
assert u.EditorAssetLibrary.save_loaded_asset(SETTINGS, only_if_is_dirty=False)
u.log("INVENZA_TAKE_AMOUNT_ASSETS_OK")
