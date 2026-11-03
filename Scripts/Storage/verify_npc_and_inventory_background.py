# Nublin Studio 2026 All Rights Reserved.
"""Read-only listen-server/client probe of real HUD geometry and map examples."""
from pathlib import Path
import json
import time
import traceback
import unreal as u

OUT = Path(u.Paths.project_saved_dir()) / "NewGameNPC20261028"
EDIT = u.InvenzaWidgetEditingLibrary
FIND = u.InvenzaTakeAmountValidationLibrary.find_preview_widget
settings = u.get_default_object(u.InvenzaSaveSettings)
for name in ["bEnableInPIE", "bAutoLoad", "bAutoSave", "bSaveOnExit"]:
    settings.set_editor_property(name, False)
assert u.get_editor_subsystem(u.LevelEditorSubsystem).load_level("/InventorySystemInvenzaPlugin/Maps/Map_Ex")
assert u.InvenzaTakeAmountValidationLibrary.start_network_preview()
callback = None
done = False
opened = None
deadline = time.monotonic() + 40


def tick(delta):
    global done, opened
    if done:
        u.unregister_slate_pre_tick_callback(callback)
        u.SystemLibrary.quit_editor()
        return
    try:
        observations = []
        for actor in u.ObjectIterator(u.Actor):
            if "UEDPIE_" not in actor.get_path_name():
                continue
            manager = actor.get_component_by_class(u.IInventoryManager)
            if not manager:
                continue
            collection = actor.get_component_by_class(u.InventoryCollectionComponent)
            access = actor.get_component_by_class(u.CharacterInventoryAccessComponent)
            observations.append(dict(actor=actor.get_path_name(), main=str(manager.get_editor_property("MainPawnInventoryRef")),
                access_main=str(access.get_editor_property("MainLootInventory")) if access else None,
                inventories=[dict(id=str(inv.get_editor_property("InventoryContainerID")), weight=inv.get_editor_property("InventoryTotalWeight"),
                    type=str(inv.get_editor_property("InventorySettings").inventory_type)) for inv in collection.get_editor_property("ActorInventories") if inv]))
        (OUT / "network-observations.json").write_text(json.dumps(observations, indent=2), encoding="utf-8")
        if time.monotonic() > deadline:
            raise RuntimeError("Timed out waiting for replicated NPC/vendor/player inventories")
        layers = [layer for layer in u.ObjectIterator(u.GameMenuLayerInv)
                  if layer.get_owning_player_pawn() and "UEDPIE_" in layer.get_owning_player_pawn().get_path_name()]
        ready = []
        for layer in layers:
            pawn = layer.get_owning_player_pawn()
            world_prefix = pawn.get_outer().get_outer().get_path_name() + ":"
            manager = pawn.get_component_by_class(u.IInventoryManager)
            bag = manager.get_editor_property("MainPawnInventoryRef")
            if not bag:
                return
            panels = [c for c in layer.get_all_pawn_inv_containers()
                      if c.get_inventory_widget_from_container_slot().get_editor_property("InventoryRef") == bag]
            if not panels:
                return
            npcs = [actor for actor in u.ObjectIterator(u.InventoryMannequin) if actor.get_path_name().startswith(world_prefix)]
            if len(npcs) != 1:
                return
            npc = npcs[0]
            access = npc.get_component_by_class(u.CharacterInventoryAccessComponent)
            npc_bag = access.get_editor_property("MainLootInventory")
            if not npc_bag or not access.get_equipment_inventory():
                return
            npc_manager = npc.get_component_by_class(u.IInventoryManager)
            assert npc_manager.get_editor_property("MainPawnInventoryRef") == npc_bag
            npc_weight = npc_bag.get_editor_property("InventoryTotalWeight")
            if abs(npc_weight - 5) > 0.01:
                return
            vendors = [component for component in u.ObjectIterator(u.VendorComponent)
                       if component.get_owner() and component.get_owner().get_path_name().startswith(world_prefix)]
            if len(vendors) != 1:
                return
            vendor_actor = vendors[0].get_owner()
            vendor_manager = vendor_actor.get_component_by_class(u.IInventoryManager)
            stock = [inventory for inventory in vendor_actor.get_component_by_class(u.InventoryCollectionComponent).get_editor_property("ActorInventories")
                     if inventory and inventory.get_editor_property("InventorySettings").inventory_type == u.InventoryType.VENDOR_INVENTORY]
            if not stock or abs(stock[0].get_editor_property("InventoryTotalWeight") - 7) > 0.01:
                return
            assert vendor_manager.get_editor_property("MainPawnInventoryRef") is None
            ready.append((layer, bag, panels[0], npc_bag, stock[0]))
        if len(ready) != 2:
            return
        if opened is None:
            for layer, bag, panel, npc_bag, stock in ready:
                layer.toggle_inventory_layout()
            opened = time.monotonic()
            return
        if time.monotonic() - opened < 2:
            return
        report = []
        for index, (layer, bag, panel, npc_bag, stock) in enumerate(ready):
            border = FIND(panel, "Border_27")
            rect = EDIT.get_preview_widget_rect(border, panel)
            desired = panel.get_desired_size()
            assert rect.z - rect.x >= desired.x - 1
            assert rect.w - rect.y >= desired.y - 1
            assert panel.slot.get_auto_size()
            assert bag.get_editor_property("InventorySettings").inventory_max_weight_capacity == 50
            root_rect = EDIT.get_preview_widget_rect(layer, layer)
            viewport_size = u.Vector2D(root_rect.z - root_rect.x, root_rect.w - root_rect.y)
            assert EDIT.render_widget(layer, viewport_size, str(OUT / ("inventory-hud-%d.png" % index)))
            EDIT.release_preview()
            assert EDIT.render_widget(panel, desired, str(OUT / ("inventory-panel-%d.png" % index)))
            EDIT.release_preview()
            report.append(dict(layer=layer.get_path_name(), background_size=[rect.z - rect.x, rect.w - rect.y],
                               content_size=[desired.x, desired.y], npc_bag_weight=5, permanent_vendor_stock_weight=7))
        (OUT / "network-verification.json").write_text(json.dumps(dict(players=report, no_assets_saved=True,
            saves_disabled=True), indent=2), encoding="utf-8")
        u.log("INVENZA_NPC_VENDOR_BACKGROUND_VERIFIED")
        u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
        done = True
    except Exception:
        (OUT / "network-error.txt").write_text(traceback.format_exc(), encoding="utf-8")
        EDIT.release_preview()
        u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
        done = True


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_pre_tick_callback(tick)
