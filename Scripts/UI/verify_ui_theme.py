# Nublin Studio 2026 All Rights Reserved.
"""Audit migrated assets and exercise live style/content separation. Never saves assets."""
from pathlib import Path
import json
import unreal as u

OUT = Path(u.Paths.project_saved_dir()) / 'UITheme20261031'
EDIT = u.InvenzaWidgetEditingLibrary
AUTHOR = u.InvenzaUIThemeEditingLibrary
before = json.loads((OUT / 'audit-before.json').read_text(encoding='utf-8'))
manifest = json.loads((OUT / 'migration.json').read_text(encoding='utf-8'))
checks, after, failures = [], [], []


def check(condition, text):
    if not condition:
        failures.append(text)
    checks.append({'check': text, 'passed': bool(condition)})


def rgb(value):
    return (value.r, value.g, value.b)


def close(a, b):
    return max(abs(x-y) for x, y in zip(a, b)) < .00003


theme = u.InvenzaUIThemeLibrary.get_selected_theme()
check(bool(theme), 'Theme is available without a GameInstance')
for old in before:
    if 'defaults' not in old:
        continue
    bp = u.load_asset(old['package'])
    current = json.loads(AUTHOR.inspect_blueprint(bp))
    current['package'] = old['package']
    after.append(current)
    cdo = u.get_default_object(bp.generated_class())
    check(cdo.get_editor_property('theme_source') == u.InvenzaUIThemeSource.THEME, old['package'] + ': theme enabled')
    check(current['parent'] == old['parent'], old['package'] + ': parent unchanged')
    check(current.get('animation_count', 0) == 0 and current.get('delegate_binding_count', 0) == 0,
          old['package'] + ': no color animation/property binding bypass')
    original_objects = [old['defaults']] + old['widgets']
    current_objects = {o['name']: o for o in [current['defaults']] + current['widgets']}
    check(len(current_objects) == len(original_objects), old['package'] + ': widget structure unchanged')
    for original in original_objects:
        updated = current_objects.get(original['name'])
        check(updated is not None, old['package'] + ':' + original['name'] + ': retained')
        if updated is None:
            continue
        for field in ('protected', 'parent', 'slot'):
            check(updated.get(field) == original.get(field), old['package'] + ':' + original['name'] + ': ' + field)
        styles = {s['path']: s for s in updated['styles']}
        for style in original['styles']:
            check(style['path'] in styles and style['value'] == styles[style['path']]['value'],
                  old['package'] + ':' + original['name'] + ':' + style['path'] + ': authored style unchanged')

palette = {str(k): v for k, v in theme.get_editor_property('palette').items()}
fonts = {str(k): v for k, v in theme.get_editor_property('typography').items()}
brushes = {str(k): v for k, v in theme.get_editor_property('brushes').items()}
for entry in manifest['assets']:
    for group in [entry['bindings']] + list(entry['children'].values()):
        for binding in group:
            check(binding['role'] in palette or binding['role'] in fonts or binding['role'] in brushes,
                  entry['package'] + ':' + binding['property_path'] + ': role exists')

world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
preview_class = u.load_class(None, '/InventorySystemInvenzaPlugin/UI/Theme/WBP_UIThemePreview.WBP_UIThemePreview_C')
widget = EDIT.create_preview(world, preview_class)
title = u.InvenzaTakeAmountValidationLibrary.find_preview_widget(widget, 'PrimaryText')
input_widget = u.InvenzaTakeAmountValidationLibrary.find_preview_widget(widget, 'SampleInput')
toggle = u.InvenzaTakeAmountValidationLibrary.find_preview_widget(widget, 'SampleToggle')
for sample in (widget, title, input_widget, toggle):
    sample.apply_ui_theme()

title.update_text(u.Text('Current quantity / Количество: 27'))
original_text = str(title.main_text_block.get_text())
original_alpha = title.get_editor_property('color').specified_color.a
input_widget.update_text(u.Text('Unsaved input / Введённый текст'))
input_widget.update_hint_text(u.Text('Preserved hint'))
title_role = next(str(b.role) for b in title.get_editor_property('theme_bindings') if b.property_path == 'Color')
changed_palette = dict(palette)
changed_palette[title_role] = u.LinearColor(.25, .5, .75, .123)
changed_palette['Button.Selected'] = u.LinearColor(.2, .7, .4, .1)
try:
    theme.set_editor_property('palette', changed_palette)
    title.apply_ui_theme()
    input_widget.apply_ui_theme()
    check(close(rgb(title.main_text_block.get_editor_property('color_and_opacity').specified_color), (.25, .5, .75)), 'Palette edits reach rendered label properties')
    check(title.get_editor_property('color').specified_color.a == original_alpha, 'Palette does not change label alpha')
    check(str(title.main_text_block.get_text()) == original_text, 'Applying theme preserves label text')
    title.update_text(u.Text('Updated quantity: 12'))
    check(close(rgb(title.main_text_block.get_editor_property('color_and_opacity').specified_color), (.25, .5, .75)), 'Updating content does not revert the theme')
    check(str(input_widget.main_editable_text_box.get_text()) == 'Unsaved input / Введённый текст', 'Applying theme preserves user input')
    check(str(input_widget.main_editable_text_box.get_editor_property('hint_text')) == 'Preserved hint', 'Applying theme preserves hint')

    normal = toggle.main_button.get_editor_property('background_color')
    toggle.set_toggle_status(True)
    on_alpha = toggle.main_button.get_editor_property('background_color').a
    toggle.apply_ui_theme()
    check(toggle.get_toggle_status(), 'Theme preserves toggle state')
    check(close(rgb(toggle.main_button.get_editor_property('background_color')), (.2, .7, .4)), 'Active toggle uses the selected-state role')
    check(toggle.main_button.get_editor_property('background_color').a == on_alpha, 'Active toggle alpha is preserved')
    toggle.set_toggle_status(False)
    check(close(rgb(toggle.main_button.get_editor_property('background_color')), rgb(normal)) and toggle.main_button.get_editor_property('background_color').a == normal.a,
          'Toggle off restores the themed normal background')

    bindings = list(title.get_editor_property('theme_bindings'))
    for binding in bindings:
        if binding.property_path == 'Color':
            binding.set_editor_property('override_locally', True)
    title.set_editor_property('theme_bindings', bindings)
    title.set_editor_property('theme_source', u.InvenzaUIThemeSource.THEME_WITH_OVERRIDES)
    local_color = title.get_editor_property('color')
    changed_palette[title_role] = u.LinearColor(.9, .2, .1, .2)
    theme.set_editor_property('palette', changed_palette)
    title.apply_ui_theme()
    check(close(rgb(title.get_editor_property('color').specified_color), rgb(local_color.specified_color)), 'Explicit local color override wins')
    title.set_editor_property('theme_source', u.InvenzaUIThemeSource.LOCAL)
    title.apply_ui_theme()
    check(close(rgb(title.get_editor_property('color').specified_color), rgb(local_color.specified_color)), 'Local mode leaves authored appearance intact')

    inherited_class = u.load_class(None, '/InventorySystemInvenzaPlugin/UI/Core/WBP_LabelBaseText.WBP_LabelBaseText_C')
    inherited = EDIT.create_preview(world, inherited_class)
    inherited_style = u.LabelStyle(text=u.Text('Owner supplied style'),
            font_info=u.SlateFontInfo(font_object=theme.get_editor_property('default_font'), size=12, typeface_font_name='Regular'),
            color=u.SlateColor(specified_color=u.LinearColor(.3, .4, .5, 1)))
    inherited.update_text_and_font(inherited_style)
    inherited.apply_ui_theme()
    check(inherited.get_editor_property('font_info').size == 12 and
          close(rgb(inherited.main_text_block.get_editor_property('color_and_opacity').specified_color), (.3, .4, .5)),
          'An explicit owner style survives subsequent theme synchronization')
    check(inherited.get_editor_property('theme_source') == u.InvenzaUIThemeSource.THEME_WITH_OVERRIDES,
          'Explicit appearance setters expose their override mode')

    button_class = u.load_class(None, '/InventorySystemInvenzaPlugin/UI/Core/Buttons/WBP_UIButton.WBP_UIButton_C')
    early_toggle = EDIT.create_preview(world, button_class)
    early_normal = early_toggle.main_button.get_editor_property('background_color')
    early_toggle.set_toggle_status(True)
    early_toggle.apply_ui_theme()
    early_toggle.set_toggle_status(False)
    check(early_toggle.main_button.get_editor_property('background_color').a == early_normal.a,
          'A toggle selected before construction retains its original normal alpha')

    image_class = u.load_class(None, '/InventorySystemInvenzaPlugin/UI/Core/WBP_BaseImage.WBP_BaseImage_C')
    image = EDIT.create_preview(world, image_class)
    icon = u.load_asset('/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture')
    image.update_image(icon)
    resource = image.main_image.get_editor_property('brush').get_editor_property('resource_object')
    image.apply_ui_theme()
    check(image.main_image.get_editor_property('brush').get_editor_property('resource_object') == resource and resource == icon, 'Theme preserves assigned content icon')
finally:
    theme.set_editor_property('palette', palette)
    EDIT.release_preview()

(OUT / 'audit-after.json').write_text(json.dumps(after, indent=2, ensure_ascii=False), encoding='utf-8')
(OUT / 'verification.json').write_text(json.dumps({'checks': checks, 'failures': failures, 'assets_saved': False}, indent=2, ensure_ascii=False), encoding='utf-8')
assert not failures, '\n'.join(failures[:30])
u.log('UI_THEME_VERIFIED ' + str(len(checks)))
