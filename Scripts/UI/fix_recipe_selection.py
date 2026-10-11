"""Restore mouse hit testing on the recipe list entry without changing its visuals."""
import unreal as u

path='/InventorySystemInvenzaPlugin/UI/Craft/Lists/WBP_RecipeListEntry'
bp=u.load_asset(path)
assert bp
bp.modify()
surface=u.find_object(u.find_object(bp,'WidgetTree'),'RecipeRowLayers')
assert surface
surface.modify()
surface.set_visibility(u.SlateVisibility.VISIBLE)
assert u.InvenzaWidgetEditingLibrary.compile_widget(bp)
defaults=u.get_default_object(bp.generated_class())
defaults.modify()
defaults.set_visibility(u.SlateVisibility.SELF_HIT_TEST_INVISIBLE)
assert u.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
assert surface.get_visibility()==u.SlateVisibility.VISIBLE
u.log('RECIPE_SELECTION_HIT_TEST_FIXED')
