# Shared UI theme

## Editing appearance

Open `InventorySystemInvenzaPlugin/Data/UI/DA_UITheme_Default` with **Show Plugin Content** enabled.

- **Palette** contains named roles such as `Text.Primary`, `Text.Secondary`, `Panel.Background`, `Panel.Inset`, `Panel.Row`, `Frame.Standard`, button states, `Drop.Valid`, `Drop.Invalid`, `Drop.Partial`, `Progress.Fill` and `Progress.Track`.
- **Default Font** supplies the font for Typography entries with **Use Default Font** enabled. Other entries retain their own Font Object. Typography includes size, face and outline settings.
- **Brushes** contains decorative frame, button, scrollbar, progress and state-icon brushes. Each brush can reference a Palette entry through **Color Role**, keeping shared colors in one place.

The default theme preserves the existing appearance. Distinct older shades and font variants remain distinct roles. Additional roles such as `Text.Queue`, `Slider.Track`, `Frame.Inventory`, `Color.Gray…` and `Color.LegacyMagenta` retain those older variants. Runtime code uses explicit roles; it does not infer a role by comparing colors.

Select a theme in **Project Settings → Game → Invenza Inventory → Interface → Theme → UI Theme**. Duplicate the default Data Asset for another project and select the copy. The selection is stored in `Config/DefaultInventorySystemInvenzaPlugin.ini`. The plugin default theme does not require `/Game` content.

The old Default UI Font, Default Text Color and four font-size settings under **Legacy Appearance** are compatibility fallbacks when no theme is selected. A selected theme is the active source.

## Widget bindings and overrides

The base widget exposes **Theme Source** in Class Defaults and on instances:

- **Theme** applies all listed bindings.
- **Theme With Overrides** skips bindings marked **Override Locally**.
- **Local** retains the widget's authored appearance.

Each **Theme Bindings** entry explicitly identifies **Widget Name**, **Property Path** and **Role**. An empty name targets the UserWidget itself; another name targets a component in its own WidgetTree. Examples include `Color`, `FontInfo`, `LeftBorderStyle.BrushColor` and `WidgetStyle.Normal`. Configure an embedded Core widget's own bindings to style its internals; the outer tree cannot address its private widget tree by name.

For a local label color, select Theme With Overrides, mark its `Color` binding as Override Locally and edit Color. Use `FontInfo` for a local font. Keep other bindings enabled if they should continue using the theme.

Existing Invenza Core elements and windows are connected, including inventories, lists, equipment, storage, trading, crafting, modals, tooltips, controls help, saves and the example frontend. Derive a new Widget Blueprint from `UInvenzaBaseWidget` or the appropriate Core class and add explicit bindings. Inheritance alone cannot assign semantic roles to unknown new components.

## Designer preview and content updates

Open `UI/Theme/WBP_UIThemePreview` for labels, Cyrillic text, an input, ordinary/toggle/disabled buttons and a progress display. Theme edits refresh open Designer instances without a player or GameInstance. Compile an individual custom Blueprint after changing its structure.

The theme is applied at widget construction and property synchronization, without Tick. An Engine Subsystem loads and caches the selected Data Asset. Runtime theme switching is not a separate user-facing feature.

`UpdateText` updates label content. `UpdateAppearance` changes appearance without assigning text. The compatible `UpdateTextAndFont` API explicitly updates both; pass the owner's already themed style. Explicit UpdateFont / UpdateAppearance / UpdateTextAndFont calls mark the affected instance fields as local overrides, so an owner-supplied style assigned before attachment survives Construct. The owner remains themed and forwards appearance changes without replacing text. Clear Override Locally or select Theme to resume the binding. Custom Blueprint state logic can use Theme Color, Theme Slate Color, Theme Font and Theme Brush with an explicit role and local fallback.

## Boundaries

The theme does not change Visibility, RenderOpacity, existing color alpha, hit testing, focus, action availability, grid dimensions, slot placement or layout padding. Palette alpha does not replace an element's current alpha. Font metrics and decorative brush shapes affect drawing, so choose them to fit the authored layout.

Item icon resources, dynamic materials, character captures and input contents stay with their data owners. Toggle, drag feedback and progress use dedicated roles while their state and gameplay data remain in the existing widgets.

## Packaging

The plugin adds the theme selected in Project Settings to the cook package list. Its fonts and brush resources are normal Data Asset dependencies. Each custom theme therefore does not need its own Always Cook setting. The preview Blueprint is an editor aid and should not be added to the gameplay layout.

Migration and verification scripts are in `Scripts/UI/*ui_theme*.py` in the Example Project, outside plugin runtime dependencies. Backups and reports for this migration are under `Saved/UITheme20261031` and are excluded from release archives.

## Authored values and binding validation

Authored UserWidget Color, FontInfo, BrushStyle and other appearance properties remain separate from resolved output. Local restores the latest local values. Theme uses theme values with an authored fallback for missing roles. Theme With Overrides respects explicit exceptions. Editing local parameters during preview does not lose them when returning to Local.

Raw UMG components use a separate typed authoring store. Editor property events update only the edited fields, preserving unrelated local values. Theme output is never assigned to the UserWidget's own authored properties. Native appearance consumers use ResolveThemeColorForProperty, ResolveThemeFont, ResolveThemeSlateColor, ResolveThemeBrush or ResolveThemeStyle on a structure copy. RefreshThemeAppearance updates drawing without replacing content.

The plugin includes the InvenzaUIThemeEditor module. Standard Unreal asset validation checks Widget Blueprints and embedded Core elements for missing widgets/properties, unsupported property types, incompatible or missing roles, duplicate targets and overlapping whole/nested properties. Diagnostics identify the Blueprint, widget, path and role. The check also runs before cook, reporting invalid bindings as packaging errors. Runtime deduplicates identical class/binding/theme errors and uses authored fallback values. Validation does not compile or save assets.

Toggle normal backgrounds are resolved from authored values or successfully resolved roles, independently of the displayed selected color. Removing a role falls back to the local normal background. Appearance refresh never broadcasts OnToggled or OnButtonClicked.
