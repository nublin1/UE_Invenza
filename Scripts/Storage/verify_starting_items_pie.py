# Nublin Studio 2026 All Rights Reserved.
"""Transient listen-server/client UI check. Persistence is disabled in this process; no assets are saved."""
from pathlib import Path
import json
import time
import traceback
import unreal as u

OUT = Path(u.Paths.project_saved_dir()) / "StartingItems20261028"
settings = u.get_default_object(u.InvenzaSaveSettings)
settings.set_editor_property("bEnableInPIE", False)
settings.set_editor_property("bAutoLoad", False)
settings.set_editor_property("bAutoSave", False)
settings.set_editor_property("bSaveOnExit", False)
assert u.get_editor_subsystem(u.LevelEditorSubsystem).load_level("/InventorySystemInvenzaPlugin/Maps/Map_Ex")
assert u.InvenzaTakeAmountValidationLibrary.start_network_preview()
deadline = time.monotonic() + 25
callback = None
done = False
last_check = 0
report = []
warm_until = None
u.log("INVENZA_STARTING_ITEMS_PIE_PROBE_SCHEDULED")


def tick(delta):
    global done, last_check, warm_until
    if done:
        u.unregister_slate_post_tick_callback(callback)
        u.SystemLibrary.quit_editor()
        return
    if time.monotonic() - last_check < 0.3:
        return
    last_check = time.monotonic()
    try:
        layers = [obj for obj in u.ObjectIterator(u.GameMenuLayerInv)
                  if obj.get_owning_player_pawn() and "UEDPIE_" in obj.get_owning_player_pawn().get_path_name()]
        ready = []
        observations = []
        for layer in layers:
            pawn = layer.get_owning_player_pawn()
            manager = pawn.get_component_by_class(u.IInventoryManager)
            bag = manager.get_editor_property("MainPawnInventoryRef") if manager else None
            state = {"layer": layer.get_path_name(), "pawn": pawn.get_path_name(),
                     "main_inventory": bag.get_path_name() if bag else None}
            observations.append(state)
            if not bag:
                continue
            containers = list(layer.get_all_pawn_inv_containers())
            bag_panels = [container for container in containers
                          if container.get_inventory_widget_from_container_slot().get_editor_property("InventoryRef") == bag]
            gear_panels = [container for container in containers
                           if "WBP_EqGrid" in container.get_inventory_widget_from_container_slot().get_class().get_name()]
            state["containers"] = [{"class": c.get_inventory_widget_from_container_slot().get_class().get_name(),
                "inventory": str(c.get_inventory_widget_from_container_slot().get_editor_property("InventoryRef"))} for c in containers]
            if len(bag_panels) != 1 or len(gear_panels) != 1:
                continue
            gear_widget = gear_panels[0].get_inventory_widget_from_container_slot()
            bag_widget = bag_panels[0].get_inventory_widget_from_container_slot()
            gear_icons = u.InvenzaTakeAmountValidationLibrary.find_preview_widget(gear_widget, "ItemsVisualsPanel").get_children_count()
            bag_icons = u.InvenzaTakeAmountValidationLibrary.find_preview_widget(bag_widget, "ItemsVisualsPanel").get_children_count()
            state["gear_icons"] = gear_icons
            state["bag_icons"] = bag_icons
            state["collection"] = pawn.get_component_by_class(u.InventoryCollectionComponent).get_editor_property("InventoryArray").export_text()
            if gear_icons < 1 or bag_icons < 1:
                continue
            ready.append((layer, bag_panels[0], gear_panels[0], gear_icons, bag_icons))
        if len(ready) < 2:
            (OUT / "pie-observations.json").write_text(json.dumps(observations, indent=2), encoding="utf-8")
            if time.monotonic() > deadline:
                raise RuntimeError("Timed out waiting for both listen-server/client backpack panels: " + str(len(ready)))
            return
        if warm_until is None:
            for layer, bag_panel, gear_panel, gear_icons, bag_icons in ready:
                if not layer.get_editor_property("bInventoryOpen"):
                    layer.toggle_inventory_layout()
            warm_until = time.monotonic() + 3
            return
        if time.monotonic() < warm_until:
            return
        for index, (layer, bag_panel, gear_panel, gear_icons, bag_icons) in enumerate(ready):
            if not layer.get_editor_property("bInventoryOpen"):
                layer.toggle_inventory_layout()
            assert bag_panel.get_editor_property("visibility") == u.SlateVisibility.VISIBLE
            assert u.InvenzaWidgetEditingLibrary.render_widget(gear_panel, u.Vector2D(500, 650), str(OUT / ("pie-equipment-%d.png" % index)))
            assert u.InvenzaWidgetEditingLibrary.render_widget(bag_panel, u.Vector2D(700, 650), str(OUT / ("pie-backpack-%d.png" % index)))
            report.append({"layer": layer.get_path_name(), "backpack_panel_visible": True,
                           "equipment_icons": gear_icons, "backpack_icons": bag_icons})
            layer.open_dual_inventory_view(gear_panel, bag_panel)
            layer.close_dual_inventory_view()
            assert not layer.get_editor_property("bInventoryOpen")
            assert bag_panel.get_editor_property("visibility") == u.SlateVisibility.COLLAPSED
            report[-1]["dual_view_close_respects_closed_inventory"] = True
        (OUT / "pie-ui-verification.json").write_text(json.dumps({"players": report,
            "persistence_disabled": True, "no_assets_saved": True}, indent=2), encoding="utf-8")
        u.log("INVENZA_STARTING_ITEMS_PIE_UI_VERIFIED")
        u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
        done = True
    except Exception:
        u.log_error(traceback.format_exc())
        u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
        done = True


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_pre_tick_callback(tick)
