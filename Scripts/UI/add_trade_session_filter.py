"""Add the optional session-sales filter using the existing Core button."""
import unreal as u

root = '/InventorySystemInvenzaPlugin/'
bp = u.load_asset(root + 'UI/Core/ItemFiltersPanel/WBP_ItemFiltersPanel')
tree = u.find_object(bp, 'WidgetTree')
bar = u.find_object(tree, 'FilterButtons')
assert bp and tree and bar
bp.modify()
bar.modify()
button = u.InvenzaWidgetEditingLibrary.add_template(
    bp, u.load_class(None, root + 'UI/Core/Buttons/WBP_UIButton.WBP_UIButton_C'), 'RecentlySoldButton')
button.set_editor_property('default_text', u.Text('↶'))
button.set_editor_property('default_image', None)
button.set_editor_property('default_size', u.Vector2D(32, 32))
button.set_editor_property('is_toggle_button', True)
button.set_editor_property('is_toggle_on', False)
button.set_editor_property('toggle_color', u.LinearColor(.82, .65, .36, 1))
button.set_tool_tip_text(u.Text('Sold this session — show items you sold to this trader. Resets when trading closes.'))
button.set_visibility(u.SlateVisibility.COLLAPSED)
if button.get_parent() != bar:
    slot = bar.add_child_to_horizontal_box(button)
    slot.set_padding(u.Margin(4, 0, 0, 0))
    slot.set_vertical_alignment(u.VerticalAlignment.V_ALIGN_CENTER)
assert u.InvenzaWidgetEditingLibrary.compile_widget(bp)
assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
u.log('TRADE_SESSION_FILTER_SAVED')
