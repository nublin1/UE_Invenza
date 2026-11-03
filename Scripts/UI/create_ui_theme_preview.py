# Nublin Studio 2026 All Rights Reserved.
"""Author the theme sample in Widget Blueprint using the project's Core elements."""
import unreal as u

PATH = '/InventorySystemInvenzaPlugin/UI/Theme'
NAME = 'WBP_UIThemePreview'
EDIT = u.InvenzaWidgetEditingLibrary
if u.EditorAssetLibrary.does_asset_exist(PATH + '/' + NAME):
    u.log('UI_THEME_PREVIEW_ALREADY_EXISTS')
else:
    factory = u.WidgetBlueprintFactory()
    factory.set_editor_property('parent_class', u.InvenzaBaseWidget)
    bp = u.AssetToolsHelpers.get_asset_tools().create_asset(NAME, PATH, u.WidgetBlueprint, factory)
    assert bp
    root = EDIT.add_template(bp, u.Border, 'ThemeSurface')
    root.set_brush_color(u.LinearColor(.024, .016, .009, 1))
    root.set_padding(u.Margin(24))
    EDIT.set_root(bp, root)
    body = EDIT.add_template(bp, u.VerticalBox, 'ThemeSamples')
    root.add_child(body)
    core = '/InventorySystemInvenzaPlugin/UI/Core/'
    label_class = u.load_class(None, core + 'WBP_LabelBaseText.WBP_LabelBaseText_C')
    button_class = u.load_class(None, core + 'Buttons/WBP_UIButton.WBP_UIButton_C')
    input_class = u.load_class(None, core + 'WBP_EditableLabelBaseText.WBP_EditableLabelBaseText_C')
    progress_class = u.load_class(None, core + 'Progress/WBP_CraftProgressDisplay.WBP_CraftProgressDisplay_C')
    font = u.load_asset('/Engine/EngineFonts/Roboto.Roboto')

    def label(parent, name, text, size=16, color=(.82, .65, .36), bold=False):
        widget = EDIT.add_template(bp, label_class, name)
        widget.set_editor_property('text', u.Text(text))
        widget.set_editor_property('font_info', u.SlateFontInfo(font_object=font, size=size,
                                   typeface_font_name='Bold' if bold else 'Regular'))
        widget.set_editor_property('color', u.SlateColor(specified_color=u.LinearColor(*color, 1)))
        slot = parent.add_child(widget)
        if hasattr(slot, 'set_padding'):
            slot.set_padding(u.Margin(0, 0, 0, 12))
        return widget

    label(body, 'ThemeHeading', 'UI theme / Тема интерфейса', 24, bold=True)
    label(body, 'ThemeDescription', 'Colors, fonts, frames and interaction states', 16)
    inset = EDIT.add_template(bp, u.Border, 'InsetSurface')
    inset.set_brush_color(u.LinearColor(.035, .024, .013, 1))
    inset.set_padding(u.Margin(16))
    body.add_child(inset)
    samples = EDIT.add_template(bp, u.VerticalBox, 'InsetSamples')
    inset.add_child(samples)
    label(samples, 'PrimaryText', 'Primary text / Основная надпись', 18, bold=True)
    label(samples, 'SecondaryText', 'Длинная русская надпись: оформление задаётся в редакторе', 15, (.48, .38, .25))
    field = EDIT.add_template(bp, input_class, 'SampleInput')
    field.set_editor_property('hint_text', u.Text('Search / Поиск'))
    field.set_editor_property('font_info', u.SlateFontInfo(font_object=font, size=16, typeface_font_name='Regular'))
    samples.add_child(field).set_padding(u.Margin(0, 0, 0, 18))
    label(samples, 'ButtonCaption', 'Normal · Hover · Press · Toggle · Disabled', 15)
    row = EDIT.add_template(bp, u.HorizontalBox, 'ButtonSamples')
    samples.add_child(row).set_padding(u.Margin(0, 0, 0, 18))
    for name, text in [('SampleButton', 'OK'), ('SampleToggle', 'Toggle'), ('SampleDisabled', 'Disabled')]:
        button = EDIT.add_template(bp, button_class, name)
        button.set_editor_property('default_text', u.Text(text))
        button.set_editor_property('is_toggle_button', name == 'SampleToggle')
        if name == 'SampleDisabled':
            button.set_is_enabled(False)
        row.add_child(button).set_padding(u.Margin(0, 0, 18, 0))
    label(samples, 'ProgressCaption', 'Craft progress / Прогресс крафта', 15)
    progress = EDIT.add_template(bp, progress_class, 'SampleProgress')
    settings = progress.get_editor_property('display_settings')
    settings.set_editor_property('use_progress_bar', True)
    settings.set_editor_property('show_percentage', True)
    progress.set_editor_property('display_settings', settings)
    samples.add_child(progress).set_padding(u.Margin(0, 0, 0, 18))
    label(samples, 'PreviewNote', 'Preview only. Inventory geometry, icons and input behavior keep their own settings.', 13, (.48, .38, .25))
    assert EDIT.compile_widget(bp)
    assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
    u.log('UI_THEME_PREVIEW_CREATED')
