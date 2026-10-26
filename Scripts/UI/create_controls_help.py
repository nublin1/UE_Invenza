"""Create the controls help widget and its F6 mapping. Run with Unreal Python."""
from pathlib import Path
import shutil
import unreal as u

source = (Path(u.Paths.project_dir())/'Scripts/UI/restyle_craft_choose.py').read_text(encoding='utf-8')
exec(source.split('# A reusable wrapping label')[0])
BUTTON = u.load_class(None, ROOT+'UI/Core/Buttons/WBP_UIButtonWithSlot.WBP_UIButtonWithSlot_C')
path = 'UI/Controls/WBP_ControlsHelp'
if u.EditorAssetLibrary.does_asset_exist(ROOT+path):
    bp = asset(path)
else:
    factory = u.WidgetBlueprintFactory()
    factory.set_editor_property('parent_class', u.ControlsHelpWidget)
    bp = u.AssetToolsHelpers.get_asset_tools().create_asset('WBP_ControlsHelp', ROOT+'UI/Controls', None, factory)
body = panel(bp, u.VerticalBox, 'HelpBody')
attach(body, label(bp, 'Title', 'Controls', 22, True), margin(0,0,0,8))
scroll = panel(bp, u.ScrollBox, 'ControlsList')
scroll.set_visibility(VIS.VISIBLE)
attach(body, scroll, margin(0,0,0,14), True)
button = make(bp, BUTTON, 'OKButton')
button.set_visibility(VIS.VISIBLE)
EDIT.set_slot_content(button, 'NamedSlot_21', size(bp, 'OKContent', label(bp, 'OKLabel', 'OK', 17, True), 100, 32))
footer = panel(bp, u.HorizontalBox, 'HelpFooter')
attach(footer, button).set_horizontal_alignment(H.H_ALIGN_CENTER)
attach(body, footer).set_horizontal_alignment(H.H_ALIGN_CENTER)
root = size(bp, 'HelpSize', frame(bp, 'HelpFrame', surface(bp, 'HelpBackground', body, BG, 18)), 660, 440)
EDIT.set_root(bp, root)
assert EDIT.compile_widget(bp)
cdo = u.get_default_object(bp.generated_class())
cdo.set_editor_property('label_class', LABEL)
cdo.set_editor_property('is_focusable', True)
save(bp)

action_path = ROOT+'Input/UIActions/IA_ControlsHelp'
action = u.load_asset(action_path)
if not action:
    action = u.AssetToolsHelpers.get_asset_tools().create_asset('IA_ControlsHelp',ROOT+'Input/UIActions',u.InputAction,u.InputAction_Factory())
action.set_editor_property('action_description',u.Text('Controls help'))
action.set_editor_property('value_type',u.InputActionValueType.BOOLEAN)
assert u.EditorAssetLibrary.save_loaded_asset(action,only_if_is_dirty=False)

def preserve(relative):
    src = PROJECT/'Plugins/InventorySystemInvenzaPlugin/Content'/(relative+'.uasset')
    dst = PROJECT/'Saved/ControlsHelp/Before'/(relative+'.uasset')
    if src.exists() and not dst.exists():
        dst.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(src,dst)

key = u.Key()
key.set_editor_property('key_name','F6')
preserve('Input/InputMappingInventoryContext')
context = u.load_asset(ROOT+'Input/InputMappingInventoryContext')
assert not any(m.key == key and m.action != action for m in context.get_editor_property('mappings')), 'F6 is already assigned'
context.unmap_all_keys_from_action(action)
context.map_key(action,key)
assert u.EditorAssetLibrary.save_loaded_asset(context,only_if_is_dirty=False)

preserve('Data/Settings/InvenzaSettings')
settings = u.load_asset(ROOT+'Data/Settings/InvenzaSettings')
settings.set_editor_property('controls_help_widget_class',bp.generated_class())
settings.set_editor_property('controls_help_action',action)
entries = list(settings.get_editor_property('additional_controls'))
for group, relative, caption in [
    ('General','Actions/IA_Move','Move'),('General','Actions/IA_Look','Look around'),
    ('General','Actions/IA_Jump','Jump'),('Inventory','UIActions/IA_TakeAll','Take all items'),
    ('Inventory','UIActions/IA_PlaceAll','Place all items')]:
    control = u.load_asset(ROOT+'Input/'+relative)
    if control and not any(e.action == control for e in entries):
        entries.append(u.ControlsHelpEntry(group=u.Text(group),label=u.Text(caption),action=control))
settings.set_editor_property('additional_controls',entries)
assert u.EditorAssetLibrary.save_loaded_asset(settings,only_if_is_dirty=False)
for mapping in context.get_editor_property('mappings'):
    u.log('HELP_MAPPING '+mapping.action.get_name()+': '+str(mapping.key))
u.log('CONTROLS_HELP_ASSETS_OK')
