# Integrating Invenza into another project

Language: [Русский](../ru/ProjectIntegration.md) | **English**

Detailed companion to section 2 of the [overview](GettingStartedOverview.md), covering the source plugin and a basic inventory on your own character.

The archive includes a playable example and its dependencies. Enable **Show Plugin Content**, open `/InventorySystemInvenzaPlugin/Maps/Map_Ex` and press **Play**. Its character, controller, HUD and GameMode are configured. The sections below describe integration with your own gameplay classes.

For a packaged game, add the example to **Packaging → List of maps to include in a packaged build**, and choose it in **Maps & Modes → Game Default Map** to start there. Plugin packaging distributes uncooked assets; the host project cooks the game.

## 2.1. Prepare the target project

Start with UE 5.6, as specified by the demo `.uproject`. Other engine versions require separate code and asset validation.

The current standalone descriptor declares **Win64** only. See
[supported targets and dependencies](SupportedTargets.md) before adding platforms.

This version includes a source C++ module, so install a C++ toolchain suitable for your engine version. On Windows, use Visual Studio or Rider with a configured compiler.

For a Blueprint-only target, a convenient source-build workflow is to add an empty C++ class in the editor, close the editor, and build. This creates a game module so the plugin can be built alongside it. Gameplay can still remain in Blueprint.

## 2.2. Copy the plugin

Close the target editor and copy this repository's `Plugins/InventorySystemInvenzaPlugin` into the target's `Plugins` folder:

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

Keep the plugin name and Content structure. Existing assets use virtual paths such as `/InventorySystemInvenzaPlugin/...`.

Copy `.uplugin`, `Source`, `Content`, `Config`, and `Resources`. Generated `Intermediate` is unnecessary. Demo `Binaries` may not match your engine build; rebuild from source for this workflow.

Unreal discovers project plugins under `Plugins` and includes source modules in project builds. See [Unreal plugin documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine?application_version=5.6).

`UE_GridInventory` and `InvenzaUIEditor` belong to the demonstration project, not runtime plugin dependencies. They are unnecessary for basic integration. A Blueprint referencing a demo-project class still needs that code migrated or its parent/reference replaced.

## 2.3. Enable and build

Enable `InventorySystemInvenzaPlugin` and Enhanced Input through Plugins or add entries to the existing `.uproject` `Plugins` array:

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

This is an array fragment, not a complete file. Preserve other entries and valid JSON. Regenerate C++ project files, build the target's Editor target in Development Editor for your platform, and open after a successful build.

The plugin's `.Build.cs` declares UMG, Enhanced Input, Gameplay Tags, and other dependencies. If your own C++ module directly uses plugin classes, add:

```csharp
PrivateDependencyModuleNames.Add("InventorySystemInvenzaPlugin");
```

Use public dependencies if your public headers require them. Blueprint configuration does not require this step.

Enable Show Plugin Content in Content Browser. Check `Data`, `Input`, `UI`, `HUDs`, and `Blueprints`.

## 2.4. Assets and dependencies

Keep all plugin Content for the first integration. After a working launch, create project-specific copies of tables/settings/widgets and replace references gradually.

| Purpose | Path inside plugin Content |
|---|---|
| Global settings | `Data/Settings/InvenzaSettings` |
| Item definitions | `Data/DT_ItemData` |
| Input | `Input` |
| Primary layout | `UI/Layouts/Layout/WBP_PrimaryLayout` |
| Example HUD | `HUDs/BP_InvenzaHUD` |
| Example character | `Blueprints/Examples/BP_Character` |
| Example controller | `Blueprints/Examples/Controllers/BP_PlayerController` |
| Example GameMode | `Blueprints/Examples/GameMode/BP_GM_Invenza` |

The example meshes, animations and prototype geometry are inside the plugin. Its map requires no original `/Game` content or demo-project C++ modules. Enhanced Input and Control Rig are declared plugin dependencies. The example native bases select Enhanced Input locally; host input defaults need not change to run `Map_Ex`.

Configure project `Config` separately below. When retaining your own character/animations, use the example as a component-setup reference.

## 2.5. Global Data Asset and tags

Plugin defaults already select `InvenzaSettings`. To use your own settings asset, change `InventorySettingsAsset` under Project Settings → Project → Invenza Inventory.

Project overrides belong in `Config/DefaultInventorySystemInvenzaPlugin.ini`:

```ini
[/Script/InventorySystemInvenzaPlugin.InvenzaInventoryDeveloperSettings]
InventorySettingsAsset=/InventorySystemInvenzaPlugin/Data/Settings/InvenzaSettings.InvenzaSettings
```

Use your copied asset's path if applicable. The GameInstance subsystem loads it automatically.

The plugin registers its own tags and legacy tag redirect from `Config/Tags/InvenzaTags.ini`. Do not copy the demo's tag list. Add project-specific tags in your project's own sources. See [configuration and migration](IntegrationConfiguration.md) for configuration precedence and packaging.

Check `MainInvTag`, drag/modal widget classes, and `PickupClass` for dropping. The main tag must match the startup inventory. For trading, check currency item and tag.

## 2.6. Enhanced Input and interaction collision

Under Project Settings → Input:

| Setting | Value |
|---|---|
| Default Player Input Class | `EnhancedPlayerInput` |
| Default Input Component Class | `EnhancedInputComponent` |

Equivalent `Config/DefaultInput.ini`:

```ini
[/Script/Engine.InputSettings]
DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput
DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent
```

See [input configuration](InputConfiguration.md) for detailed keys/actions/modifiers. Assign the manager's `UISettings.InventoryMappingContext`, inventory toggle, and transfer modifiers. The plugin supplies `InputMappingInventoryContext` and actions; inspect actual keys in the asset.

The manager adds its context at priority 1 and binds to PlayerController Input Component. `InteractionComponent` binds to the character's Input Component. Check both when using custom input architecture.

The standalone default uses the built-in **Visibility** trace channel. Interactive collision must block the selected channel. Change the global `InteractionTraceChannel` in Invenza Inventory settings, and review serialized `InteractionComponent.TraceChannel` / `StorageGridVisualizer.InteractionChannel` overrides.

A dedicated channel is optional. Create a free trace channel in the host project if needed; Invenza does not reserve `ECC_GameTraceChannel1` or replace collision profiles. The Example Project uses that slot for its own Interaction channel.

## 2.7. Your character

Add `InventoryCollectionComponent` and `UIInventoryManager`. For world interaction, add `InteractionComponent` and configure `KeyBindings`, trace channel, and distance.

The current implementation finds `CameraComponent` on the character and traces from its position/direction. Adapt this if the camera belongs to another actor.

Start with one manager `StartupInventories` entry:

| Field | Initial setup |
|---|---|
| `InventoryClass` | `SlotbasedInventory` for a grid |
| `InventoryTag` | Match global `MainInvTag` |
| `InventoryType` | `MainInventory` |
| `InventoryMaxWeightCapacity` | `-1` for unlimited weight |
| `MaxStackCount` | `-1` for unlimited stacks |
| `StartItems` | One/two `DT_ItemData` rows with positive quantities |

Choose one UI approach:

**Authored grid:** keep `bCollectInvDataFromWidget = true`. Place an inventory container with a prepared grid in the game menu. Match its `TargetInventoryTag` to `InventoryTag`. The manager locates it through the UI provider and collects slot structure.

**Generated grid:** use `bCollectInvDataFromWidget = false`, positive `InitNumberRows`/`InitNumColumns`, cell dimensions, and slot class. Assign compatible `ContainerWidgetClass`, `InventoryWidgetClass`, and optional `OperationPanelWidgetClass`. The manager creates UI through the provider. Verify Blueprint compatibility: the container needs `ContainerSlot`; the grid needs its base class's expected children.

Also assign item visual, dragged item, tooltip, and highlight classes in `UISettings`, separately from the global asset.

## 2.8. HUD and layers

Components alone do not provide UI. Inspect `BP_InvenzaHUD`, `WBP_PrimaryLayout`, `WBP_GameMenuLayer`, and `WBP_GameLayer`.

Set your map's GameMode to use the intended Default Pawn, PlayerController, and Invenza HUD or equivalent custom HUD. Check World Settings GameMode Override.

A custom HUD should:

1. Create `PrimaryLayout` for the local PlayerController and add it to screen.
2. Prepare `GameLayer`, `GameMenuLayer`, `MenuLayer`, and `ModalLayer` with required child names/types.
3. Use a `GameMenuLayerInv`-based layer to register the inventory UI provider; use `GameLayer` for interaction prompts.
4. Implement `HUDProvider.GetPrimaryLayout` to return the created layout.
5. Call LocalPlayer `ModalWindowManager.InitializeUI` after HUD/layout preparation to connect modal dialogs.

`GameHUD_Inz` does not create the layout by itself; Blueprint or your implementation must do so.

Layers find Owning Player Pawn and manager in `NativeOnInitialized`. Supply the correct Owning Player after the Pawn exists. The manager automatically initializes 0.1 seconds after BeginPlay, so its UI provider must be ready then. The current deferred timer is UObject-bound and canceled in EndPlay.

If your project spawns/possesses characters later, coordinate registration and initialization explicitly. Repeating full initialization without state checks may duplicate inventories or handlers; increasing the delay is not a replacement for coordinated startup.

## 2.9. First launch

For new mechanics, see [world containers](WorldContainers.md), [shared storage](StorageAccessPoint.md), and [character exchange](CharacterInventoryExchange.md). `InventoryMannequin` has an optional mesh configured through Example Mannequin Mesh; the standalone plugin does not require `/Game` mannequin content. Shared storage also needs `WBP_StorageAccess` and `WBP_InventoryWorkspace`.

For [persistence](InventorySaving.md), configure the save profile, stable identities, and menu classes separately.

Use a test map with one local player:

1. Compile Character, PlayerController, HUD, and Widget Blueprints; fix missing parents/references.
2. Start PIE and verify GameMode creates the intended Pawn/controller/HUD.
3. Check starting items, opening, and dragging.
4. Add a pickup/container and check interaction.
5. Check context menus, including HUD provider, modal layer, and actions.

Then add equipment, crafting, trading, and more inventories. Build a separate package and repeat essential checks there.

## 2.10. Common issues

| Symptom | Check |
|---|---|
| Plugin missing | `.uplugin` location, folder structure, enabled state |
| Module fails to load | Build result for engine/platform |
| `Settings Asset is null` | Assigned asset and path |
| `UIProvider is not set` | Game menu, Owning Player, Pawn availability at initialization |
| Missing tag / empty grid | Matching tags, grid creation mode, widget structure |
| Missing starting items | Table/row, quantity, available slots |
| Toggle key fails | Enhanced Input, controller input component, context/action |
| Target not detected | Character camera, trace channel/range, collision Block |
| Context menu missing | `HUDProvider`, returned layout, modal layer, `InitializeUI` |

Some bindings use PlayerController index 0. Multiple local players need separate adaptation and testing.

## Blueprint-only packaging in UE 5.6

The plugin declares `EnabledByDefault=false`; explicitly enable it in the host's
`.uproject` or Plugins browser. This is intentional: UE 5.6 must recognize that
its prebuilt generic UnrealGame does not contain this project's code plugin and
create a temporary game target when packaging a Blueprint-only host. A successful
cook without that target does not prove that the packaged module can load.
A compatible C++ compiler is still needed to build that target. A project with
its own C++ game module links the plugin through its normal build.
