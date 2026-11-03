# Nublin Studio 2026 All Rights Reserved.
"""Offscreen before/after images; only transient instances are changed."""
from pathlib import Path
import unreal as u

OUT = Path(u.Paths.project_saved_dir()) / 'UITheme20261031'
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
EDIT = u.InvenzaWidgetEditingLibrary
cases = [('/InventorySystemInvenzaPlugin/UI/Theme/WBP_UIThemePreview', (900, 550)),
         ('/InventorySystemInvenzaPlugin/UI/Craft/WBP_CraftMenuChoose', (1000, 600))]
settings = u.get_default_object(u.InvenzaInventoryDeveloperSettings)
selected = u.InvenzaUIThemeLibrary.get_selected_theme()
try:
    # Bypass the theme on transient instances for an authored-style reference.
    for stage in ('local-reference', 'after'):
        settings.set_editor_property('ui_theme', None if stage == 'local-reference' else selected)
        for path, size in cases:
            if not u.EditorAssetLibrary.does_asset_exist(path):
                continue
            cls = u.load_class(None, path + '.' + path.rsplit('/', 1)[-1] + '_C')
            widget = EDIT.create_preview(world, cls)
            for _ in range(3):
                assert EDIT.render_widget(widget, u.Vector2D(*size), str(OUT / (stage + '-' + cls.get_name() + '.png')))
            if path.endswith('WBP_UIThemePreview'):
                progress = u.InvenzaTakeAmountValidationLibrary.find_preview_widget(widget, 'SampleProgress')
                progress.set_progress(3.5, 5.)
                toggle = u.InvenzaTakeAmountValidationLibrary.find_preview_widget(widget, 'SampleToggle')
                toggle.set_toggle_status(True)
                for _ in range(3):
                    assert EDIT.render_widget(widget, u.Vector2D(*size), str(OUT / (stage + '-states.png')))
            EDIT.release_preview()
        u.log('UI_THEME_RENDER_COMPLETE ' + stage)
finally:
    settings.set_editor_property('ui_theme', selected)
    EDIT.release_preview()
u.SystemLibrary.quit_editor()
