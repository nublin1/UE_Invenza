"""Update only ingredient backgrounds; preserve recipe click and other UI edits."""
import unreal as u

root='/InventorySystemInvenzaPlugin/'
edit=u.InvenzaWidgetEditingLibrary
image_class=u.load_class(None,root+'UI/Core/WBP_BaseImage.WBP_BaseImage_C')
for path in ['UI/Craft/Ingredient/WBP_RequirementOptionEntry','UI/Craft/Ingredient/WBP_RequirementOptionEntryAlt']:
    bp=u.load_asset(root+path)
    assert bp
    bp.modify()
    tree=u.find_object(bp,'WidgetTree')
    surface=u.find_object(tree,'IngredientSurface')
    background=u.find_object(tree,'IngredientSurfaceBackground')
    row=u.find_object(tree,'IngredientRow')
    marker=u.find_object(tree,'SelectedMarkerSize')
    assert surface and background and row and marker
    for widget in [surface,background,row,marker]: widget.modify()
    style=background.get_editor_property('brush_style')
    style.brush_color=u.LinearColor(.035,.024,.013,1)
    background.set_editor_property('brush_style',style)
    selected=edit.add_template(bp,image_class,'SelectionBackground')
    selected.set_editor_property('base_material',None)
    selected.set_editor_property('brush_style',u.UIBrushStyle(
        brush=u.SlateBrush(draw_as=u.SlateBrushDrawType.BOX,tint_color=u.SlateColor(specified_color=u.LinearColor.WHITE)),
        brush_color=u.LinearColor(.065,.045,.025,1)))
    selected.set_visibility(u.SlateVisibility.HIT_TEST_INVISIBLE)
    selected.set_render_opacity(0)
    # Keep the background behind the content and the existing selection stripe.
    surface.clear_children()
    for widget,padding in [(background,0),(selected,0),(row,10),(marker,0)]:
        slot=surface.add_child(widget)
        slot.set_padding(u.Margin(padding,padding,padding,padding))
        slot.set_horizontal_alignment(u.HorizontalAlignment.H_ALIGN_LEFT if widget==marker else u.HorizontalAlignment.H_ALIGN_FILL)
        slot.set_vertical_alignment(u.VerticalAlignment.V_ALIGN_FILL)
    assert edit.compile_widget(bp)
    assert u.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
u.log('INGREDIENT_HIGHLIGHT_FIXED')
