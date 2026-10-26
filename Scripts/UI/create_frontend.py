"""Build the frontend's designer-editable assets. Run after building the Editor target.

UnrealEditor-Cmd <project> -run=pythonscript -script=<this file> -unattended
Only /Game/Frontend assets are created or updated. Existing Invenza UI stays shared.
"""
from pathlib import Path
import unreal as u

ROOT = '/Game/Frontend/'
CORE = '/InventorySystemInvenzaPlugin/UI/Core/'
EDIT = u.InvenzaWidgetEditingLibrary
ASSETS = u.AssetToolsHelpers.get_asset_tools()
u.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(['/InventorySystemInvenzaPlugin/', '/Game/Frontend/'], force_rescan=True)
VIS = u.SlateVisibility
GOLD = u.LinearColor(.82, .65, .36, 1)
MUTED = u.LinearColor(.48, .40, .29, 1)
PANEL = u.LinearColor(.035, .024, .013, 1)
CARD = u.LinearColor(.065, .045, .025, 1)

def create(path, cls, factory):
    full = ROOT + path
    existing = u.load_asset(full) if u.EditorAssetLibrary.does_asset_exist(full) else None
    if existing: return existing
    folder, name = full.rsplit('/', 1)
    result = ASSETS.create_asset(name, folder, cls, factory)
    assert result, full
    return result

def save(asset):
    assert u.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)

def make(bp, cls, name):
    w = EDIT.add_template(bp, cls, name)
    w.set_visibility(VIS.VISIBLE)
    return w

def panel(bp, cls, name):
    w = make(bp, cls, name)
    w.clear_children()
    return w

def add(parent, child, pad=0, fill=False):
    child.remove_from_parent()
    slot = parent.add_child(child)
    if hasattr(slot, 'set_padding'): slot.set_padding(u.Margin(pad,pad,pad,pad))
    if hasattr(slot, 'set_horizontal_alignment'): slot.set_horizontal_alignment(u.HorizontalAlignment.H_ALIGN_FILL)
    if isinstance(slot, (u.VerticalBoxSlot, u.HorizontalBoxSlot)):
        slot.set_size(u.SlateChildSize(1., u.SlateSizeRule.FILL if fill else u.SlateSizeRule.AUTOMATIC))
    return slot

def text(bp, name, value, size=16, muted=False):
    w = make(bp, u.TextBlock, name)
    w.set_text(u.Text(value)); w.set_auto_wrap_text(True)
    font = w.get_editor_property('font'); font.size = size
    w.set_font(font); w.set_color_and_opacity(u.SlateColor(specified_color=MUTED if muted else GOLD))
    return w

LABEL = u.load_class(None, CORE+'WBP_LabelBaseText.WBP_LabelBaseText_C')
FONT = u.load_asset('/Engine/EngineFonts/Roboto')
factory=u.WidgetBlueprintFactory(); factory.set_editor_property('parent_class',u.UIButton)
button_bp=create('UI/WBP_MenuButton',None,factory)
main=panel(button_bp,u.Button,'MainButton')
add(main,make(button_bp,u.NamedSlot,'ContentSlot'))
EDIT.set_root(button_bp,main)
main.set_background_color(u.LinearColor.WHITE)
style=main.get_editor_property('widget_style')
for field,color in [('normal',u.LinearColor(.12,.08,.04,1)),('hovered',u.LinearColor(.22,.15,.07,1)),('pressed',u.LinearColor(.3,.2,.08,1)),('disabled',PANEL)]:
    style.set_editor_property(field,u.SlateBrush(draw_as=u.SlateBrushDrawType.BOX,tint_color=u.SlateColor(specified_color=color)))
main.set_style(style)
assert EDIT.compile_widget(button_bp); save(button_bp)
BUTTON = button_bp.generated_class()

def label(bp, name, value, size=16):
    w = make(bp, LABEL, name)
    font = u.SlateFontInfo(font_object=FONT, typeface_font_name='Regular', size=size)
    w.set_editor_properties(dict(text=u.Text(value), font_info=font, color=u.SlateColor(specified_color=GOLD)))
    return w

def sized(bp, name, child, width=None, height=None):
    w = panel(bp, u.SizeBox, name)
    if width: w.set_width_override(width)
    if height: w.set_height_override(height)
    add(w, child)
    return w

def button(bp, name, value):
    w = make(bp, BUTTON, name)
    EDIT.set_slot_content(w, 'ContentSlot', sized(bp, name+'Body', label(bp,name+'Label',value),height=42))
    return w

def card(bp, name, child, pad=18):
    w = panel(bp, u.Border, name)
    w.set_brush_color(CARD); add(w, child,pad)
    return w

def combo(bp, name): return make(bp, u.ComboBoxString, name)

def edit(bp, name, value):
    w=make(bp,u.EditableTextBox,name)
    w.set_text(u.Text(value))
    style=w.get_editor_property('widget_style')
    ts=style.get_editor_property('text_style')
    ts.set_editor_property('font',u.SlateFontInfo(font_object=FONT,typeface_font_name='Regular',size=16))
    style.set_editor_property('text_style',ts)
    style.set_editor_property('foreground_color',u.SlateColor(specified_color=GOLD))
    for field in ['background_image_normal','background_image_hovered','background_image_focused']:
        style.set_editor_property(field,u.SlateBrush(draw_as=u.SlateBrushDrawType.BOX,tint_color=u.SlateColor(specified_color=PANEL)))
    w.set_editor_property('widget_style',style)
    return w

def row(bp, parent, name, title, control):
    r = panel(bp, u.HorizontalBox, name+'Row')
    add(r, sized(bp,name+'CaptionSize',label(bp,name+'Caption',title),width=220),6)
    add(r, control,6,True); add(parent,r,4)

# Audio tree: unclassified project sounds default to Effects; music opts into Music.
master=create('Audio/SC_Master',u.SoundClass,u.SoundClassFactory())
music=create('Audio/SC_Music',u.SoundClass,u.SoundClassFactory())
effects=create('Audio/SC_Effects',u.SoundClass,u.SoundClassFactory())
master.set_editor_property('child_classes',[music,effects])
music.set_editor_property('parent_class',master)
effects.set_editor_property('parent_class',master)
mix=create('Audio/SM_UserSettings',u.SoundMix,u.SoundMixFactory())
for a in [master,music,effects,mix]: save(a)

# Local server-row variant of the existing Invenza button.
if not u.EditorAssetLibrary.does_asset_exist(ROOT+'UI/WBP_ServerRow'):
    assert u.EditorAssetLibrary.duplicate_asset(CORE+'Buttons/WBP_UIButton',ROOT+'UI/WBP_ServerRow')
server=u.load_asset(ROOT+'UI/WBP_ServerRow')
main_label=u.find_object(u.find_object(server,'WidgetTree'),'MainLabel')
font=u.SlateFontInfo(font_object=FONT,typeface_font_name='Regular',size=17); main_label.set_font(font)
main_label.set_color_and_opacity(u.SlateColor(specified_color=GOLD))
assert EDIT.compile_widget(server); save(server)

factory=u.WidgetBlueprintFactory(); factory.set_editor_property('parent_class',u.FrontendMenuWidget)
bp=create('UI/WBP_FrontendMenu',None,factory)
root=panel(bp,u.Border,'FrontendBackground'); root.set_brush_color(PANEL); root.set_padding(u.Margin(32,28,32,28))
body=panel(bp,u.VerticalBox,'FrontendBody')
add(body,label(bp,'Brand','INVENZA',36),4)
add(body,text(bp,'BrandHint','Игра начинается здесь',16,True),4)
pages=panel(bp,u.WidgetSwitcher,'Pages'); add(body,pages,12,True)

# Home.
home=panel(bp,u.HorizontalBox,'HomePage'); add(pages,home)
intro=panel(bp,u.VerticalBox,'HomeIntro')
add(intro,label(bp,'HomeTitle','Главное меню',28),8)
add(intro,text(bp,'HomeDescription','Создайте игру для друзей или присоединитесь к существующей.\nНастройте изображение и звук перед началом.',20),8)
add(intro,text(bp,'HomeFootnote','Локальная сеть и Steam используют одно меню подключения.',16,True),8)
add(home,intro,18,True)
actions=panel(bp,u.VerticalBox,'HomeActions')
for name,value in [('OpenNetwork','Сетевая игра'),('OpenSettings','Настройки'),('QuitGame','Выйти из игры')]:
    add(actions,button(bp,name,value),8)
add(home,sized(bp,'HomeActionsWidth',actions,width=340),18)

# Settings.
settings=panel(bp,u.VerticalBox,'SettingsPage'); add(pages,settings)
add(settings,label(bp,'SettingsTitle','Настройки',28),6)
columns=panel(bp,u.HorizontalBox,'SettingsColumns'); add(settings,columns,4,True)
graphics=panel(bp,u.VerticalBox,'GraphicsControls'); add(graphics,label(bp,'GraphicsTitle','Графика',22),8)
for name,title in [('Resolution','Разрешение'),('WindowMode','Режим экрана'),('Quality','Качество'),('FrameLimit','Частота кадров')]: row(bp,graphics,name,title,combo(bp,name))
row(bp,graphics,'VSync','Вертикальная синхр.',make(bp,u.CheckBox,'VSync'))
add(columns,card(bp,'GraphicsCard',graphics),8,True)
audio=panel(bp,u.VerticalBox,'AudioControls'); add(audio,label(bp,'AudioTitle','Аудио',22),8)
for name,title in [('MasterVolume','Общая громкость'),('MusicVolume','Музыка'),('EffectsVolume','Эффекты')]:
    slider=make(bp,u.Slider,name); slider.set_min_value(0.); slider.set_max_value(1.); slider.set_step_size(.01)
    add(audio,label(bp,name+'Caption',title),8); add(audio,slider,12)
add(audio,text(bp,'AudioHint','Изменения сохраняются кнопкой «Применить».',15,True),8)
add(columns,sized(bp,'AudioWidth',card(bp,'AudioCard',audio),width=360),8)
add(settings,text(bp,'SettingsStatus',''),6)
settingsactions=panel(bp,u.HorizontalBox,'SettingsActions')
for name,value in [('ApplySettings','Применить'),('ResetSettings','По умолчанию'),('SettingsBack','Назад')]: add(settingsactions,button(bp,name,value),6,True)
add(settings,settingsactions)

# Session browser.
network=panel(bp,u.VerticalBox,'NetworkPage'); add(pages,network)
heading=panel(bp,u.HorizontalBox,'NetworkHeading'); add(heading,label(bp,'NetworkTitle','Сетевая игра',28),6,True)
add(heading,text(bp,'BackendLabel','LAN',16,True),6); add(network,heading)
netcols=panel(bp,u.HorizontalBox,'NetworkColumns'); add(network,netcols,4,True)
host=panel(bp,u.VerticalBox,'HostControls'); add(host,label(bp,'HostTitle','Создать игру',22),8)
add(host,label(bp,'ServerNameCaption','Название игры'),6)
add(host,edit(bp,'ServerName','Invenza'),6)
row(bp,host,'PlayerCount','Игроков',combo(bp,'PlayerCount'))
add(host,button(bp,'HostGame','Создать и играть'),6)
add(host,label(bp,'DirectTitle','Подключиться по IP',20),12)
add(host,edit(bp,'IPAddress','127.0.0.1:7777'),6)
add(host,button(bp,'ConnectIP','Подключиться'),6)
add(host,text(bp,'IPHint','Для подключения по IP запустите профиль LAN.',14,True),6)
add(netcols,sized(bp,'HostWidth',card(bp,'HostCard',host),width=420),8)
browser=panel(bp,u.VerticalBox,'BrowserControls'); add(browser,label(bp,'BrowserTitle','Доступные игры',22),8)
scroll=panel(bp,u.ScrollBox,'ServerScroll'); add(scroll,panel(bp,u.VerticalBox,'ServerList')); add(browser,scroll,8,True)
browseractions=panel(bp,u.HorizontalBox,'BrowserActions')
add(browseractions,button(bp,'FindGames','Найти / обновить'),6,True); add(browseractions,button(bp,'CancelSearch','Отменить поиск'),6,True)
add(browser,browseractions); add(netcols,card(bp,'BrowserCard',browser),8,True)
add(network,text(bp,'NetworkStatus','Готово к подключению'),8)
add(network,button(bp,'BackHome','В главное меню'),6)

# Display confirmation has no other active controls until kept or reverted.
confirm=panel(bp,u.VerticalBox,'ConfirmationPage'); add(pages,confirm)
add(confirm,label(bp,'ConfirmTitle','Подтверждение экрана',28),18)
add(confirm,text(bp,'ConfirmLabel','Сохранить новый режим экрана?',22),18)
add(confirm,button(bp,'KeepDisplay','Сохранить'),12); add(confirm,button(bp,'RevertDisplay','Вернуть прежние'),12)

scale=panel(bp,u.ScaleBox,'FrontendScale')
scale.set_stretch(u.Stretch.SCALE_TO_FIT)
scale.set_stretch_direction(u.StretchDirection.DOWN_ONLY)
add(scale,sized(bp,'FrontendSize',body,width=1200,height=760))
add(root,scale,28); EDIT.set_root(bp,root)
assert EDIT.compile_widget(bp)
u.get_default_object(bp.generated_class()).set_editor_property('server_button_class',server.generated_class())
save(bp)

# Small session control; F10 also works while the gameplay cursor is captured.
factory=u.WidgetBlueprintFactory(); factory.set_editor_property('parent_class',u.FrontendSessionBar)
bar=create('UI/WBP_SessionBar',None,factory)
canvas=panel(bar,u.CanvasPanel,'SessionCanvas')
leave=button(bar,'LeaveGame','F10 · В главное меню')
slot=add(canvas,sized(bar,'SessionButtonSize',leave,width=280,height=48))
slot.set_anchors(u.Anchors(minimum=u.Vector2D(1,0),maximum=u.Vector2D(1,0)))
slot.set_alignment(u.Vector2D(1,0)); slot.set_position(u.Vector2D(-24,24)); slot.set_auto_size(True)
EDIT.set_root(bar,canvas); assert EDIT.compile_widget(bar); save(bar)

# Dedicated non-partitioned frontend map. Gameplay remains on Map_Ex.
sub=u.get_editor_subsystem(u.LevelEditorSubsystem)
map_path=ROOT+'Maps/L_MainMenu'
if u.EditorAssetLibrary.does_asset_exist(map_path): sub.load_level(map_path)
else: assert sub.new_level(map_path)
world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
world.get_world_settings().set_editor_property('default_game_mode',u.FrontendMenuGameMode)
assert sub.save_current_level()
print('FRONTEND_ASSETS_CREATED',map_path)
