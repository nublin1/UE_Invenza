# Nublin Studio 2026 All Rights Reserved.
"""Exercise actual hold timers and client RPC pickup delivery; save no assets or player data."""
from pathlib import Path
import json
import time
import traceback
import unreal as u

OUT = Path(u.Paths.project_saved_dir()) / "MultiPickup20261028"
EDIT = u.InvenzaWidgetEditingLibrary
settings = u.get_default_object(u.InvenzaSaveSettings)
for key in ["bEnableInPIE", "bAutoLoad", "bAutoSave", "bSaveOnExit"]:
    settings.set_editor_property(key, False)
assert u.get_editor_subsystem(u.LevelEditorSubsystem).load_level("/InventorySystemInvenzaPlugin/Maps/Map_Ex")
assert u.InvenzaTakeAmountValidationLibrary.start_network_preview()
cls = u.load_asset("/InventorySystemInvenzaPlugin/Blueprints/Examples/Pickups/BP_ScrapBox").generated_class()
HEADLESS = "-NullRHI" in u.SystemLibrary.get_command_line()
phase = 0
started = time.monotonic()
phase_at = started
last_check = 0
callback = None
done = False
report = {"save_disabled": True, "assets_saved": False}
source = None
client_source = None
client_pawn = None
server_pawn = None
server_world = None
server_manager = None
bag = None
before_bag = None
before_ground = None
capture = None
target = None


def spawn(world, actor_class, location, rotation=None):
    transform = u.Transform(location=location, rotation=rotation or u.Rotator())
    actor = EDIT.spawn_preview_actor(world, actor_class, transform)
    assert actor
    return actor


def totals(items):
    result = {}
    for item in items:
        name = str(item.get_editor_property("ItemID"))
        result[name] = result.get(name, 0) + item.get_editor_property("Quantity")
    return result


def bag_totals():
    collection = bag.get_editor_property("ItemCollectionLinked")
    return {str(entry.item_id): entry.amount for entry in collection.collect_items_aggregated(bag.get_editor_property("InventoryContainerID"))}


def ground_totals():
    items = []
    prefix = server_world.get_path_name() + ":"
    for pickup in u.ObjectIterator(u.PickupComponent):
        owner = pickup.get_owner()
        if owner and owner.get_path_name().startswith(prefix) and owner.get_owner() == server_pawn:
            items.extend(pickup.get_pickup_items())
    return totals(items)


def change_phase(value):
    global phase, phase_at
    phase = value
    phase_at = time.monotonic()


def tick(delta):
    global done, last_check, source, client_source, client_pawn, server_pawn, server_world
    global server_manager, bag, before_bag, before_ground, capture, target
    if done:
        u.unregister_slate_pre_tick_callback(callback)
        u.SystemLibrary.quit_editor()
        return
    now = time.monotonic()
    if now - last_check < 0.1:
        return
    last_check = now
    try:
        (OUT / "pie-progress.json").write_text(json.dumps(dict(phase=phase, elapsed=now-started)), encoding="utf-8")
        if now - phase_at > 35:
            raise RuntimeError("Multi-pickup PIE timeout, phase " + str(phase))
        if phase == 0:
            layers = [layer for layer in u.ObjectIterator(u.GameMenuLayerInv)
                      if layer.get_owning_player_pawn() and "UEDPIE_" in layer.get_owning_player_pawn().get_path_name()]
            host = next((layer.get_owning_player_pawn() for layer in layers if layer.get_owning_player_pawn().has_authority()), None)
            client_pawn = next((layer.get_owning_player_pawn() for layer in layers if not layer.get_owning_player_pawn().has_authority()), None)
            if not host or not client_pawn or now - started < 6:
                return
            server_world = host.get_outer().get_outer()
            server_pawn = next((pawn for pawn in u.GameplayStatics.get_all_actors_of_class(server_world, u.Pawn)
                                if pawn.is_player_controlled() and not pawn.is_locally_controlled()), None)
            if not server_pawn:
                return
            server_manager = server_pawn.get_component_by_class(u.IInventoryManager)
            bag = server_manager.get_editor_property("MainPawnInventoryRef")
            if not bag or abs(bag.get_editor_property("InventoryTotalWeight") - 35) > 0.01:
                return
            config = bag.get_editor_property("InventorySettings").copy()
            config.set_editor_property("InventoryMaxWeightCapacity", 36.0)
            bag.set_inventory_settings(config)
            before_bag = bag_totals()
            before_ground = ground_totals()
            source = spawn(server_world, cls, server_pawn.get_actor_location() + u.Vector(100, 0, 0))
            report["source"] = source.get_path_name()
            change_phase(1)
        elif phase == 1:
            client_prefix = client_pawn.get_outer().get_outer().get_path_name() + ":"
            actors = [actor for actor in u.ObjectIterator(u.Actor) if actor.get_class() == cls and actor.get_path_name().startswith(client_prefix)]
            (OUT / "pie-replication.json").write_text(json.dumps(dict(
                source_replicates=source.get_editor_property("replicates"),
                server_items=totals(source.get_component_by_class(u.PickupComponent).get_pickup_items()),
                matching_clients=[a.get_path_name() for a in actors],
                all_instances=[a.get_path_name() for a in u.ObjectIterator(u.Actor) if a.get_class() == cls],
                client_items=[totals(a.get_component_by_class(u.PickupComponent).get_pickup_items()) for a in actors]), indent=2), encoding="utf-8")
            if not actors:
                return
            client_source = actors[0]
            pickup = client_source.get_component_by_class(u.PickupComponent)
            if totals(pickup.get_pickup_items()) != {"Board": 2, "Rope": 1, "IronIngot": 1}:
                return
            assert pickup.is_bundle_pickup()
            assert totals(pickup.get_pickup_items()) == {"Board": 2, "Rope": 1, "IronIngot": 1}
            assert str(pickup.get_interaction_action_text(u.InteractionType.PRIMARY, False)) == "Разобрать"
            server_pickup = source.get_component_by_class(u.PickupComponent)
            assert source.get_component_by_class(u.StaticMeshComponent).static_mesh == server_pickup.get_editor_property("CustomMesh")
            report["all_bundle_items_replicated"] = True
            if not HEADLESS:
                position = source.get_actor_location() + u.Vector(80, -100, 85)
                camera = spawn(server_world, u.SceneCapture2D, position,
                               u.MathLibrary.find_look_at_rotation(position, source.get_actor_location() + u.Vector(0, 0, 14)))
                capture = camera.get_component_by_class(u.SceneCaptureComponent2D)
                target = u.RenderingLibrary.create_render_target2d(server_world, 800, 600, u.TextureRenderTargetFormat.RTF_RGBA8)
                capture.set_editor_properties(dict(texture_target=target, capture_source=u.SceneCaptureSource.SCS_FINAL_COLOR_LDR,
                    capture_every_frame=False, capture_on_movement=False, fov_angle=40.0))
                capture.capture_scene()
            interaction = client_pawn.get_component_by_class(u.InteractionComponent)
            widget = next((w for w in u.ObjectIterator(u.InteractionWidget) if w.get_owning_player_pawn() == client_pawn), None)
            assert widget
            assert EDIT.preview_pickup_input(client_pawn, pickup, True)
            change_phase(2)
        elif phase == 2 and now - phase_at > 0.5:
            assert bag_totals() == before_bag, "Resources were granted before hold completion"
            assert source and client_source
            pickup = client_source.get_component_by_class(u.PickupComponent)
            widget = next(w for w in u.ObjectIterator(u.InteractionWidget) if w.get_owning_player_pawn() == client_pawn)
            action = widget.get_editor_property("FirstRow").get_editor_property("ActionText").get_text()
            assert "Разбираем" in str(action), str(action)
            progress = widget.get_editor_property("InteractionProgressBar").get_editor_property("percent")
            assert 0 < progress < 1, progress
            if not HEADLESS:
                assert EDIT.render_widget(widget, u.Vector2D(600, 180), str(OUT / "dismantle-hold.png"))
                capture.capture_scene()
                u.RenderingLibrary.export_render_target(server_world, target, str(OUT), "scrap-box.png")
            assert EDIT.preview_pickup_input(client_pawn, pickup, False)
            report["hold_progress"] = progress
            report["active_action"] = str(action)
            change_phase(3)
        elif phase == 3 and now - phase_at > 2.5:
            assert source and client_source
            assert bag_totals() == before_bag, "Cancelled hold granted resources"
            report["early_release_cancels_without_grant"] = True
            assert EDIT.preview_pickup_input(client_pawn, client_source.get_component_by_class(u.PickupComponent), True)
            change_phase(4)
        elif phase == 4 and now - phase_at > 3.0:
            server_left = [actor for actor in u.GameplayStatics.get_all_actors_of_class(server_world, cls)]
            client_left = list(u.GameplayStatics.get_all_actors_of_class(client_pawn.get_outer().get_outer(), cls))
            if server_left or client_left:
                return
            after_bag = bag_totals()
            after_ground = ground_totals()
            (OUT / "pie-totals.json").write_text(json.dumps(dict(before_bag=before_bag, after_bag=after_bag,
                before_ground=before_ground, after_ground=after_ground), indent=2), encoding="utf-8")
            deltas = {}
            for name, expected in {"Board": 2, "Rope": 1, "IronIngot": 1}.items():
                stored = after_bag.get(name, 0) - before_bag.get(name, 0)
                dropped = after_ground.get(name, 0) - before_ground.get(name, 0)
                assert stored + dropped == expected, (name, stored, dropped, expected)
                deltas[name] = dict(stored=stored, dropped=dropped)
            assert any(value["stored"] for value in deltas.values()), deltas
            assert any(value["dropped"] for value in deltas.values()), deltas
            assert bag.get_editor_property("InventoryTotalWeight") <= 36.001
            report.update(client_rpc_grants_once=True, source_destroyed_on_server_and_client=True,
                          partial_capacity_results=deltas, final_bag_weight=bag.get_editor_property("InventoryTotalWeight"))
            (OUT / "pie-verification.json").write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
            u.log("INVENZA_MULTI_PICKUP_PIE_VERIFIED")
            EDIT.release_preview()
            u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
            done = True
    except Exception:
        u.log_error(traceback.format_exc())
        EDIT.release_preview()
        u.get_editor_subsystem(u.LevelEditorSubsystem).editor_request_end_play()
        done = True


u.EditorPythonScripting.set_keep_python_script_alive(True)
callback = u.register_slate_pre_tick_callback(tick)
