# Nublin Studio 2026 All Rights Reserved.
"""Add a station mode button to the two crafting Widget Blueprints only.

Existing buttons, visibility, opacity and mouse handling are preserved.
Run after compiling the native crafting mode interface.
"""
import hashlib
import json
from pathlib import Path
import shutil
import unreal as u

ROOT = '/InventorySystemInvenzaPlugin/'
PROJECT = Path(u.Paths.project_dir())
CONTENT = PROJECT / 'Plugins/InventorySystemInvenzaPlugin/Content'
OUT = PROJECT / 'Saved/CraftMode'
EDIT = u.InvenzaWidgetEditingLibrary
PANEL = 'UI/Craft/WBP_DashboardControlPanel'
DASHBOARD = 'UI/Craft/WBP_CraftDashboard'
TEXTS = 'Data/Localization/ST_InvenzaRuntimeTexts'
SETTINGS = 'Data/Settings/InvenzaSettings'


def backup(relative):
    source = CONTENT / (relative + '.uasset')
    target = OUT / 'Before' / (relative + '.uasset')
    if not target.exists():
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)


def widget(bp, name):
    return u.find_object(u.find_object(bp, 'WidgetTree'), name)


def input_settings(bp):
    tree = u.find_object(bp, 'WidgetTree')
    return {w.get_name(): [str(w.get_visibility()), w.get_render_opacity()]
            for w in u.ObjectIterator(u.Widget) if w.get_outer() == tree}


def save(bp):
    assert EDIT.compile_widget(bp), bp.get_name()
    assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)


def run():
    OUT.mkdir(parents=True, exist_ok=True)
    baseline_path = OUT / 'asset-hashes-before.json'
    if not baseline_path.exists():
        hashes = {str(p.relative_to(CONTENT)): hashlib.sha256(p.read_bytes()).hexdigest()
                  for p in CONTENT.rglob('*') if p.suffix in ['.uasset', '.umap']}
        baseline_path.write_text(json.dumps(hashes, indent=2), encoding='utf-8')
    for relative in [PANEL, DASHBOARD, TEXTS, SETTINGS]:
        backup(relative)
    strings = u.load_asset(ROOT + TEXTS)
    captions = {
        'InvenzaCraft.AutomaticMode': 'Mode: Automatic',
        'InvenzaCraft.OperatorMode': 'Mode: Operator required',
        'InvenzaCraft.ModeTooltip': 'Switch between automatic production and production requiring an operator.',
    }
    for key, text in captions.items():
        EDIT.set_string_table_entry(strings, key, text)
    assert u.EditorAssetLibrary.save_loaded_asset(strings, only_if_is_dirty=False)

    # Approved station configuration: the operator tag and its one block reason only.
    settings = u.load_asset(ROOT + SETTINGS)
    operator_tag = u.GameplayTag()
    assert operator_tag.import_text('(TagName="CraftSystem.Blocks.NoOperator")')
    settings.set_editor_property('block_no_operator', operator_tag)
    reasons = list(settings.get_editor_property('available_block_reasons'))
    if not any(reason.tag.export_text() == operator_tag.export_text() for reason in reasons):
        reasons.append(u.BlockReasonData(tag=operator_tag,
            message=EDIT.string_table_text(strings, 'InvenzaCraft.NoOperator')))
    settings.set_editor_property('available_block_reasons', reasons)
    assert u.EditorAssetLibrary.save_loaded_asset(settings, only_if_is_dirty=False)

    panel = u.load_asset(ROOT + PANEL)
    before = input_settings(panel)
    holder = widget(panel, 'BtnsHolder')
    assert isinstance(holder, u.HorizontalBox)
    pause = widget(panel, 'Btn_Pause')
    reference_frame = widget(panel, 'Btn_PauseFrame')
    body = EDIT.add_template(panel, u.VerticalBox, 'CraftControlsBody')
    if holder.get_parent() != body:
        assert holder.get_parent() is None, 'Unexpected panel structure'
        EDIT.set_root(panel, body)
        body.add_child_to_vertical_box(holder)
    button = EDIT.add_template(panel, pause.get_class(), 'Btn_AutomaticCrafting')
    button.set_editor_properties(dict(
        default_text=EDIT.string_table_text(strings, 'InvenzaCraft.AutomaticMode'),
        is_toggle_button=False))
    button.set_tool_tip_text(EDIT.string_table_text(strings, 'InvenzaCraft.ModeTooltip'))
    size = EDIT.add_template(panel, u.SizeBox, 'AutomaticCraftingButtonSize')
    size.set_min_desired_height(42)
    if size.get_child_at(0) != button:
        size.add_child(button)
    button.slot.set_padding(u.Margin(10, 4, 10, 4))
    frame = EDIT.add_template(panel, reference_frame.get_class(), 'AutomaticCraftingButtonFrame')
    for side in ['top', 'bottom', 'left', 'right']:
        name = side + '_border_style'
        frame.set_editor_property(name, reference_frame.get_editor_property(name))
    EDIT.set_slot_content(frame, 'NamedSlot', size)
    if frame.get_parent() != body:
        body.add_child_to_vertical_box(frame)
    frame.slot.set_padding(u.Margin(0, 6, 0, 2))
    after = input_settings(panel)
    assert all(after[name] == state for name, state in before.items()), 'Input settings changed'
    save(panel)

    dashboard = u.load_asset(ROOT + DASHBOARD)
    before_dashboard = input_settings(dashboard)
    assert EDIT.compile_widget(dashboard)
    defaults = u.get_default_object(dashboard.generated_class())
    defaults.set_editor_properties(dict(
        automatic_crafting_button_text=EDIT.string_table_text(strings, 'InvenzaCraft.AutomaticMode'),
        operator_crafting_button_text=EDIT.string_table_text(strings, 'InvenzaCraft.OperatorMode')))
    dashboard.modify()
    assert u.EditorAssetLibrary.save_loaded_asset(dashboard, only_if_is_dirty=False)
    assert input_settings(dashboard) == before_dashboard

    baseline = json.loads(baseline_path.read_text(encoding='utf-8'))
    changed = [Path(name).as_posix() for name, digest in baseline.items()
               if hashlib.sha256((CONTENT / name).read_bytes()).hexdigest() != digest]
    assert set(changed) <= {p + '.uasset' for p in [PANEL, DASHBOARD, TEXTS, SETTINGS]}, changed
    (OUT / 'asset-changes.json').write_text(json.dumps(dict(
        changed=changed, input_settings_preserved=True), indent=2), encoding='utf-8')
    u.log('CRAFT_MODE_BUTTON_ASSETS_OK')


try:
    run()
finally:
    u.SystemLibrary.quit_editor()
