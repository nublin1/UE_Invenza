# UE_Invenza

Documentation / Документация: [English](Docs/en/README.md) | [Русский](Docs/ru/README.md)

Invenza: Modular Inventory System for Unreal Engine 5
Invenza is a flexible and modular inventory system for Unreal Engine 5, designed to handle item interactions, containers, trading, equipment management, and state saving. Built on a component-based architecture and widget-driven UI, it offers seamless integration with both single-player and multiplayer environments.

🛠 Character Components
To set up Invenza in your Character Blueprint, add the following components:

UIInventoryManager – Manages the inventory UI and related actions.

InventoryCollectionComponent – Stores the character’s items.

UInteractionComponent – Handles world object interactions.

UEquipmentComponent (optional) – Manages equipment (requires UIInventoryManager).

InvenzaSaveManager (optional) – Handles inventory saving/loading.

🔧 Features
Grid-based and list-based inventory views

Drag-and-drop support

Advanced filtering and sorting

Quick item transfers

Persistent inventory saving

Trading system

Flexible setup via Data Tables and components

📌 Installation
Add the required components to your Character Blueprint.

Configure Data Tables for items, equipment, and trade settings.

Attach the appropriate inventory components to containers, vendors, and interactable objects.

🛠 Configuration & Usage
Refer to the Invenza documentation for detailed setup and customization options.

- [Общее описание и настройка](Docs/ru/GettingStartedOverview.md)
- [Подключение к другому проекту](Docs/ru/ProjectIntegration.md)
- [Настройка управления](Docs/ru/InputConfiguration.md)
- [Предметы и контейнеры в мире](Docs/ru/WorldContainers.md)
- [Общий доступ к хранилищам](Docs/ru/StorageAccessPoint.md)
- [Манекен и обмен с другим персонажем](Docs/ru/CharacterInventoryExchange.md)
- [Все разделы документации](Docs/ru/README.md)
- [Сохранение и загрузка / Saving and loading](Docs/ru/InventorySaving.md) — [English](Docs/en/InventorySaving.md)
