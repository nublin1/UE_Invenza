# Nublin Studio 2026 All Rights Reserved.
"""Explicit, scoped migration of audited UI assets. Run with Unreal Editor Python.

Authoring-time color matching creates saved semantic bindings. Runtime never guesses
roles from colors. This script does not change visibility, alpha, layout or content.
"""
from pathlib import Path
import json
import re
import unreal as u

PROJECT = Path(u.Paths.project_dir())
OUT = PROJECT / 'Saved/UITheme20261031'
THEME_PATH = '/InventorySystemInvenzaPlugin/Data/UI/DA_UITheme_Default'
PREVIEW_PATH = '/InventorySystemInvenzaPlugin/UI/Theme/WBP_UIThemePreview'
EDITOR = u.InvenzaUIThemeEditingLibrary
EDIT = u.InvenzaWidgetEditingLibrary
SOURCE = u.InvenzaUIThemeSource.THEME

# These names group existing values without changing the first theme's appearance.
KNOWN_COLORS = [
    ((.82, .65, .36), 'Text.Primary'),
    ((.48, .38, .25), 'Text.Secondary'),
    ((.024, .016, .009), 'Panel.Background'),
    ((.035, .024, .013), 'Panel.Inset'),
    ((.065, .045, .025), 'Panel.Row'),
    ((.20, .14, .075), 'Frame.Standard'),
    ((.24, .17, .08), 'Panel.Selected'),
    ((.18, .12, .055), 'Panel.Hovered'),
    ((.65, .43, .12), 'Progress.Fill'),
    ((.018, .012, .007), 'Progress.Track'),
    ((.028, .019, .011), 'Panel.World'),
    ((.32, .23, .11), 'Frame.World'),
    ((.495514, .495514, .495514), 'Button.Normal'),
    ((.724303, .724303, .724303), 'Button.Hovered'),
    ((.384314, .384314, .384314), 'Button.Pressed'),
    ((.527115, .527115, .527115), 'Text.Disabled'),
    ((.745404, .552011, .254152), 'Text.Inventory'),
    ((.243137, .113725, .007843), 'Frame.Cell'),
    ((1., .651406, .141263), 'Button.Selected'),
    ((.020289, .012286, .006512), 'Panel.Inventory'),
    ((.16, .11, .05), 'Panel.Header'),
    ((.11, .08, .045), 'Panel.Alternate'),
]
palette, typography, brushes = {}, {}, {}
color_values, font_values, brush_values = {}, {}, {}
excluded = []
current_package = ''

# Replace historical palette labels with purpose-based names, retaining exact RGB.
ROLE_RENAMES = dict(zip(
    ['Color.Accent' + str(i) for i in [6,7,8,10,14,18,19,22,24,25,27,28,29,30,31,32,33,34,35,36,37,38,39,42,48,51,52,54,55,56,57,60,62,63,64,66,67,69,70,72,74,75]],
    ['Button.Filled.Normal','Button.Filled.Hovered','Button.Filled.Pressed','Text.ButtonForeground','Text.Selection',
     'Slot.Active','Input.Selection.Blue','Input.Selection.DarkBlue','Text.ButtonHovered','Button.Background','Frame.Inventory',
     'Text.Modal','Text.ModalSecondary','Slider.Track','Slider.Handle','Button.ModalPrimary.Normal','Button.ModalPrimary.Hovered',
     'Button.ModalPrimary.Pressed','Text.ButtonModal','Button.ModalSecondary.Normal','Button.ModalSecondary.Hovered',
     'Button.ModalSecondary.Pressed','Button.ModalSecondary.Disabled','Progress.Timer','Status.Confirmed','Text.Weight','Drop.Trash',
     'Text.Error','Text.Queue','Panel.Recipe.Hovered','Panel.Recipe.Selected','Text.Placeholder','Slot.EquipmentBorder',
     'Slot.InventoryBorder','Text.SlotName','Icon.Money','Progress.Interaction','Frame.InventorySeparator','Text.ItemQuantity',
     'Debug.Active','Debug.Game','Debug.Modal']))
KNOWN_COLORS += [
    ((0.119999997, 0.079999998, 0.039999999), 'Button.Filled.Normal'),
    ((0.219999999, 0.150000006, 0.070000000), 'Button.Filled.Hovered'),
    ((0.300000012, 0.200000003, 0.079999998), 'Button.Filled.Pressed'),
    ((0.000000000, 1.000000000, 1.000000000), 'Text.ButtonForeground'),
    ((0.020000000, 0.300000012, 0.000000000), 'Text.Selection'),
    ((0.700999975, 0.224999994, 0.003000000), 'Slot.Active'),
    ((0.009134059, 0.351532608, 0.964686215), 'Input.Selection.Blue'),
    ((0.000000000, 0.187821001, 0.679542005), 'Input.Selection.DarkBlue'),
    ((0.876235008, 1.000000000, 0.000000000), 'Text.ButtonHovered'),
    ((0.861957014, 0.027970999, 1.000000000), 'Button.Background'),
    ((0.242281005, 0.104616001, 0.006995000), 'Frame.Inventory'),
    ((0.854992628, 0.879622400, 0.913098633), 'Text.Modal'),
    ((0.423267663, 0.485149950, 0.577580452), 'Text.ModalSecondary'),
    ((0.061246052, 0.084376208, 0.124771819), 'Slider.Track'),
    ((0.246201321, 0.479320168, 0.708375752), 'Slider.Handle'),
    ((0.054480277, 0.147027269, 0.242281124), 'Button.ModalPrimary.Normal'),
    ((0.080219820, 0.212230757, 0.346704066), 'Button.ModalPrimary.Hovered'),
    ((0.034339808, 0.093058966, 0.155926466), 'Button.ModalPrimary.Pressed'),
    ((0.896269381, 0.921581864, 0.955973327), 'Text.ButtonModal'),
    ((0.029556835, 0.040915199, 0.057805430), 'Button.ModalSecondary.Normal'),
    ((0.046665087, 0.068478167, 0.097587347), 'Button.ModalSecondary.Hovered'),
    ((0.018500220, 0.026241222, 0.038204372), 'Button.ModalSecondary.Pressed'),
    ((0.014443844, 0.018500220, 0.026241222), 'Button.ModalSecondary.Disabled'),
    ((0.015084000, 1.000000000, 0.000000000), 'Progress.Timer'),
    ((0.273995996, 0.724268019, 0.283699989), 'Status.Confirmed'),
    ((0.723055005, 0.520995975, 0.230739996), 'Text.Weight'),
    ((1.000000000, 0.239298001, 0.259195000), 'Drop.Trash'),
    ((0.639999986, 0.230000004, 0.140000001), 'Text.Error'),
    ((0.552012026, 0.417885005, 0.234550998), 'Text.Queue'),
    ((0.100000001, 0.072999999, 0.035000000), 'Panel.Recipe.Hovered'),
    ((0.180000007, 0.119999997, 0.050000001), 'Panel.Recipe.Selected'),
    ((0.379999995, 0.289999992, 0.180000007), 'Text.Placeholder'),
    ((0.155926004, 0.078186996, 0.000000000), 'Slot.EquipmentBorder'),
    ((0.156250000, 0.078369997, 0.000000000), 'Slot.InventoryBorder'),
    ((0.871366978, 0.679542005, 0.346704006), 'Text.SlotName'),
    ((0.920975029, 1.000000000, 0.475739986), 'Icon.Money'),
    ((0.000000000, 0.500000000, 1.000000000), 'Progress.Interaction'),
    ((0.730461001, 0.539479017, 0.262250990), 'Frame.InventorySeparator'),
    ((0.679542005, 0.520995975, 0.274677008), 'Text.ItemQuantity'),
    ((1.000000000, 0.819999993, 0.200000003), 'Debug.Active'),
    ((1.000000000, 0.610495985, 0.000000000), 'Debug.Game'),
    ((0.000000000, 0.552012026, 1.000000000), 'Debug.Modal'),
]


def unique_name(stem, values, value):
    name, index = stem, 2
    while name in values and values[name]['value'] != value:
        name, index = stem + '.Variant' + str(index), index + 1
    return name


def color_role(style, obj):
    rgb = tuple(style['rgba'][:3])
    key = tuple(round(x, 6) for x in rgb)
    path = style['path']
    if path.endswith('NotAllowedColor'):
        name = 'Drop.Invalid'
    elif path.endswith('AllowedColor'):
        name = 'Drop.Valid'
    elif path.endswith('PartialColor'):
        name = 'Drop.Partial'
    elif key in color_values:
        return color_values[key]
    else:
        name = next((role for color, role in KNOWN_COLORS
                     if max(abs(a-b) for a, b in zip(rgb, color)) < .00003), None)
        if name is None:
            if max(rgb) - min(rgb) < .00001:
                name = ('Color.White' if rgb[0] > .99999 else 'Color.Black' if rgb[0] < .00001
                        else 'Color.Gray' + str(round(rgb[0]*1000)))
            elif key == (0., 1., 0.):
                name = 'Drop.Valid'
            elif key == (1., 0., 0.):
                name = 'Drop.Invalid'
            elif key == (1., 0., 1.):
                name = 'Color.LegacyMagenta'
            else:
                # Keep legacy accents distinct; their role is explicit after migration.
                name = 'Color.Accent' + str(len(color_values) + 1)
    value = '(R=%0.9f,G=%0.9f,B=%0.9f,A=1.000000000)' % rgb
    name = unique_name(name, palette, value)
    palette[name] = {'type': 'color', 'value': value}
    color_values.setdefault(key, name)
    return name


def font_role(style):
    value = style['value']
    native_roles = {'RowStyle.FontInfo': ('Controls.Row', 15), 'HeadingStyle.FontInfo': ('Controls.Heading', 17),
                    'TextStyle.FontInfo': ('Craft.Progress', 13), 'RestockLabelStyle.FontInfo': ('Operations.Caption', 18)}
    alias = native_roles.get(style['path'])
    if alias and style.get('size') == alias[1]:
        typography[alias[0]] = {'type': 'font', 'value': value, 'use_default_font': True}
        return alias[0]
    if value in font_values:
        return font_values[value]
    face = style['face'] if style['face'] != 'None' else 'Default'
    font = style['font'].split('/')[-1].split('.')[0]
    size = str(style['size']).replace('.0', '').replace('.', '_')
    stem = ('Text.' if font == 'Roboto' else 'Legacy.' + font + '.') + face + size
    role = unique_name(stem, typography, value)
    typography[role] = {'type': 'font', 'value': value, 'use_default_font': font == 'Roboto'}
    font_values[value] = role
    return role


def brush_role(style, obj):
    value = style['value']
    if value in brush_values:
        return brush_values[value]
    leaf = style['path'].split('.')[-1]
    resource = style.get('resource', 'None').split('/')[-1].split('.')[0]
    stem = 'Brush.' + (resource if resource != 'None' else leaf)
    role = unique_name(stem, brushes, value)
    brushes[role] = {'type': 'brush', 'value': value, 'color_role': color_role(style, obj)}
    brush_values[value] = role
    return role


def binding_for(style, obj):
    path, cls = style['path'], obj['class']
    if obj['themed'] and path in ('ColorAndOpacity', 'ForegroundColor'):
        return None, 'UserWidget identity multiplier / inherited foreground'
    if style['type'] == 'brush':
        # Dynamic icon, capture, material and content brushes are never theme assets.
        if 'Image' in cls and not any(word in obj['name'].lower() for word in ('surface', 'backdrop', 'background')):
            return None, 'Content/action image resource stays local'
        if path == 'BrushStyle.Brush' and not any(word in obj['name'].lower() for word in ('surface', 'backdrop', 'background')):
            return None, 'Content image resource stays local'
        return brush_role(style, obj), None
    if style['type'] == 'font':
        return font_role(style), None
    return color_role(style, obj), None


def collect_object(obj, target_name):
    result = []
    for style in obj['styles']:
        role, reason = binding_for(style, obj)
        if reason:
            excluded.append({'object': obj['name'], 'path': style['path'], 'reason': reason})
            continue
        result.append({'widget_name': target_name, 'property_path': style['path'], 'role': role})
        if style['type'] in ('font', 'brush'):
            definitions = typography if style['type'] == 'font' else brushes
            definitions[role].setdefault('source', {'package': current_package,
                    'widget': '' if obj['name'].startswith('Default__') else obj['name'], 'property': style['path']})
    return result


def export_binding(value):
    return u.InvenzaUIThemeBinding(widget_name=value['widget_name'],
                                 property_path=value['property_path'], role=value['role'], override_locally=False)


def merge_bindings(widget, bindings):
    # Embedded widgets retain bindings to their own internal UMG components.
    result = {(str(b.widget_name).lower(), str(b.property_path).lower()): b for b in widget.get_editor_property('theme_bindings')}
    for binding in result.values():
        if str(binding.role) in ROLE_RENAMES:
            binding.set_editor_property('role', ROLE_RENAMES[str(binding.role)])
    for entry in bindings:
        result[((entry['widget_name'] or 'None').lower(), entry['property_path'].lower())] = export_binding(entry)
    return list(result.values())


def run():
    global current_package
    before = json.loads((OUT / 'audit-before.json').read_text(encoding='utf-8'))
    valid = [a for a in before if 'defaults' in a and a['package'] != PREVIEW_PATH]
    preview = u.load_asset(PREVIEW_PATH)
    assert preview, 'Create the authorized preview Blueprint before migrating'
    record = json.loads(EDITOR.inspect_blueprint(preview))
    record['package'] = PREVIEW_PATH
    valid.append(record)
    assert not any(a.get('animation_count', 0) or a.get('delegate_binding_count', 0) or a['pins'] for a in valid), 'Review live style bindings before authoring'
    # Process inherited layer Blueprints after WBP_Layer; all others have native parents.
    valid.sort(key=lambda a: (a['parent'].startswith('/InventorySystemInvenzaPlugin/'), a['package']))
    scope = []
    for asset in valid:
        current_package = asset['package']
        root = collect_object(asset['defaults'], '')
        children = {}
        for obj in asset['widgets']:
            if obj['themed']:
                children[obj['name']] = collect_object(obj, '')
            else:
                root += collect_object(obj, obj['name'])
        scope.append({'package': asset['package'], 'bindings': root, 'children': children})
    # Stable aliases replace the four old Project Settings typography defaults.
    aliases = {'Controls.Row': ('RowStyle.FontInfo', 15), 'Controls.Heading': ('HeadingStyle.FontInfo', 17),
               'Craft.Progress': ('TextStyle.FontInfo', 13), 'Operations.Caption': ('RestockLabelStyle.FontInfo', 18)}
    for role, (path, size) in aliases.items():
        original = next(s for a in valid for o in [a['defaults']] + a['widgets'] for s in o['styles']
                        if s['path'] == path and s.get('size') == size)
        typography.setdefault(role, {'type': 'font', 'value': original['value'], 'use_default_font': True})
    theme = u.load_asset(THEME_PATH)
    if not theme:
        factory = u.DataAssetFactory()
        factory.set_editor_property('data_asset_class', u.InvenzaUITheme)
        theme = u.AssetToolsHelpers.get_asset_tools().create_asset('DA_UITheme_Default',
                    THEME_PATH.rsplit('/', 1)[0], u.InvenzaUITheme, factory)
    assert theme
    theme.modify()
    for field in ('palette', 'typography', 'brushes'):
        theme.set_editor_property(field, {})
    theme.set_editor_property('default_font', u.load_asset('/Engine/EngineFonts/Roboto.Roboto'))
    for styles in (palette, typography, brushes):
        for role, value in sorted(styles.items()):
            if 'source' in value:
                source = value['source']
                owner = u.load_asset(source['package'])
                obj = (u.find_object(u.find_object(owner, 'WidgetTree'), source['widget']) if source['widget']
                       else u.get_default_object(owner.generated_class()))
                assert EDITOR.copy_theme_style(theme, role, obj, source['property'],
                            value.get('color_role', ''), value.get('use_default_font', False)), role
            else:
                assert EDITOR.set_theme_style(theme, role, value['type'], value['value'],
                            value.get('color_role', ''), value.get('use_default_font', False)), role
    assert u.EditorAssetLibrary.save_loaded_asset(theme, only_if_is_dirty=False)
    for entry in scope:
        if '-ThemeStylesOnly' in u.SystemLibrary.get_command_line():
            continue
        bp = u.load_asset(entry['package'])
        cdo = u.get_default_object(bp.generated_class())
        assert EDITOR.configure_widget(cdo, merge_bindings(cdo, entry['bindings']), SOURCE)
        tree = u.find_object(bp, 'WidgetTree')
        for name, bindings in entry['children'].items():
            widget = u.find_object(tree, name)
            assert widget, entry['package'] + ':' + name
            assert EDITOR.configure_widget(widget, merge_bindings(widget, bindings), SOURCE)
        bp.modify()
        if hasattr(EDITOR, 'compile_theme_blueprint'):
            assert EDITOR.compile_theme_blueprint(bp), entry['package']
        else:
            u.BlueprintEditorLibrary.compile_blueprint(bp)
        assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False), entry['package']
        u.log('UI_THEME_MIGRATED ' + entry['package'])
    manifest = {'theme': THEME_PATH, 'assets': scope, 'palette': palette,
                'typography': typography, 'brushes': brushes, 'excluded': excluded}
    (OUT / 'migration.json').write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
    u.log('UI_THEME_MIGRATION_COMPLETE ' + str(len(scope)))


run()
