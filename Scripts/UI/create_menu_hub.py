"""Author only the common menu, its pages, and save/load input changes in Unreal Editor."""
from pathlib import Path
import csv
import shutil
import unreal as u

PROJECT = Path(u.Paths.project_dir())
PLUGIN = '/InventorySystemInvenzaPlugin/'
FRONT = '/Game/Frontend/'
helpers = (PROJECT/'Scripts/UI/restyle_craft_choose.py').read_text(encoding='utf-8')
exec(helpers.split('# A reusable wrapping label')[0])
LABEL = WRAPPED = u.load_class(None, PLUGIN+'UI/SaveLoad/WBP_SaveLabel.WBP_SaveLabel_C')
BUTTON = u.load_class(None, FRONT+'UI/WBP_MenuButton.WBP_MenuButton_C')


def preserve(path):
    disk = PROJECT/('Content/'+path[len('/Game/'):] if path.startswith('/Game/') else 'Plugins/InventorySystemInvenzaPlugin/Content/'+path[len(PLUGIN):])
    disk = disk.with_suffix('.uasset')
    target = PROJECT/'Saved/MenuHub/Before'/disk.relative_to(PROJECT)
    if disk.exists() and not target.exists():
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(disk, target)


def create(path, parent):
    if u.EditorAssetLibrary.does_asset_exist(path): return u.load_asset(path)
    factory = u.WidgetBlueprintFactory(); factory.set_editor_property('parent_class', parent)
    directory, name = path.rsplit('/', 1)
    result = u.AssetToolsHelpers.get_asset_tools().create_asset(name, directory, None, factory)
    assert result, path
    return result


table_path = FRONT+'Localization/ST_FrontendMenu'
table = u.load_asset(table_path) if u.EditorAssetLibrary.does_asset_exist(table_path) else None
if table is None:
    directory, name = table_path.rsplit('/',1)
    table = u.AssetToolsHelpers.get_asset_tools().create_asset(name, directory, None, u.StringTableFactory())
with (PROJECT/'Scripts/UI/Data/FrontendMenuTexts.csv').open(encoding='utf-8', newline='') as source:
    for row in csv.DictReader(source): EDIT.set_string_table_entry(table,row['Key'],row['SourceString'])
assert u.EditorAssetLibrary.save_loaded_asset(table,only_if_is_dirty=False)

# Menu captions wrap to their assigned column width, including narrow audio/help panels.
label_path = FRONT+'UI/WBP_MenuLabel'
if not u.EditorAssetLibrary.does_asset_exist(label_path):
    assert u.EditorAssetLibrary.duplicate_asset(PLUGIN+'UI/SaveLoad/WBP_SaveLabel',label_path)
label_bp = u.load_asset(label_path)
widget(label_bp,'MainTextBlock').set_auto_wrap_text(True)
widget(label_bp,'MainTextBlock').set_editor_property('wrap_text_at',0.)
save(label_bp)
LABEL = WRAPPED = label_bp.generated_class()


def tr(key): return EDIT.string_table_text(table,key)


def caption(bp,name,key,sz=16,bold=False):
    result = label(bp,name,None,sz,bold)
    result.set_editor_property('text',tr(key))
    return result


def native_text(bp,name,key=None,sz=15):
    result=make(bp,u.TextBlock,name)
    result.set_text(tr(key) if key else u.Text(''))
    result.set_auto_wrap_text(True)
    font=BASE_FONT.copy(); font.size=sz; result.set_font(font)
    result.set_color_and_opacity(u.SlateColor(specified_color=GOLD))
    return result


def action(bp,name,key):
    button=make(bp,BUTTON,name);button.set_visibility(VIS.VISIBLE)
    EDIT.set_slot_content(button,'ContentSlot',size(bp,name+'ContentSize',caption(bp,name+'Label',key),height=42))
    return button


def row(bp,parent,name,key,control):
    line=panel(bp,u.HorizontalBox,name+'Row')
    attach(line,size(bp,name+'CaptionSize',caption(bp,name+'Caption',key,15),width=210),margin(0,0,14,0))
    control.set_visibility(VIS.VISIBLE)
    attach(line,control,0,True)
    attach(parent,line,margin(0,8,0,8))


def edit(bp,name,value):
    result=make(bp,u.EditableTextBox,name); result.set_visibility(VIS.VISIBLE)
    result.set_text(u.Text(value))
    return result


# Settings own their controls and display-mode confirmation.
bp=create(FRONT+'UI/WBP_SettingsPage',u.FrontendSettingsPage)
pages=panel(bp,u.WidgetSwitcher,'SettingsPages')
body=panel(bp,u.VerticalBox,'SettingsBody'); attach(pages,body)
attach(body,caption(bp,'SettingsTitle','Settings',26,True),margin(0,0,0,16))
columns=panel(bp,u.HorizontalBox,'SettingsColumns');attach(body,columns,0,True)
graphics=panel(bp,u.VerticalBox,'GraphicsControls')
attach(graphics,caption(bp,'GraphicsTitle','Graphics',21,True),margin(0,0,0,12))
for name in ['Resolution','WindowMode','Quality','FrameLimit']:
    row(bp,graphics,name,name,make(bp,u.ComboBoxString,name))
row(bp,graphics,'VSync','VSync',make(bp,u.CheckBox,'VSync'))
attach(columns,surface(bp,'GraphicsSurface',graphics,PANEL,16),margin(0,0,14,0),True)
audio=panel(bp,u.VerticalBox,'AudioControls')
attach(audio,caption(bp,'AudioTitle','Audio',21,True),margin(0,0,0,12))
for name in ['MasterVolume','MusicVolume','EffectsVolume']:
    attach(audio,caption(bp,name+'Caption',name,15),margin(0,10,0,6))
    slider=make(bp,u.Slider,name);slider.set_visibility(VIS.VISIBLE)
    slider.set_min_value(0.);slider.set_max_value(1.);slider.set_step_size(.01)
    attach(audio,slider,margin(0,0,0,12))
attach(audio,caption(bp,'AudioHint','AudioHint',13))
attach(columns,size(bp,'AudioColumnWidth',surface(bp,'AudioSurface',audio,PANEL,16),width=330))
attach(body,native_text(bp,'SettingsStatus'),margin(0,12,0,8))
buttons=panel(bp,u.HorizontalBox,'SettingsActions')
for name,key in [('ApplySettings','Apply'),('ResetSettings','ResetSettings'),('SettingsBack','Back')]:attach(buttons,action(bp,name,key),margin(4),True)
attach(body,buttons)
confirm=panel(bp,u.VerticalBox,'DisplayConfirmation');attach(pages,confirm)
attach(confirm,caption(bp,'DisplayConfirmTitle','ConfirmDisplay',26,True),margin(0,18,0,20))
attach(confirm,native_text(bp,'ConfirmLabel',sz=20),margin(0,0,0,24))
attach(confirm,action(bp,'KeepDisplay','KeepDisplay'),margin(0,0,0,12))
attach(confirm,action(bp,'RevertDisplay','RevertDisplay'))
EDIT.set_root(bp,pages);assert EDIT.compile_widget(bp)
cdo=u.get_default_object(bp.generated_class())
cdo.set_editor_property('resolution_presets',[u.IntPoint(1280,720),u.IntPoint(1600,900),u.IntPoint(1920,1080)])
cdo.set_editor_property('window_mode_labels',[tr(k) for k in ['Fullscreen','Borderless','Windowed']])
cdo.set_editor_property('quality_labels',[tr(k) for k in ['QualityLow','QualityMedium','QualityHigh','QualityEpic','QualityCinematic','QualityCustom']])
cdo.set_editor_property('frame_limit_presets',[0.,30.,60.,90.,120.,144.,165.,240.])
save(bp); SETTINGS=bp.generated_class()

# Network browser is a separate page, retaining the session subsystem.
bp=create(FRONT+'UI/WBP_NetworkPage',u.FrontendNetworkPage)
body=panel(bp,u.VerticalBox,'NetworkBody')
heading=panel(bp,u.HorizontalBox,'NetworkHeading')
attach(heading,caption(bp,'NetworkTitle','Network',26,True),0,True)
attach(heading,native_text(bp,'BackendLabel','LANBackend'))
attach(body,heading,margin(0,0,0,16))
columns=panel(bp,u.HorizontalBox,'NetworkColumns');attach(body,columns,0,True)
host=panel(bp,u.VerticalBox,'HostControls')
attach(host,caption(bp,'HostTitle','HostTitle',21,True),margin(0,0,0,12))
attach(host,caption(bp,'ServerNameCaption','ServerName'),margin(0,0,0,6))
attach(host,edit(bp,'ServerName','Invenza'),margin(0,0,0,12))
row(bp,host,'PlayerCount','PlayerCount',make(bp,u.ComboBoxString,'PlayerCount'))
attach(host,action(bp,'HostGame','HostGame'),margin(0,0,0,18))
attach(host,caption(bp,'DirectTitle','DirectTitle',20,True),margin(0,0,0,8))
attach(host,edit(bp,'IPAddress','127.0.0.1:7777'),margin(0,0,0,8))
attach(host,action(bp,'ConnectIP','ConnectIP'),margin(0,0,0,8))
attach(host,caption(bp,'IPHint','IPHint',13))
attach(columns,size(bp,'HostWidth',surface(bp,'HostSurface',host,PANEL,16),width=420),margin(0,0,14,0))
browser=panel(bp,u.VerticalBox,'BrowserControls')
attach(browser,caption(bp,'BrowserTitle','BrowserTitle',21,True),margin(0,0,0,12))
scroll=panel(bp,u.ScrollBox,'ServerScroll');scroll.set_visibility(VIS.VISIBLE)
attach(scroll,panel(bp,u.VerticalBox,'ServerList'));attach(browser,scroll,0,True)
buttons=panel(bp,u.HorizontalBox,'BrowserActions')
for name,key in [('FindGames','FindGames'),('CancelSearch','CancelSearch')]:attach(buttons,action(bp,name,key),margin(4),True)
attach(browser,buttons)
attach(columns,surface(bp,'BrowserSurface',browser,PANEL,16),0,True)
attach(body,native_text(bp,'NetworkStatus'),margin(0,12,0,8))
attach(body,action(bp,'BackHome','Back'))
EDIT.set_root(bp,body);assert EDIT.compile_widget(bp)
u.get_default_object(bp.generated_class()).set_editor_property('server_button_class',u.load_class(None,FRONT+'UI/WBP_ServerRow.WBP_ServerRow_C'))
save(bp);NETWORK=bp.generated_class()

# Separate save and load pages use the existing save behavior, with no nested input ownership.
views=[]
for name,view,title in [('WBP_SaveGamePage',u.InvenzaSaveView.SAVE,'SaveTitle'),('WBP_LoadGamePage',u.InvenzaSaveView.LOAD,'LoadTitle')]:
    path=PLUGIN+'UI/SaveLoad/'+name
    if not u.EditorAssetLibrary.does_asset_exist(path):
        assert u.EditorAssetLibrary.duplicate_asset(PLUGIN+'UI/SaveLoad/WBP_InvenzaSaveMenu',path)
    bp=u.load_asset(path)
    body=widget(bp,'SaveBody');body.clear_children()
    save_controls=panel(bp,u.VerticalBox,'SaveControls')
    load_controls=panel(bp,u.VerticalBox,'LoadControls')
    session_controls=panel(bp,u.VerticalBox,'SessionControls')
    for child in ['QuickSaveButtonSize','ManualSaveButtons']:attach(save_controls,widget(bp,child),margin(0,0,0,6))
    for child in ['ContinueButtonSize','SlotsSize','LoadButtonSize']:attach(load_controls,widget(bp,child),margin(0,0,0,6))
    for child in ['TrustRow','TrustDescriptionLabel','RecoverButtonSize','ResetButtonSize','Confirmation']:attach(session_controls,widget(bp,child),margin(0,0,0,6))
    for child in [widget(bp,'SaveHeader'),widget(bp,'ScopeLabel'),save_controls,load_controls,session_controls,widget(bp,'StatusLabel')]:attach(body,child,margin(0,0,0,10))
    page_texts=u.load_asset(PLUGIN+'Data/Localization/ST_InvenzaMenuPages')
    assert page_texts, 'Run Scripts/Packaging/prepare_standalone_assets.py first.'
    widget(bp,'TitleLabel').set_editor_property('text',EDIT.string_table_text(page_texts,title))
    widget(bp,'CloseButtonLabel').set_editor_property('text',EDIT.string_table_text(page_texts,'Back'))
    widget(bp,'SaveWindowSize').set_width_override(900);widget(bp,'SaveWindowSize').set_height_override(600)
    assert EDIT.compile_widget(bp)
    cdo=u.get_default_object(bp.generated_class());cdo.set_editor_property('view',view);cdo.set_editor_property('embedded_menu_page',True)
    save(bp);views.append(bp.generated_class())
SAVE,LOAD=views

# Shared top-level menu: only page selection and root actions.
path=FRONT+'UI/WBP_FrontendMenu';preserve(path)
bp=u.load_asset(path)
body=panel(bp,u.VerticalBox,'FrontendBody')
attach(body,caption(bp,'MenuTitle','MenuTitle',28,True),margin(0,0,0,6))
attach(body,caption(bp,'MenuHint','MenuHint',15),margin(0,0,0,16))
pages=panel(bp,u.WidgetSwitcher,'Pages');attach(body,pages,0,True)
home=panel(bp,u.VerticalBox,'HomePage');attach(pages,home)
for name,key in [('ResumeButton','Resume'),('OpenSave','Save'),('OpenLoad','Load'),('OpenSettings','Settings'),('OpenNetwork','Network'),('LeaveSession','LeaveSession'),('QuitGame','Quit')]:
    attach(home,action(bp,name,key),margin(0,0,0,10))
attach(home,caption(bp,'MenuStatus','WaitForSave',14));widget(bp,'MenuStatus').set_editor_property('text',u.Text(''))
for name,cls in [('SettingsPage',SETTINGS),('NetworkPage',NETWORK),('SavePage',SAVE),('LoadPage',LOAD)]:attach(pages,make(bp,cls,name))
content=frame(bp,'MenuFrame',surface(bp,'MenuSurface',body,BG,24))
content=size(bp,'FrontendSize',content,width=1080,height=740)
fit=panel(bp,u.ScaleBox,'FrontendScale');fit.set_stretch(u.Stretch.SCALE_TO_FIT);fit.set_stretch_direction(u.StretchDirection.DOWN_ONLY)
slot=attach(fit,content);slot.set_horizontal_alignment(H.H_ALIGN_CENTER);slot.set_vertical_alignment(V.V_ALIGN_CENTER)
canvas=panel(bp,u.CanvasPanel,'FrontendCanvas');slot=canvas.add_child(fit)
slot.set_anchors(u.Anchors(u.Vector2D(0,0),u.Vector2D(1,1)));slot.set_offsets(margin(20))
EDIT.set_root(bp,canvas);assert EDIT.compile_widget(bp)
u.get_default_object(bp.generated_class()).set_editor_property('is_focusable',True)
save(bp)

# Retain the existing entry widget, replacing its old direct-exit behavior and caption.
path=FRONT+'UI/WBP_SessionBar';preserve(path)
bp=u.load_asset(path)
widget(bp,'LeaveGameLabel').set_editor_property('text',tr('MenuButton'))
save(bp)

# No dedicated save/load keyboard shortcuts; access is through the common menu.
path=PLUGIN+'Input/InputMappingInventoryContext';preserve(path)
context=u.load_asset(path)
for action_name in ['IA_SaveInv','IA_LoadInv']:
    control=u.load_asset(PLUGIN+'Input/UIActions/'+action_name)
    assert control
    context.unmap_all_keys_from_action(control)
assert u.EditorAssetLibrary.save_loaded_asset(context,only_if_is_dirty=False)
u.log('INVENZA_MENU_HUB_ASSETS_OK')
