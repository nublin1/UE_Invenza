# Nublin Studio 2026 All Rights Reserved.
"""Regressions for authoring/resolved styles, Designer edits and invalid bindings. No asset saves."""
from pathlib import Path
import json
import unreal as u

OUT=Path(u.Paths.project_saved_dir())/'UIThemeFollowup20261101'
OUT.mkdir(parents=True,exist_ok=True)
E=u.InvenzaWidgetEditingLibrary
A=u.InvenzaUIThemeEditingLibrary
world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
theme=u.InvenzaUIThemeLibrary.get_selected_theme()
colors={str(k):v for k,v in theme.get_editor_property('palette').items()}
checks=[]

def check(value, name):
    checks.append({'name':name,'passed':bool(value)})
    assert value,name

def color_rgb(value):
    return [round(value.r,5),round(value.g,5),round(value.b,5)]

def create(name):
    path='/InventorySystemInvenzaPlugin/UI/Core/'+name
    w=E.create_preview(world,u.load_class(None,path+'.'+name.rsplit('/',1)[-1]+'_C'))
    w.set_editor_property('theme_source',u.InvenzaUIThemeSource.THEME)
    return w

def color_role(w, widget, path):
    return next(str(b.role) for b in w.get_editor_property('theme_bindings') if str(b.widget_name)==widget and b.property_path==path)

try:
    label=create('WBP_LabelBaseText')
    authored=u.LinearColor(.11,.22,.33,1)
    label.set_editor_property('color',u.SlateColor(specified_color=authored))
    label.update_text(u.Text('Content survives / Текст сохраняется'))
    font=label.get_editor_property('font_info');font.set_editor_property('size',17)
    label.set_editor_property('font_info',font)
    role=color_role(label,'None','Color')
    changed=dict(colors);changed[role]=u.LinearColor(.7,.4,.2,1)
    theme.set_editor_property('palette',changed)
    label.apply_ui_theme()
    check(color_rgb(label.get_editor_property('color').specified_color)==color_rgb(authored),'Theme leaves authored color intact')
    check(color_rgb(label.main_text_block.get_editor_property('color_and_opacity').specified_color)==[.7,.4,.2],'Theme resolves displayed color')
    label.set_editor_property('theme_source',u.InvenzaUIThemeSource.LOCAL);label.apply_ui_theme()
    check(color_rgb(label.main_text_block.get_editor_property('color_and_opacity').specified_color)==color_rgb(authored),'Theme to Local restores authored color')
    check(label.main_text_block.get_editor_property('font').size==17,'Local restores authored font')
    label.set_editor_property('theme_source',u.InvenzaUIThemeSource.THEME);label.apply_ui_theme()
    label.update_appearance(font,u.SlateColor(specified_color=u.LinearColor(.2,.3,.4,1)))
    label.apply_ui_theme()
    check(color_rgb(label.main_text_block.get_editor_property('color_and_opacity').specified_color)==[.2,.3,.4],'Explicit appearance update becomes an actual local override')
    check(str(label.main_text_block.get_text())=='Content survives / Текст сохраняется','Style transitions preserve content')
    label.set_theme_property_override('Color',False);label.apply_ui_theme()
    check(color_rgb(label.main_text_block.get_editor_property('color_and_opacity').specified_color)==[.7,.4,.2],'Disabling override returns to the theme')

    button=create('Buttons/WBP_UIButton')
    A.make_designer_preview(button)
    normal=button.main_button.get_editor_property('background_color')
    normal_role=color_role(button,'MainButton','BackgroundColor')
    changed[normal_role]=u.LinearColor(.14,.25,.36,1)
    theme.set_editor_property('palette',changed)
    button.apply_ui_theme()
    local=u.InvenzaUIThemeValue(type=u.InvenzaUIThemeValueType.COLOR,color=u.LinearColor(.16,.27,.38,normal.a))
    check(A.edit_preview_style(button,'MainButton','BackgroundColor',local),'Edit raw UMG style through an editor property event')
    button.set_editor_property('theme_source',u.InvenzaUIThemeSource.LOCAL);button.apply_ui_theme()
    check(color_rgb(button.main_button.get_editor_property('background_color'))==[.16,.27,.38],'Designer edits made while themed survive Local restoration')
    button.set_editor_property('theme_source',u.InvenzaUIThemeSource.THEME);button.apply_ui_theme()
    button.set_toggle_status(True)
    del changed[normal_role]
    theme.set_editor_property('palette',changed);button.apply_ui_theme()
    check(button.get_toggle_status(),'Missing-role fallback preserves active toggle state')
    button.set_toggle_status(False)
    check(color_rgb(button.main_button.get_editor_property('background_color'))==[.16,.27,.38],'Missing normal role restores edited authored background, not selected color')
    check(button.main_button.get_editor_property('background_color').a==normal.a,'Toggle fallback preserves original alpha')

    compound=create('Buttons/WBP_UIButton')
    A.make_designer_preview(compound)
    raw=compound.main_button.get_editor_property('widget_style').normal
    original_tint=color_rgb(raw.tint_color.specified_color)
    compound_role=color_role(compound,'MainButton','WidgetStyle.Normal')
    tint_role=str(theme.get_editor_property('brushes')[compound_role].color_role)
    changed[tint_role]=u.LinearColor(.37,.48,.59,1)
    theme.set_editor_property('palette',changed);compound.apply_ui_theme()
    edited=compound.main_button.get_editor_property('widget_style').normal
    margin=edited.margin;margin.set_editor_property('left',.42);edited.set_editor_property('margin',margin)
    A.edit_preview_style(compound,'MainButton','WidgetStyle.Normal',u.InvenzaUIThemeValue(type=u.InvenzaUIThemeValueType.BRUSH,brush=edited))
    compound.set_editor_property('theme_source',u.InvenzaUIThemeSource.LOCAL);compound.apply_ui_theme()
    restored=compound.main_button.get_editor_property('widget_style').normal
    check(abs(restored.margin.left-.42)<.00001,'Nested Designer brush edit restores its new local margin')
    check(color_rgb(restored.tint_color.specified_color)==original_tint,'Editing margin does not capture the themed tint as authored data')

    # Typed diagnostics, independent of the result being visibly different.
    owner=create('Buttons/WBP_UIButton')
    brush_role=str(next(iter(theme.get_editor_property('brushes').keys())))
    bindings=[
      u.InvenzaUIThemeBinding(widget_name='AbsentWidget',property_path='BrushColor',role='Text.Primary'),
      u.InvenzaUIThemeBinding(widget_name='MainButton',property_path='AbsentProperty',role='Text.Primary'),
      u.InvenzaUIThemeBinding(widget_name='MainButton',property_path='RenderTransform',role='Text.Primary'),
      u.InvenzaUIThemeBinding(widget_name='MainLabel',property_path='Font',role='Text.Primary'),
      u.InvenzaUIThemeBinding(widget_name='MainLabel',property_path='ShadowColorAndOpacity',role='AbsentRole'),
      u.InvenzaUIThemeBinding(widget_name='MainButton',property_path='BackgroundColor',role='Text.Primary'),
      u.InvenzaUIThemeBinding(widget_name='MainButton',property_path='BackgroundColor',role='Text.Primary'),
      u.InvenzaUIThemeBinding(widget_name='MainButton',property_path='WidgetStyle.Normal',role=brush_role),
      u.InvenzaUIThemeBinding(widget_name='MainButton',property_path='WidgetStyle.Normal.TintColor',role='Text.Primary')]
    owner.set_editor_property('theme_bindings',bindings)
    results=A.validate_theme_widget(owner,None)
    statuses={r.status.name for r in results}
    for expected in ['MISSING_WIDGET','MISSING_PROPERTY','UNSUPPORTED_PROPERTY','WRONG_ROLE_TYPE','MISSING_ROLE','DUPLICATE','OVERLAP']:
        check(expected in statuses,'Diagnostic detects '+expected)
    check(all(r.diagnostic and r.binding.property_path for r in results),'Diagnostics identify the actual binding path')
    # Repeated passes must not turn a resolved value into a new authored baseline.
    label.set_editor_property('theme_source',u.InvenzaUIThemeSource.LOCAL)
    for i in range(10):
        label.set_editor_property('theme_source',u.InvenzaUIThemeSource.THEME);label.apply_ui_theme()
        label.set_editor_property('theme_source',u.InvenzaUIThemeSource.LOCAL);label.apply_ui_theme()
    check(color_rgb(label.main_text_block.get_editor_property('color_and_opacity').specified_color)==[.2,.3,.4],'Ten repeated mode changes retain the latest local values')
finally:
    theme.set_editor_property('palette',colors)
    E.release_preview()
    (OUT/'verification.json').write_text(json.dumps({'checks':checks,'assets_saved':False},indent=2,ensure_ascii=False),encoding='utf-8')
u.log('THEME_AUTHORING_VERIFIED '+str(len(checks)))
