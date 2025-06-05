# UE_Invenza

Invenza: Modular Inventory System for Unreal Engine 5
Invenza is a flexible and modular inventory system for Unreal Engine 5, designed to handle item interactions, containers, trading, equipment management, and state saving. Built on a component-based architecture and widget-driven UI, it offers seamless integration with both single-player and multiplayer environments.

🛠 Character Components
To set up Invenza in your Character Blueprint, add the following components:

UIInventoryManager – Manages the inventory UI and related actions.

ItemCollection – Stores the character’s items.

UInteractionComponent – Handles world object interactions.

UEquipmentManagerComponent (optional) – Manages equipment (requires UIInventoryManager).

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
