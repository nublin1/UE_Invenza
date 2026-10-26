# Active interaction distance

Language: [Русский](../ru/InteractionDistance.md) | **English**

Global setting: `/InventorySystemInvenzaPlugin/Data/Settings/InvenzaSettings`, **Interaction → Max Interaction Distance**. Select the asset through Project Settings → Invenza Inventory → Inventory Settings Asset.

The default is **600 cm**. Distance is measured in 3D between participant and target actor positions. The boundary is inclusive: interaction ends when the value is exceeded. This does not change trace/detection range. `StorageAccessPoint` also checks its own `MaxUseDistance`.

The server checks active interaction every **0.25 s** using a timer, which stops when interaction ends. Moving either participant out of range or destroying the target ends interaction through `StopInteract`. The client receives a session number so a late notification cannot close a newly reopened interaction.

Container, vendor, and access-point references are revoked; existing notifications close their UI. A crafting station removes only the operator registered by that participant. Closing its panel does not clear the queue; operator-dependent stations retain their normal blocking rules.

The logic does not depend on the player's camera or controller. NPC interactions through `InteractionComponent` use the same mechanism. Server handlers in `UIInventoryManager` called without an `InteractionComponent` use a fallback timer with the same global limit. For custom C++ that starts a persistent interaction outside normal `Interact`, call `TrackActiveInteraction(Target, Type)` on the server after successful entry and use `StopInteract` to finish.

Test: `Invenza.Interaction.Range`.

Historical automation names in this guide refer to checks whose source files were removed at the owner's request. They are not available in the current distribution; use the described manual scenarios.
