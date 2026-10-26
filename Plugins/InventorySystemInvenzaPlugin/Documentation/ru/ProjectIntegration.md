# Подключение Invenza к другому проекту

Язык: **Русский** | [English](../en/ProjectIntegration.md)

Это подробное дополнение к разделу 2 [общего описания](GettingStartedOverview.md). Оно описывает перенос текущего плагина из репозитория и подключение базового инвентаря к своему персонажу.

Архив содержит готовую карту-пример `Maps/Map_Ex` и её зависимости. После включения плагина включите **Show Plugin Content**, откройте `/InventorySystemInvenzaPlugin/Maps/Map_Ex` и нажмите **Play**. Персонаж, контроллер, HUD и GameMode уже назначены. Ниже описано подключение к собственным игровым классам.

Для упаковки игры добавьте эту карту в **Project Settings → Packaging → List of maps to include in a packaged build**. Чтобы игра запускалась на ней, выберите её в **Maps & Modes → Game Default Map**. Архив плагина содержит исходные ассеты; cook выполняется при сборке целевого проекта.

## 2.1. Подготовка целевого проекта

Для первого подключения используйте UE 5.6 — эту версию указывает демонстрационный `.uproject`. Перенос на другую версию движка потребует отдельной проверки сборки и ассетов.

Текущий descriptor самостоятельной поставки объявляет только **Win64**.
Матрица версий и зависимости модулей описаны в [Supported targets](../en/SupportedTargets.md).

Текущая версия Invenza содержит исходный C++-модуль. Для сборки нужен установленный инструментальный набор C++ для выбранной версии Unreal Engine. На Windows можно собирать проект через Visual Studio или Rider с настроенным компилятором.

Если целевой проект создан только на Blueprint, удобный путь для работы с исходной версией плагина — добавить в него пустой C++-класс средствами редактора, затем закрыть редактор и собрать проект. Это создаст проектный C++-модуль и позволит собирать плагин вместе с проектом. Саму игровую логику после этого можно продолжать писать на Blueprint.

## 2.2. Копирование плагина

Закройте редактор целевого проекта. Скопируйте плагин из папки `Plugins/InventorySystemInvenzaPlugin` этого репозитория в одноимённую папку внутри `Plugins` целевого проекта.

Ожидаемая структура:

```text
MyProject/
├── MyProject.uproject
├── Config/
├── Content/
└── Plugins/
    └── InventorySystemInvenzaPlugin/
        ├── InventorySystemInvenzaPlugin.uplugin
        ├── Config/
        ├── Content/
        ├── Resources/
        └── Source/
```

Сохраните имя плагина и структуру его `Content`. Ассеты используют виртуальные пути вида `/InventorySystemInvenzaPlugin/...`; сохранение этих путей упрощает перенос существующих ссылок.

Для сборки из исходников переносите `.uplugin`, `Source`, `Content`, `Config` и `Resources`. Генерируемую папку `Intermediate` переносить не требуется. Бинарные файлы из `Binaries` демонстрационного проекта могут не соответствовать сборке вашего движка; для этого варианта подключения плагин следует собрать заново из исходников.

Unreal Engine обнаруживает проектные плагины в папке `Plugins`, а исходные модули включаются в сборку проекта. Это стандартный механизм [проектных плагинов Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine?application_version=5.6).

Модули `UE_GridInventory` и `InvenzaUIEditor` относятся к демонстрационному проекту. В зависимостях runtime-модуля плагина они не указаны, поэтому для базового подключения переносить их не нужно. Если выбранный Blueprint ссылается на класс демонстрационного проекта, такую ссылку потребуется заменить или отдельно перенести соответствующий код.

## 2.3. Включение и сборка

Включите `InventorySystemInvenzaPlugin` и `Enhanced Input` в целевом проекте. Это можно сделать через окно Plugins или добавить следующие записи в существующий массив `Plugins` файла `.uproject`:

```json
{
    "Name": "InventorySystemInvenzaPlugin",
    "Enabled": true
},
{
    "Name": "EnhancedInput",
    "Enabled": true
}
```

Это фрагмент массива, а не полный `.uproject`: сохраните остальные записи и корректный JSON. Затем обновите файлы C++-проекта и соберите Editor target целевого проекта в конфигурации Development Editor для своей платформы. Откройте редактор после успешной сборки.

Модуль Invenza уже объявляет свои зависимости от UMG, Enhanced Input, Gameplay Tags и других модулей движка в `InventorySystemInvenzaPlugin.Build.cs`. Если ваш собственный C++-модуль напрямую использует классы Invenza, добавьте зависимость `InventorySystemInvenzaPlugin` в его `.Build.cs`:

```csharp
PrivateDependencyModuleNames.Add("InventorySystemInvenzaPlugin");
```

Используйте `PublicDependencyModuleNames`, если зависимость нужна публичным заголовкам вашего модуля. Для настройки через Blueprint этот шаг не требуется.

В Content Browser включите отображение содержимого плагинов — Show Plugin Content. Проверьте, что доступны папки Invenza `Data`, `Input`, `UI`, `HUDs` и `Blueprints`.

## 2.4. Ассеты и их зависимости

Для первого подключения сохраните полный `Content` плагина. После рабочего запуска можно подготовить собственные копии таблиц, настроек и виджетов в содержимом проекта и постепенно заменить ссылки на них.

Основные ассеты для проверки:

| Назначение | Путь внутри Content плагина |
|---|---|
| Общие настройки | `Data/Settings/InvenzaSettings` |
| Предметы | `Data/DT_ItemData` |
| Действия и контексты ввода | `Input` |
| Главный layout | `UI/Layouts/Layout/WBP_PrimaryLayout` |
| HUD-пример | `HUDs/BP_InvenzaHUD` |
| Персонаж-пример | `Blueprints/Examples/BP_Character` |
| Контроллер-пример | `Blueprints/Examples/Controllers/BP_PlayerController` |
| Пример GameMode | `Blueprints/Examples/GameMode/BP_GM_Invenza` |

Меши, анимации и геометрия карты-примера перенесены внутрь `Content` плагина. Исходные `/Game/Characters` и `/Game/LevelPrototyping`, а также модули `UE_GridInventory` и `InvenzaUIEditor` для её запуска не нужны. Enhanced Input и Control Rig объявлены зависимостями плагина.

Конфигурацию из папки проекта `Config` настройте отдельно по следующим пунктам. При использовании своих персонажа и анимаций перенесённый пример служит ориентиром для настройки компонентов.

## 2.5. Общий Data Asset и Gameplay Tags

Плагин уже выбирает `InvenzaSettings` по умолчанию. Чтобы использовать собственный ассет, измените `InventorySettingsAsset` в Project Settings → Project → Invenza Inventory.

Эквивалентная запись в `Config/DefaultInventorySystemInvenzaPlugin.ini` целевого проекта:

```ini
[/Script/InventorySystemInvenzaPlugin.InvenzaInventoryDeveloperSettings]
InventorySettingsAsset=/InventorySystemInvenzaPlugin/Data/Settings/InvenzaSettings.InvenzaSettings
```

Если используете свою копию настроек, укажите её путь. Подсистема Invenza загружает выбранный ассет при инициализации GameInstance. Создавать подсистему вручную не требуется.

Плагин сам регистрирует теги и redirect старой категории из `Config/Tags/InvenzaTags.ini`. Копировать список тегов демо не требуется. Собственные теги добавляйте в источники целевого проекта. Настройки проекта имеют приоритет над одноимённым конфигом плагина. При обновлении перенесите две секции Invenza из старого `DefaultGame.ini` в `DefaultInventorySystemInvenzaPlugin.ini`. Подробности и проверка упаковки: [Integration configuration](../en/IntegrationConfiguration.md).

В Data Asset убедитесь, что назначены `MainInvTag`, классы перетаскивания и модальных виджетов, а для выбрасывания предметов — `PickupClass`. Основной тег должен совпадать с тегом стартового инвентаря. Для торговли отдельно проверьте предмет валюты и её Gameplay Tag.

## 2.6. Enhanced Input и канал взаимодействия

На готовой `Map_Ex` ввод настроен базовыми классами `InvenzaExampleCharacter` и `InvenzaExamplePlayerController`; глобальные классы ввода целевого проекта менять не требуется. Карта использует встроенный канал Visibility. Следующие настройки относятся к подключению собственных персонажа и контроллера.

В Project Settings → Input проверьте классы ввода:

| Настройка | Значение |
|---|---|
| Default Player Input Class | `EnhancedPlayerInput` |
| Default Input Component Class | `EnhancedInputComponent` |

В исходном проекте эти значения записаны в `Config/DefaultInput.ini`:

```ini
[/Script/Engine.InputSettings]
DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput
DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent
```

Подробное назначение клавиш, действий и модификаторов приведено в [руководстве по настройке управления](InputConfiguration.md).

В `UISettings` компонента `UIInventoryManager` назначьте `InventoryMappingContext`, действие открытия инвентаря и нужные модификаторы переноса. В плагине есть `InputMappingInventoryContext` и соответствующие Input Actions. Конкретные клавиши проверьте в самом Mapping Context.

Менеджер добавляет контекст инвентаря с приоритетом 1 и привязывает действия к Input Component PlayerController. `InteractionComponent` привязывает действия взаимодействия к Input Component персонажа. При собственной архитектуре ввода проверьте оба места: наличия одного контекста недостаточно, если нужный Input Component отсутствует.

По умолчанию используется встроенный канал **Visibility**. Интерактивная коллизия должна блокировать выбранный канал. Общий `InteractionTraceChannel` задаётся в Invenza Inventory; проверьте также сохранённые переопределения `InteractionComponent.TraceChannel` и `StorageGridVisualizer.InteractionChannel` в Blueprint и экземплярах.

Отдельный канал необязателен. При необходимости создайте свободный Trace Channel в проекте: плагин не занимает `ECC_GameTraceChannel1` и не меняет collision profiles. Только Example Project использует этот слот для своего канала Interaction.

## 2.7. Подключение к своему персонажу

Добавьте к Character Blueprint компоненты `InventoryCollectionComponent` и `UIInventoryManager`. Для взаимодействия с миром добавьте `InteractionComponent` и назначьте ему действия в `KeyBindings`, канал и дистанцию трассировки.

Текущая реализация взаимодействия ищет `CameraComponent` на персонаже и выполняет трассировку от его положения по направлению взгляда. Для персонажа с камерой на другом акторе эту часть потребуется адаптировать.

Для первого запуска оставьте одну запись в `StartupInventories` менеджера:

| Поле | Начальная настройка |
|---|---|
| `InventoryClass` | `SlotbasedInventory` для сеточного инвентаря |
| `InventoryTag` | Тот же тег, что в `MainInvTag` общего Data Asset |
| `InventoryType` | `MainInventory` |
| `InventoryMaxWeightCapacity` | `-1` для отсутствия ограничения веса |
| `MaxStackCount` | `-1` для отсутствия ограничения количества стаков |
| `StartItems` | Одна-две ссылки на строки `DT_ItemData` с положительным количеством |

Далее выберите один из двух способов подключения интерфейса.

**Готовая сетка в UI.** Оставьте `bCollectInvDataFromWidget = true`. Разместите контейнер инвентаря с готовым сеточным виджетом в игровом меню. В виджете назначьте `TargetInventoryTag`, совпадающий с `InventoryTag`. Менеджер найдёт этот виджет через UI provider и получит структуру слотов из него.

**Сетка по настройкам.** Задайте `bCollectInvDataFromWidget = false`, ненулевые `InitNumberRows` и `InitNumColumns`, размеры ячеек и класс слота. Назначьте совместимые `ContainerWidgetClass`, `InventoryWidgetClass` и, при необходимости, `OperationPanelWidgetClass`. Менеджер создаст виджет и добавит его через UI provider. Совместимость выбранных Blueprint-классов нужно проверить в редакторе; для контейнера важен `ContainerSlot`, для сеточного виджета — ожидаемые базовым классом элементы.

В `UISettings` также назначьте классы визуального предмета, перетаскивания, подсказки и подсветки. Их поля находятся на менеджере, отдельно от общего Data Asset.

## 2.8. Подключение HUD и слоёв UI

Одного добавления компонентов к персонажу недостаточно: инвентарь должен получить UI provider. Для первого подключения изучите `BP_InvenzaHUD` и виджеты `WBP_PrimaryLayout`, `WBP_GameMenuLayer` и `WBP_GameLayer` из плагина.

Настройте GameMode целевой карты: назначьте свой Character в Default Pawn Class, подходящий PlayerController и HUD Invenza либо собственный HUD с соответствующей логикой. Проверьте GameMode Override в World Settings карты — он может перекрывать настройки проекта.

Если создаёте свой HUD, его логика должна:

1. Создать экземпляр `PrimaryLayout` для локального PlayerController и добавить его на экран.
2. Подготовить слои `GameLayer`, `GameMenuLayer`, `MenuLayer` и `ModalLayer` с ожидаемыми именами и типами дочерних виджетов.
3. Использовать слой игрового меню на основе `GameMenuLayerInv` для регистрации UI provider инвентаря. Для подсказок взаимодействия используется `GameLayer`.
4. Реализовать интерфейс `HUDProvider`, чтобы `GetPrimaryLayout` возвращал созданный layout.
5. После подготовки HUD и layout вызвать `InitializeUI` у `ModalWindowManager` — подсистемы LocalPlayer. Это связывает модальные окна со слоем `ModalLayer`.

Базовый C++-класс `GameHUD_Inz` сам по себе не создаёт layout. Создание интерфейса должно выполняться в Blueprint HUD или в вашей реализации.

**Порядок запуска важен.** Слои ищут Owning Player Pawn и менеджер в `NativeOnInitialized`. Создавайте их с правильным Owning Player, когда персонаж уже доступен. Менеджер автоматически вызывает `InitializeInventoryManager` через 0,1 секунды после BeginPlay, поэтому UI provider должен быть готов к этому моменту.

Если архитектура проекта создаёт или назначает персонажа позже, нужно согласовать регистрацию UI и инициализацию менеджера в реализации. Повторный вызов всей инициализации без проверки состояния может повторно создать инвентари или добавить обработчики событий. Увеличение задержки не заменяет согласованный порядок запуска.

## 2.9. Первый запуск

Для [сохранения и загрузки](InventorySaving.md) отдельно настройте профиль, постоянные идентификаторы и классы меню.

Для новых механик после базового подключения используйте отдельные инструкции: [предметы и контейнеры](WorldContainers.md), [общее хранилище](StorageAccessPoint.md), [манекен и обмен с NPC](CharacterInventoryExchange.md). Для `InventoryMannequin` меш необязателен и задаётся через Example Mannequin Mesh; standalone-плагин не требует содержимого /Game. Для склада проверяйте также `WBP_StorageAccess` и общий `WBP_InventoryWorkspace`.

Проверяйте базовое подключение в отдельной тестовой карте с одним локальным игроком:

1. Откройте и скомпилируйте используемые Character, PlayerController, HUD и Widget Blueprint. Исправьте отсутствующие родительские классы и ссылки.
2. Запустите PIE и проверьте, что активный GameMode создаёт нужного персонажа, контроллер и HUD.
3. Проверьте отображение стартовых предметов, открытие инвентаря и перетаскивание между слотами.
4. Добавьте тестовый Pickup или контейнер и проверьте взаимодействие через выбранный канал.
5. Проверьте контекстное меню, если оно используется: для него должны быть готовы HUD provider, модальный слой и настройки действий.

После этого подключайте дополнительные инвентари, экипировку, торговлю и крафт. Для проверки packaged build выполните отдельную сборку целевого проекта и повторите основные действия в ней.

## 2.10. Типичные проблемы при переносе

| Симптом | Что проверить |
|---|---|
| Плагин не виден | Положение `.uplugin`, структуру папок и включение плагина |
| Модуль не загружается | Результат сборки под выбранную версию движка и платформу |
| `Settings Asset is null` | Выбор `InventorySettingsAsset` в Project Settings и путь ассета |
| `UIProvider is not set` | Создание игрового меню, правильного Owning Player и наличие Pawn к моменту инициализации слоя |
| `TargetInventoryTag is not set` или пустая сетка | Совпадение тегов, выбранный способ создания сетки и структуру виджета |
| Стартовые предметы отсутствуют | Таблицу и Row Name в `StartItems`, количество и доступные слоты |
| Клавиша открытия не работает | Enhanced Input, Input Component контроллера, контекст и назначенную Input Action |
| Объект не обнаруживается | Камеру персонажа, канал трассировки, дистанцию и ответ Block на коллизии объекта |
| Контекстное меню не появляется | `HUDProvider`, возвращаемый layout, `ModalLayer` и вызов `ModalWindowManager.InitializeUI` |

В текущем коде часть привязок ввода использует PlayerController с индексом 0. Подключение для нескольких локальных игроков требует отдельной адаптации и проверки.
