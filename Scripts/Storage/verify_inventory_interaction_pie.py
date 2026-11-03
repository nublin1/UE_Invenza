# Nublin Studio 2026 All Rights Reserved.
"""Transient listen-server/client regression probe; no saves or asset writes."""
from pathlib import Path
import json
import time
import traceback
import unreal as u

OUT = Path(u.Paths.project_saved_dir()) / "InventoryInteraction20261028"
ROOT = "/InventorySystemInvenzaPlugin/"
EDIT = u.InvenzaWidgetEditingLibrary
settings = u.get_default_object(u.InvenzaSaveSettings)
for key in ["bEnableInPIE", "bAutoLoad", "bAutoSave", "bSaveOnExit"]:
    settings.set_editor_property(key, False)
assert u.get_editor_subsystem(u.LevelEditorSubsystem).load_level(ROOT + "Maps/Map_Ex")
assert u.InvenzaTakeAmountValidationLibrary.start_network_preview()
deadline = time.monotonic() + 55
phase = 0
last_check = 0
ready_at = 0
records = []
drops = []
done = False
callback = None


def inventory_of(panel):
    return panel.get_inventory_widget_from_container_slot().get_editor_property("InventoryRef")


def snapshot(panel):
    slot = panel.slot
    return dict(parent=panel.get_parent(), position=slot.get_position(), size=slot.get_size(),
                anchors=slot.get_anchors().export_text(), alignment=slot.get_alignment(),
                auto=slot.get_auto_size(), visibility=panel.get_visibility())


def tick(delta):
    global phase, last_check, ready_at, done
    if done:
        u.unregister_slate_pre_tick_callback(callback)
        u.SystemLibrary.quit_editor()
        return
    now = time.monotonic()
    if now - last_check < 0.3:
        return
    last_check = now
    try:
        if now > deadline:
            raise RuntimeError("Inventory interaction probe timeout, phase " + str(phase))
        layers = [layer for layer in u.ObjectIterator(u.GameMenuLayerInv)
                  if layer.get_owning_player_pawn() and "UEDPIE_" in layer.get_owning_player_pawn().get_path_name()]
        if len(layers) < 2:
            return
        if phase == 0:
            for layer in layers:
                pawn = layer.get_owning_player_pawn()
                manager = pawn.get_component_by_class(u.IInventoryManager)
                if not manager.get_editor_property("MainPawnInventoryRef"):
                    return
            for manager in u.ObjectIterator(u.IInventoryManager):
                if manager.get_owner() and "UEDPIE_" in manager.get_owner().get_path_name():
                    EDIT.add_preview_player_storage(manager)
            phase = 1
            return
        if phase == 1:
            for layer in layers:
                pawn = layer.get_owning_player_pawn()
                manager = pawn.get_component_by_class(u.IInventoryManager)
                inventories = list(manager.get_interaction_inventories())
                if len(inventories) != 2:
                    return
                bag = manager.get_editor_property("MainPawnInventoryRef")
                assert bag.get_editor_property("InventorySettings").inventory_max_weight_capacity == 50
                panels = list(manager.get_interaction_inventory_widgets())
                assert len(panels) == 2 and len(set(panels)) == 2
                assert list(manager.get_interaction_inventory_widgets()) == panels
                # Add newly created preview pockets to the ordinary HUD before borrowing them.
                for panel in panels:
                    if not panel.get_parent():
                        layer.add_pawn_inv_container_widget(panel)
                startup = u.InventoryStartupData()
                config = bag.get_editor_property("InventorySettings").copy()
                config.set_editor_properties(dict(custom_inventory_id="PreviewExternal", inventory_type=u.InventoryType.LOOT_CONTAINER,
                                                  container_title=u.Text("Preview container")))
                startup.set_editor_property("settings", config)
                collection = pawn.get_component_by_class(u.InventoryCollectionComponent)
                external = u.InventoryBase.create_inventory_advanced(pawn, startup, pawn, collection)
                if not layer.get_editor_property("bInventoryOpen"):
                    layer.toggle_inventory_layout()
                before = [snapshot(panel) for panel in panels]
                count_before = len(layer.get_all_pawn_inv_containers())
                # Repeat the exact previously broken path: inventory already open, then a container.
                for repeat in range(4):
                    manager.open_secondary_inventory(external, u.InteractableType.CONTAINER)
                    dual = layer.get_editor_property("DualInventoryWidget")
                    assert u.InvenzaTakeAmountValidationLibrary.find_preview_widget(dual, "PlayerInventories").get_children_count() == 2
                    assert list(manager.get_interaction_inventory_widgets()) == panels
                    layer.close_dual_inventory_view()
                    assert layer.get_editor_property("bInventoryOpen")
                    assert [snapshot(panel) for panel in panels] == before
                    assert len(layer.get_all_pawn_inv_containers()) == count_before
                # Switching external containers must remove the former window without copying player panels.
                other_startup = u.InventoryStartupData()
                other_config = config.copy()
                other_config.set_editor_property("custom_inventory_id", "PreviewExternalSecond")
                other_startup.set_editor_property("settings", other_config)
                other_external = u.InventoryBase.create_inventory_advanced(pawn, other_startup, pawn, collection)
                manager.open_secondary_inventory(external, u.InteractableType.CONTAINER)
                manager.open_secondary_inventory(other_external, u.InteractableType.CONTAINER)
                assert list(manager.get_interaction_inventory_widgets()) == panels
                layer.toggle_inventory_layout()
                assert layer.get_editor_property("bInventoryOpen")
                assert [snapshot(panel) for panel in panels] == before
                # Close through the actual title button delegate chain.
                manager.open_secondary_inventory(external, u.InteractableType.CONTAINER)
                title = u.InvenzaTakeAmountValidationLibrary.find_preview_widget(panels[0], "TitleBar")
                close = u.InvenzaTakeAmountValidationLibrary.find_preview_widget(title, "Button_Close")
                button = u.InvenzaTakeAmountValidationLibrary.find_preview_widget(close, "MainButton")
                assert EDIT.press_preview_button(button)
                assert [snapshot(panel) for panel in panels] == before
                # Also start with a closed ordinary inventory and preserve its closed state.
                layer.toggle_inventory_layout()
                closed = [snapshot(panel) for panel in panels]
                manager.open_secondary_inventory(external, u.InteractableType.CONTAINER)
                layer.close_dual_inventory_view()
                assert not layer.get_editor_property("bInventoryOpen")
                assert [snapshot(panel) for panel in panels] == closed
                layer.toggle_inventory_layout()
                manager.open_secondary_inventory(external, u.InteractableType.CONTAINER)
                world = pawn.get_outer().get_outer()
                # No demonstration pickups or startup overflow should remain for this 35-weight starter set.
                ground = [p for p in u.ObjectIterator(u.PickupComponent) if p.get_owner() and p.get_owner().get_owner() == pawn]
                assert not ground, "Unexpected starting pickups"
                if "AUTHORITY" in str(pawn.get_local_role()):
                    handle = u.DataTableRowHandle(data_table=u.load_asset(ROOT + "Data/DT_ItemData"), row_name="BackpackSmall")
                    actor = u.InvenzayUtility.drop_item(world, pawn, handle, 1,
                        pawn.get_actor_location() + u.Vector(120, 0, 0), u.Rotator())
                    assert actor, "Drop must find a floor"
                    drops.append((actor, actor.get_actor_location().z))
                    nowhere = u.InvenzayUtility.drop_item(world, pawn, handle, 1,
                        u.Vector(1000000, 1000000, 1000000), u.Rotator())
                    assert nowhere is None, "Unsupported drop must fail instead of losing the item"
                records.append(dict(layer=layer, manager=manager, panels=panels, external=external,
                                    closed=closed, before=before))
            ready_at = now + 4
            phase = 2
            return
        if phase == 2 and now >= ready_at:
            for index, record in enumerate(records):
                layer = record["layer"]
                dual = layer.get_editor_property("DualInventoryWidget")
                assert EDIT.render_widget(dual, u.Vector2D(1100, 750), str(OUT / ("interaction-%d.png" % index)))
                assert EDIT.render_widget(record["panels"][0], u.Vector2D(550, 650), str(OUT / ("backpack-%d.png" % index)))
                # Verify the other consumer of the same list: the shared storage workspace.
                workspace_class = u.load_asset(ROOT + "UI/Inventory/Workspace/WBP_InventoryWorkspace").generated_class()
                workspace = EDIT.create_preview(layer.get_owning_player_pawn().get_outer().get_outer(), workspace_class)
                workspace.set_player_widgets(record["panels"])
                assert u.InvenzaTakeAmountValidationLibrary.find_preview_widget(workspace, "PlayerInventoryList").get_children_count() == 2
                EDIT.release_preview()
                # Restore via the menu's retained placements, even after another temporary parent.
                layer.close_dual_inventory_view()
                assert [snapshot(panel) for panel in record["panels"]] == record["before"]
            for actor, initial_z in drops:
                assert actor.get_actor_location().z > initial_z - 10, "Pickup fell through the floor"
                mesh = actor.get_component_by_class(u.StaticMeshComponent)
                assert abs(mesh.get_physics_linear_velocity().z) < 10, "Pickup has not settled"
            (OUT / "pie-verification.json").write_text(json.dumps(dict(players=len(records),
                player_storages_per_player=2, repeated_open_close_cycles=4, reused_panels=True,
                placements_restored=True, open_and_closed_state_restored=True, workspace_multiple_storages=True,
                switch_container_and_toggle_close=True, title_button_close=True,
                physical_drops_settled=len(drops), no_startup_pickups=True, no_floor_drop_rejected=True,
                no_assets_saved=True, persistence_disabled=True), indent=2), encoding="utf-8")
            u.log("INVENZA_INTERACTION_PIE_VERIFIED")
            u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
            done = True
    except Exception:
        (OUT / "pie-error.txt").write_text(traceback.format_exc(), encoding="utf-8")
        u.log_error(traceback.format_exc())
        u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
        done = True


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_pre_tick_callback(tick)
