#  Nublin Studio 2026 All Rights Reserved.

"""Add the storage restock action using the existing Core button and label."""
import unreal as u

root = '/InventorySystemInvenzaPlugin/'
edit = u.InvenzaWidgetEditingLibrary
bp = u.load_asset(root + 'UI/Core/OperationsPanel/WBP_Inventory_OperationsPanel')
tree = u.find_object(bp, 'WidgetTree')
place_all = u.find_object(tree, 'Button_PlaceAll')
panel = place_all.get_parent()
bp.modify()
panel.modify()
button = edit.add_template(bp, u.load_class(None, root + 'UI/Core/Buttons/WBP_UIButtonWithSlot.WBP_UIButtonWithSlot_C'), 'Button_PlaceMatching')
button.set_editor_property('is_toggle_button', False)
button.set_editor_property('default_size', u.Vector2D(35, 35))
button.set_tool_tip_text(u.Text('Restock — move all stacks of items already present in this storage from your inventory.'))
button.set_visibility(u.SlateVisibility.COLLAPSED)
box = edit.add_template(bp, u.SizeBox, 'PlaceMatchingSize')
box.set_width_override(35)
box.set_height_override(35)
box.set_visibility(u.SlateVisibility.HIT_TEST_INVISIBLE)
label = edit.add_template(bp, u.load_class(None, root + 'UI/Core/WBP_LabelBaseText.WBP_LabelBaseText_C'), 'PlaceMatchingLabel')
label.set_editor_property('text', u.Text('+='))
font = label.get_editor_property('font_info')
font.font_object = u.load_asset('/Engine/EngineFonts/Roboto')
font.typeface_font_name = 'Bold'
font.size = 18
label.set_editor_property('font_info', font)
label.set_editor_property('color', u.SlateColor(specified_color=u.LinearColor(.82, .65, .36, 1)))
label.set_visibility(u.SlateVisibility.HIT_TEST_INVISIBLE)
box.clear_children()
slot = box.add_child(label)
slot.set_horizontal_alignment(u.HorizontalAlignment.H_ALIGN_CENTER)
slot.set_vertical_alignment(u.VerticalAlignment.V_ALIGN_CENTER)
edit.set_slot_content(button, 'NamedSlot_21', box)
children = [panel.get_child_at(i) for i in range(panel.get_children_count()) if panel.get_child_at(i) != button]
children.insert(children.index(place_all) + 1, button)
# Preserve every existing slot's settings when inserting the new button.
slot_settings = [(w, w.slot.get_editor_property('padding'), w.slot.get_editor_property('size'),
                  w.slot.get_editor_property('horizontal_alignment'), w.slot.get_editor_property('vertical_alignment'))
                 for w in children if w != button]
panel.clear_children()
for child in children:
    slot = panel.add_child_to_vertical_box(child)
    if child == button:
        slot.set_padding(u.Margin(0, 3, 0, 3))
        slot.set_horizontal_alignment(u.HorizontalAlignment.H_ALIGN_CENTER)
    else:
        _, padding, size, horizontal, vertical = next(s for s in slot_settings if s[0] == child)
        slot.set_padding(padding)
        slot.set_size(size)
        slot.set_horizontal_alignment(horizontal)
        slot.set_vertical_alignment(vertical)
assert edit.compile_widget(bp)
assert u.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
u.log('PLACE_MATCHING_BUTTON_SAVED')
