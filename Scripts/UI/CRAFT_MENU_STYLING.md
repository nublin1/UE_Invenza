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
