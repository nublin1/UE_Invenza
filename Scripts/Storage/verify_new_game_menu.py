# Nublin Studio 2026 All Rights Reserved.
"""Exercise new-game menu, travel, manual save preservation and load in an isolated profile."""
from pathlib import Path
import hashlib
import json
import time
import traceback
import uuid
import unreal as u

OUT = Path(u.Paths.project_saved_dir()) / "NewGameNPC20261028"
SAVE_DIR = Path(u.Paths.project_saved_dir()) / "SaveGames"
PROFILE = "NewGameProbe_" + uuid.uuid4().hex
EDIT = u.InvenzaWidgetEditingLibrary
FIND = u.InvenzaTakeAmountValidationLibrary.find_preview_widget
settings = u.get_default_object(u.InvenzaSaveSettings)
assert not settings.get_editor_property("bAutoSave")
assert not settings.get_editor_property("bSaveOnExit")
settings.set_editor_property("Profile", PROFILE)
settings.set_editor_property("bEnableInPIE", True)
settings.set_editor_property("bAutoLoad", True)
assert u.get_editor_subsystem(u.LevelEditorSubsystem).load_level("/Game/Frontend/Maps/L_MainMenu")
assert EDIT.start_standalone_preview()
callback = None
phase = 0
last_check = 0
phase_started = time.monotonic()
first_ids = None
manual_hashes = None
menu_ref = None
done = False


def hashes():
    return {file.name: hashlib.sha256(file.read_bytes()).hexdigest()
            for file in SAVE_DIR.glob("Invenza_*" + PROFILE + "_*.sav")}


def press(menu, name):
    button = FIND(FIND(menu, name), "MainButton")
    assert EDIT.press_preview_button(button), name


def game_state():
    world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_game_world()
    if not world:
        return None
    pawn = u.GameplayStatics.get_player_pawn(world, 0)
    if not pawn:
        return None
    manager = pawn.get_component_by_class(u.IInventoryManager)
    layer = EDIT.get_preview_inventory_ui(manager)
    if not layer:
        return None
    layers = [layer]
    layer = layers[0]
    pawn = layer.get_owning_player_pawn()
    world = pawn.get_outer().get_outer()
    prefix = world.get_path_name() + ":"
    bag = pawn.get_component_by_class(u.IInventoryManager).get_editor_property("MainPawnInventoryRef")
    if not bag or abs(bag.get_editor_property("InventoryTotalWeight") - 35) > 0.01:
        return None
    npcs = [actor for actor in u.ObjectIterator(u.InventoryMannequin) if actor.get_path_name().startswith(prefix)]
    if len(npcs) != 1:
        return None
    npc_bag = npcs[0].get_component_by_class(u.CharacterInventoryAccessComponent).get_editor_property("MainLootInventory")
    if not npc_bag or abs(npc_bag.get_editor_property("InventoryTotalWeight") - 5) > 0.01:
        return None
    return layer, world, bag, npc_bag


def ids(state):
    return [state[2].get_editor_property("InventoryContainerID"), state[3].get_editor_property("InventoryContainerID")]


def tick(delta):
    global phase, last_check, phase_started, first_ids, manual_hashes, menu_ref, done
    now = time.monotonic()
    if now - last_check < 0.25:
        return
    last_check = now
    try:
        if done:
            assert hashes() == manual_hashes, "PIE exit wrote or changed saves"
            (OUT / "new-game-verification.json").write_text(json.dumps(dict(
                profile=PROFILE, new_game_from_main_menu=True, new_game_during_play=True,
                cancel_confirmation=True, existing_manual_save_preserved=True,
                fresh_player_and_npc_identities=True, manual_load_restores_both_backpacks=True,
                no_exit_or_periodic_save=True, files=list(manual_hashes)), indent=2), encoding="utf-8")
            u.log("INVENZA_NEW_GAME_MENU_VERIFIED")
            u.unregister_slate_pre_tick_callback(callback)
            u.SystemLibrary.quit_editor()
            return
        (OUT / "new-game-progress.json").write_text(json.dumps(dict(phase=phase, files=hashes()), indent=2), encoding="utf-8")
        if now - phase_started > 45:
            raise RuntimeError("New game probe timed out in phase " + str(phase))
        if phase == 0:
            menus = [menu for menu in u.ObjectIterator(u.FrontendMenuWidget)
                     if menu.get_owning_player() and "UEDPIE_" in menu.get_owning_player().get_path_name()]
            if not menus or now - phase_started < 3:
                return
            menu = menus[0]
            assert not hashes()
            press(menu, "NewGame")
            assert FIND(menu, "Pages").get_active_widget().get_name() == "NewGameConfirmation"
            assert EDIT.render_widget(menu, u.Vector2D(1280, 800), str(OUT / "new-game-confirmation.png"))
            EDIT.release_preview()
            press(menu, "CancelNewGame")
            assert FIND(menu, "Pages").get_active_widget().get_name() == "HomePage"
            assert EDIT.render_widget(menu, u.Vector2D(1280, 800), str(OUT / "main-menu.png"))
            EDIT.release_preview()
            press(menu, "NewGame")
            press(menu, "ConfirmNewGame")
            phase = 1
            phase_started = now
            return
        state = game_state()
        if not state:
            return
        layer, world, bag, npc_bag = state
        save = EDIT.get_preview_save_subsystem(world)
        if phase == 1:
            if now - phase_started < 3:
                return
            assert bag.get_editor_property("InventorySettings").inventory_max_weight_capacity == 50
            assert save.save_session("Manual1", False)
            manual_hashes = hashes()
            assert len(manual_hashes) == 1
            first_ids = ids(state)
            menu_class = u.load_asset("/Game/Frontend/UI/WBP_FrontendMenu").generated_class()
            menu_ref = EDIT.create_preview(world, menu_class)
            menu_ref.add_to_viewport(100)
            press(menu_ref, "NewGame")
            press(menu_ref, "ConfirmNewGame")
            menu_ref = None
            phase = 2
            phase_started = now
            return
        if phase == 2:
            if now - phase_started < 3 or ids(state) == first_ids:
                return
            assert all(current != previous for current, previous in zip(ids(state), first_ids))
            assert hashes() == manual_hashes
            assert save.load_session("Manual1")
            phase = 3
            phase_started = now
            return
        if phase == 3:
            if now - phase_started < 2 or ids(state) != first_ids:
                return
            assert hashes() == manual_hashes
            EDIT.release_preview()
            u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
            done = True
    except Exception:
        (OUT / "new-game-error.txt").write_text(traceback.format_exc(), encoding="utf-8")
        EDIT.release_preview()
        u.log_error(traceback.format_exc())
        u.unregister_slate_pre_tick_callback(callback)
        u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
        u.SystemLibrary.quit_editor()


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_pre_tick_callback(tick)
