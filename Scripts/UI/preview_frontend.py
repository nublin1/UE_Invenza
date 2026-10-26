from pathlib import Path
import unreal as u
out=Path(u.Paths.project_saved_dir())/'Frontend'
out.mkdir(parents=True,exist_ok=True)
sub=u.get_editor_subsystem(u.UnrealEditorSubsystem)
world=sub.get_editor_world()
cls=u.load_class(None,'/Game/Frontend/UI/WBP_FrontendMenu.WBP_FrontendMenu_C')
widget=u.InvenzaWidgetEditingLibrary.create_preview(world,cls)
for i,name in [(u.FrontendPage.HOME,'home'),(u.FrontendPage.SETTINGS,'settings'),(u.FrontendPage.NETWORK,'network'),(u.FrontendPage.SAVE,'save'),(u.FrontendPage.LOAD,'load')]:
    widget.show_page(i)
    # ScaleBox requests an additional layout pass after a geometry change.
    for _ in range(3):
        assert u.InvenzaWidgetEditingLibrary.render_widget(widget,u.Vector2D(1440,960),str(out/(name+'.png')))
widget.show_page(u.FrontendPage.SETTINGS)
for _ in range(3):
    assert u.InvenzaWidgetEditingLibrary.render_widget(widget,u.Vector2D(1280,720),str(out/'settings-720p.png'))
u.InvenzaWidgetEditingLibrary.release_preview()
print('FRONTEND_PREVIEWS_READY')
u.SystemLibrary.quit_editor()
