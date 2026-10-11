# Цвета и отступы меню крафта

Цвета сохранены в Blueprint-ассетах, в экземплярах вложенных Core-виджетов.
Общего Data Asset с палитрой пока нет. Скрипты в этой папке нужны только
для редактирования ассетов; во время игры они не выполняются.

В Content Browser включите Show Plugin Content и откройте
InventorySystemInvenzaPlugin → UI.

| Что настраивается | Blueprint | Элемент в Hierarchy |
|---|---|---|
| Фон всего меню | Craft/WBP_CraftDashboard | CraftSurfaceBackground |
| Фон очереди | Craft/WBP_CraftDashboard | QueueBackground |
| Фон карточки рецепта | Craft/Lists/WBP_QueueCraftList_Dashboard | EntrySurfaceBackground |
| Фон сообщения о блокировке | Core/Status/WBP_StatusMessage | StatusSurfaceBackground |

У перечисленных фоновых элементов: **Details → UI → Material → Brush Style → Brush Color**.
Вложенный **Brush → Tint** оставлен белым, чтобы цвет менялся в одном поле.
Выбирайте экземпляр в Designer родительского виджета: цвет задан именно там,
а не в Class Defaults общего WBP_BaseImage.

- Название рецепта: WBP_QueueCraftList_Dashboard → QueueItemName → UI → Config → Color.
- Числа и вспомогательные подписи: Core/Progress/WBP_RemainingCounter_CraftDashboard
  и WBP_RemainingCounter_CraftDashboard_WorkAmount → нужный Core Label → UI → Config → Color.
- Рамка карточки: WBP_QueueCraftList_Dashboard → EntryFrame → Top/Bottom/Left/Right Border
  → соответствующий Border Style → Brush Color.
- Аналогично рамка очереди находится в QueueFrame, а рамки кнопок —
  в WBP_DashboardControlPanel → Btn_AddTaskFrame / Btn_PauseFrame.
- Промежуток между карточками: WBP_QueueCraftList_Dashboard → Class Defaults
  → Padding → Bottom = 8. ListView добавляет этот отступ к собственным отступам.

Толщина пустых Border задана через Padding: 1 с каждой стороны даёт 2 единицы Slate.
Это сохраняет видимые линии при уменьшении интерфейса примерно до 67%.

После изменения свойств нажмите Compile и Save.

## Окно выбора рецепта

`Craft/WBP_CraftMenuChoose` использует ту же палитру и размер 1160 × 680,
что и панель очереди. Его размер и положение в игре задаются у экземпляра
`WBP_CraftMenuChoose` внутри `WBP_GameMenuInvenza`.

| Что настраивается | Blueprint | Элемент |
|---|---|---|
| Основной фон | Craft/WBP_CraftMenuChoose | ChooseSurfaceBackground |
| Фон списка | Craft/WBP_CraftMenuChoose | RecipeColumnSurfaceBackground |
| Фон деталей | Craft/WBP_CraftMenuChoose | DetailSurfaceBackground |
| Фон строки рецепта | Craft/Lists/WBP_RecipeListEntry | RecipeRowBackground |
| Выбор и наведение | Craft/Lists/WBP_RecipeListEntry | SelectionHighlight / HoverHighlight / SelectionIndicator |
| Фон результата | Craft/WBP_CraftMenuDetail | OutputSurfaceBackground |
| Фон ингредиентов | Craft/Ingredient/WBP_RequirementOptionEntry и …EntryAlt | IngredientSurfaceBackground |
| Выбранный ингредиент | Те же два виджета | SelectionBackground (фон) / SelectedMarker (полоска) |

Фоновые Core Image: **Brush Style → Brush Color**; вложенный Brush Tint белый.
Рамки: четыре **Border Style → Brush Color** у соответствующего Core Frame.
Тексты: **UI → Config → Color / Font Info** у экземпляров Core Label.
Непрозрачность SelectionHighlight / HoverHighlight / SelectionIndicator
меняется в C++ только при выборе строки и наведении, без Tick.

У ингредиентов обычный фон одинаковый для основного и альтернативных ресурсов.
`URequirementOptionEntry::SetToggleStatus` одновременно переключает непрозрачность
`SelectionBackground` и `SelectedMarker`; светлый фон не закреплён за основным ресурсом.

Специализированные варианты сохраняют общие Core-ассеты без изменений:

- `Core/WBP_LabelBaseText_Wrapped` — текст с переносом строк.
- `Craft/WBP_CraftChooseTitleBar` — заголовок и существующая кнопка закрытия.
- `Craft/Lists/WBP_RecipeSearch` — поиск рецептов без неработающих категорий предметов.
- `Craft/Lists/WBP_RecipeNameText` — перенос названия в списке; ширина переноса 150,
  чтобы ListView сразу рассчитывал полную высоту длинной строки.
- `Craft/WBP_CraftChooseQuantity` — компактная компоновка существующих кнопок количества.
- `Craft/Ingredient/WBP_IngredientAmounts` — счётчик «Need / Have».

Кнопка добавления использует существующий `Core/Buttons/WBP_UIButton`, как в
панели очереди. Общие Blueprint кнопок не изменены. У стрелок увеличения и
максимума в локальном селекторе выставлен Render Scale X = -1.

Промежутки строк: **Class Defaults → Padding → Bottom**, 8 у рецептов и 10
у групп требований. Название, описание, результат и суммарный объём работы
обновляются через `UCraftMenuDetail::SetCraftDetail`. Рецепты и предметные
иконки берутся из данных; изображения-заглушки в игровые данные не добавлены.

Скрипт редактирования: `Scripts/UI/restyle_craft_choose.py`.
Исходные ассеты перед его первым запуском сохранены в `Saved/CraftUI/BeforeChoose`.
