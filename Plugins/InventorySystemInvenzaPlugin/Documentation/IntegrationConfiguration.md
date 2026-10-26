# Integration configuration

## Defaults and project overrides

Invenza owns the `InventorySystemInvenzaPlugin` configuration hierarchy.
Both **Project Settings → Project → Invenza Inventory** and **Invenza Save System**
save project overrides to:

```text
<Project>/Config/DefaultInventorySystemInvenzaPlugin.ini
```

Defaults ship in the file with the same name under the plugin's `Config` folder.
Project overrides take precedence; never edit the installed plugin to customize
a project. Configuration changes should be followed by an editor restart so
native and Blueprint component defaults are reloaded consistently.

Example project override (use your actual asset path):

```ini
[/Script/InventorySystemInvenzaPlugin.InvenzaInventoryDeveloperSettings]
InteractionTraceChannel=ECC_Visibility
InventorySettingsAsset=/MyPlugin/Data/MyInventorySettings.MyInventorySettings

[/Script/InventorySystemInvenzaPlugin.InvenzaSaveSettings]
Profile=MyProfile
AutoSaveSeconds=180.0
```

When upgrading from the earlier `Config=Game` version, move both Invenza sections
from the project's `DefaultGame.ini` into `DefaultInventorySystemInvenzaPlugin.ini`.
Move any platform-specific overrides to the corresponding
`Config/<Platform>/<Platform>InventorySystemInvenzaPlugin.ini` too. Preserve all
other project configuration. Save files in `Saved/SaveGames` are unaffected.
The supplied Example Project has already been migrated.

## Gameplay Tags and redirects

The runtime module registers `Config/Tags/InvenzaTags.ini` before loading its
runtime settings assets. This source contains the plugin's tags and the legacy
`Item.Category.Ingredients` → `Item.Category.Ingredient` redirect. Copying tags
from the Example Project is unnecessary. Add project-specific tags through
the project's Gameplay Tags settings or its own tag sources.

Compatibility redirects for renamed C++ classes, properties and functions are
in the plugin's `Config/Engine.ini`. Keep that file when distributing the plugin.

## Interaction collision

The default channel is the built-in **Visibility** channel. Invenza does not add
custom collision channels or modify the host's collision profiles. Choose the
global `InteractionTraceChannel` in Invenza Inventory settings. Interaction
components and storage grids may override it individually through `TraceChannel`
and `InteractionChannel`; serialized Blueprint/instance overrides take precedence.

For a dedicated channel, create a trace channel in **Project Settings → Collision**,
normally with default response **Ignore**. Select its actual slot in the Invenza
settings and make the interactable meshes block that channel with query collision
enabled. Check existing component overrides too. Do not assume that
`ECC_GameTraceChannel1` is free. The Example Project alone uses its own
**Interaction** channel in that slot.

## Input and game setup

For the supplied example, open `Maps/Map_Ex` in plugin content and press Play.
Its GameMode already selects the character, controller and HUD. The native
`InvenzaExampleCharacter` and `InvenzaExamplePlayerController` bases select
Enhanced Input for those example actors without changing global host settings.
Control Rig is a declared dependency for the migrated example animations.
The following input setup applies when integrating your own actors.

Enhanced Input is a declared plugin dependency. In your project, select
`EnhancedPlayerInput` and `EnhancedInputComponent` under **Project Settings → Input**
if your input architecture uses the supplied components. These are host settings;
Invenza does not replace your input classes or add the demo's gameplay context.

Assign the inventory manager's `UISettings` mapping context/actions and the
interaction component's `KeyBindings`. The manager installs its inventory context;
your character/controller must install the contexts for its other actions. Keep
your own GameMode, pawn, HUD and online-subsystem configuration. The Example
Project's `DefaultInput.ini` and `DefaultEngine.ini` are examples, not files to copy
over an existing project.

## Cooking and staging

UAT `BuildPlugin` creates the distributable plugin, not a cooked game. Keep the
plugin's `Config` directory, including `Tags`, when installing it. UE 5.6 stages
enabled plugins' configuration files during normal game packaging.

Explicitly allow the project's override file in its `Config/DefaultGame.ini`
to avoid UE's warning for an unfamiliar configuration name (replace `MyProject`
with the `.uproject` filename without its extension):

```ini
[Staging]
+AllowedConfigFiles=MyProject/Config/DefaultInventorySystemInvenzaPlugin.ini
```

Allow any platform-specific override files you use in the same way. This entry
only affects staging; it does not contain Invenza settings themselves.

Ensure assets selected only through configuration are included in your game cook.
For custom settings, widgets, String Tables and optional meshes, use your project's
Asset Manager cook rules or explicit **Packaging → Additional Asset Directories to
Cook**. Include their dependencies. The supplied defaults must also be checked in
your packaged game; an editor-only load does not prove cook inclusion. Do not
enable cooking the entire Example Project to solve missing plugin references.

## Verification

Start with an empty host project using the standalone distribution. Verify the
default settings asset, save window, runtime text table and inventory templates.
Then override the channel and save profile in the host configuration, restart and
verify that the overrides win. Test interaction, input, UI and save/load in a
packaged game as well as PIE. The repository's read-only
`Scripts/Packaging/audit_integration.py` checks configuration and asset resolution
in an editor commandlet without compiling or saving Blueprints.

## Blueprint-only packaging in UE 5.6

The plugin declares `EnabledByDefault=false`; explicitly enable it in the host's
`.uproject` or Plugins browser. This is intentional: UE 5.6 must recognize that
its prebuilt generic UnrealGame does not contain this project's code plugin and
create a temporary game target when packaging a Blueprint-only host. A successful
cook without that target does not prove that the packaged module can load.
A compatible C++ compiler is still needed to build that target. A project with
its own C++ game module links the plugin through its normal build.
