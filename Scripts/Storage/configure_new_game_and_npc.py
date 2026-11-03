# Nublin Studio 2026 All Rights Reserved.
"""Author only the requested frontend entry and NPC/vendor examples."""
from pathlib import Path
import json
import shutil
import unreal as u

PROJECT = Path(u.Paths.project_dir()).resolve()
OUT = PROJECT / "Saved/NewGameNPC20261028"
ROOT = "/InventorySystemInvenzaPlugin/"
EDIT = u.InvenzaWidgetEditingLibrary
ALLOWED = {
    "/Game/Frontend/UI/WBP_FrontendMenu",
    "/Game/Frontend/Localization/ST_FrontendMenu",
    ROOT + "Data/Localization/ST_InvenzaRuntimeTexts",
    ROOT + "Maps/Map_Ex",
}
saved = []


def backup(path, extension=".uasset"):
    assert path in ALLOWED
    relative = ("Content/" + path.removeprefix("/Game/") if path.startswith("/Game/")
                else "Plugins/InventorySystemInvenzaPlugin/Content/" + path.removeprefix(ROOT)) + extension
    destination = OUT / "Before" / relative
    if not destination.exists():
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(PROJECT / relative, destination)


def save(asset):
    path = asset.get_path_name().split(".")[0]
    assert path in ALLOWED
    assert u.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
    saved.append(path)


text_path = "/Game/Frontend/Localization/ST_FrontendMenu"
backup(text_path)
texts = u.load_asset(text_path)
for key, value in {
    "NewGame": "Новая игра",
    "NewGameHint": "Начать новое прохождение",
    "NewGameTitle": "Начать новую игру?",
    "NewGameDescription": "Несохранённый прогресс текущей игры будет потерян. Ручные сохранения останутся доступны.",
    "ConfirmNewGame": "Начать новую игру",
    "CancelNewGame": "Отмена",
}.items():
    EDIT.set_string_table_entry(texts, key, value)
save(texts)

path = "/Game/Frontend/UI/WBP_FrontendMenu"
backup(path)
bp = u.load_asset(path)
tree = u.find_object(bp, "WidgetTree")
original = {obj.get_name(): (str(obj.get_visibility()), obj.get_render_opacity())
            for obj in u.ObjectIterator(u.Widget) if obj.get_outer() == tree}
button_class = u.load_asset("/Game/Frontend/UI/WBP_MenuButton").generated_class()
label_class = u.load_asset("/Game/Frontend/UI/WBP_MenuLabel").generated_class()


def label(name, key):
    result = EDIT.add_template(bp, label_class, name)
    result.set_editor_property("text", EDIT.string_table_text(texts, key))
    return result


def button(name, key):
    result = EDIT.add_template(bp, button_class, name)
    body = EDIT.add_template(bp, u.SizeBox, name + "ContentSize")
    body.set_height_override(42)
    body.add_child(label(name + "Label", key))
    EDIT.set_slot_content(result, "ContentSlot", body)
    return result


home = u.find_object(tree, "HomePage")
if not u.find_object(tree, "NewGame"):
    start = button("NewGame", "NewGame")
    start.set_tool_tip_text(EDIT.string_table_text(texts, "NewGameHint"))
    slot = EDIT.insert_template_child(home, 1, start)
    assert slot
    slot.set_padding(u.Margin(0, 0, 0, 10))

pages = u.find_object(tree, "Pages")
if not u.find_object(tree, "NewGameConfirmation"):
    confirmation = EDIT.add_template(bp, u.VerticalBox, "NewGameConfirmation")
    for child in [label("NewGameTitle", "NewGameTitle"), label("NewGameDescription", "NewGameDescription"),
                  button("ConfirmNewGame", "ConfirmNewGame"), button("CancelNewGame", "CancelNewGame")]:
        slot = confirmation.add_child(child)
        slot.set_padding(u.Margin(0, 0, 0, 16))
    pages.add_child(confirmation)
assert original == {name: (str(u.find_object(tree, name).get_visibility()), u.find_object(tree, name).get_render_opacity())
                    for name in original}
assert EDIT.compile_widget(bp)
save(bp)

runtime_path = ROOT + "Data/Localization/ST_InvenzaRuntimeTexts"
backup(runtime_path)
runtime_texts = u.load_asset(runtime_path)
EDIT.set_string_table_entry(runtime_texts, "ExampleStorage.Trader", "Trader pockets")
save(runtime_texts)

map_path = ROOT + "Maps/Map_Ex"
backup(map_path, ".umap")
assert u.get_editor_subsystem(u.LevelEditorSubsystem).load_level(map_path)
actors = u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors()
npc = next(actor for actor in actors if isinstance(actor, u.InventoryMannequin))
access = npc.get_component_by_class(u.CharacterInventoryAccessComponent)
manager = npc.get_component_by_class(u.IInventoryManager)
equipment = npc.get_component_by_class(u.EquipmentComponent)
previous_equipment_rows = [row.export_text() for row in equipment.get_editor_property("SlotRows")]
equipment_table = u.load_asset(ROOT + "Data/DT_Equipment")
(OUT / "npc-previous-equipment-rows.json").write_text(json.dumps(previous_equipment_rows, indent=2), encoding="utf-8")
assert EDIT.set_equipment_definition_rows(equipment, [u.DataTableRowHandle(data_table=equipment_table, row_name=name)
    for name in u.DataTableFunctionLibrary.get_data_table_row_names(equipment_table)])
assert manager
character = u.load_asset(ROOT + "Blueprints/Examples/BP_Character")
player_manager = next(obj for obj in u.ObjectIterator(u.IInventoryManager)
    if obj.get_name() == "BP_InvManager_GEN_VARIABLE" and obj.get_path_name().startswith(character.get_path_name().split(".")[0]+"."))
gear = next(entry.copy() for entry in player_manager.get_editor_property("StartupInventories")
            if "UI.Invenza.Equipment" in entry.settings.inventory_tag.export_text())
settings = gear.get_editor_property("settings").copy()
settings.set_editor_property("container_widget_class", u.load_asset(ROOT + "UI/Equipment/WBP_EquipmentInv").generated_class())
settings.set_editor_property("bIsReferenceContainer", False)
gear.set_editor_property("settings", settings)
manager.set_editor_property("StartupInventories", [gear])
ordinary = list(access.get_editor_property("BagStartingItems"))
if ordinary:
    manager.set_editor_property("GeneralStartingItems", ordinary)
access.set_editor_property("BagStartingItems", [])
access.set_editor_property("bCreateDefaultInventories", False)
access.set_editor_property("bUsePortableBackpacks", True)
manager.set_editor_property("bUsePortableBackpacks", True)
manager.set_editor_property("ExampleGroundBackpacks", [])

vendors = []
for actor in actors:
    vendor = actor.get_component_by_class(u.VendorComponent)
    if not vendor:
        continue
    vendor_manager = actor.get_component_by_class(u.IInventoryManager)
    if not vendor_manager:
        continue
    # Keep the existing stock identifier and rows for save compatibility.
    startups = list(vendor_manager.get_editor_property("StartupInventories"))
    assert any(entry.settings.inventory_type == u.InventoryType.VENDOR_INVENTORY for entry in startups)
    vendor_manager.set_editor_property("ExampleGroundBackpacks", [])
    vendors.append(actor.get_path_name())
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
assert u.EditorLoadingAndSavingUtils.save_map(world, map_path)
saved.append(map_path)
(OUT / "asset-changes.json").write_text(json.dumps(dict(saved=saved, npc=npc.get_path_name(),
    npc_equipment_rows_before=previous_equipment_rows, npc_starting_backpack=gear.export_text(), ordinary=[entry.export_text() for entry in manager.get_editor_property("GeneralStartingItems")],
    vendors=vendors, existing_visibility_and_opacity_preserved=True), indent=2), encoding="utf-8")
u.log("INVENZA_NEW_GAME_NPC_ASSETS_CONFIGURED")
u.SystemLibrary.quit_editor()
