# Standalone plugin with playable example

The distributable code plugin does not require the demo project's `/Game` content.
The archive includes the playable `Maps/Map_Ex`, its character, controller, HUD,
GameMode and migrated mannequin/prototype dependencies inside the plugin.
The repository also contains the full Example Project and its separate frontend.

## Plugin installation

Copy the packaged `InventorySystemInvenzaPlugin` directory into your project's
`Plugins` directory, enable the plugin, and restart the editor. Enhanced Input is
declared as a plugin dependency, together with Control Rig for the example animations. Build a source distribution with the toolchain
supported by your Unreal Engine version.

For configuration precedence, migration, input and collision setup, see
[Integration configuration](IntegrationConfiguration.md).

Plugin defaults are provided by `Config/DefaultInventorySystemInvenzaPlugin.ini`. You can override them through
**Project Settings → Invenza Inventory** and **Invenza Save System**. The default
inventory settings asset, equipment definitions, save-window class and save text
table are inside the plugin. Do not copy the demo project's entire configuration
into an existing project.

Invenza registers its own gameplay tags from `Config/Tags/InvenzaTags.ini`. Projects
can add more categories, equipment slots and UI tags without changing that file.
The module registers this source directly; no manual tag copy is required.

## Interaction traces

The standalone default uses the built-in **Visibility** trace channel. It does
not reserve `GameTraceChannel1` in the host project. Make interactive collision
components block the selected channel.

The global default is `InteractionTraceChannel` in Invenza Inventory settings.
Individual interaction components and storage grids can still override their
channels. The Example Project explicitly retains its existing **Interaction**
channel and collision profiles. After changing defaults, check any Blueprint
instances that have serialized their own channel overrides.

## Optional mannequin

`AInventoryMannequin` is an example actor with inventory and equipment components.
Its visual mesh is optional. Assign **Example Mannequin Mesh** in Project Settings
or configure the actor's mesh in your own Blueprint. **Example Equipment Table**
selects its initial slot definitions. Example defaults are applied when the actor
is constructed or initialized; native class registration does not load its
equipment table. No mesh under `/Game/Characters` is required
by the C++ module.

The repository's `Config/DefaultInventorySystemInvenzaPlugin.ini` explicitly assigns its demo mannequin;
this project-specific choice is not included in the standalone plugin defaults.

## Run the included example

Enable **Show Plugin Content**, open
`/InventorySystemInvenzaPlugin/Maps/Map_Ex` and press **Play**.
The map's GameMode selects the example character, controller and HUD. The example
native base classes select Enhanced Input locally, so the host project's default
input classes do not need changing. The map uses Visibility for interaction.
No `/Game` content or project-specific C++ modules need migrating.

To include this map in a packaged game, add it to **Packaging → List of maps to
include in a packaged build**. Select it under **Maps & Modes → Game Default Map**
if it should open on startup. `BuildPlugin` distributes uncooked assets; the host
project performs its own game cook.

`Config/FilterPlugin.ini` still excludes legacy redirectors, obsolete packages and
the old external actors/objects below `Maps/ThirdPersonMap`.

The plugin's item table uses the engine cube for its placeholder mesh. The save
and load pages use `Data/Localization/ST_InvenzaMenuPages` instead of the frontend
project's text table. The existing on-screen captions are preserved.

## UI authoring and localization

Runtime C++ does not construct widget trees. The character exchange window uses
`UI/Inventory/Character/WBP_CharacterInventoryExchange`. Storage headers use
`UI/Inventory/Workspace/WBP_StorageInventoryRow`, and controls-help rows use
`UI/Controls/WBP_ControlsHelpRow`. These templates reuse the existing Core buttons
and labels. Their classes are editable in Invenza Inventory settings.

`Data/Localization/ST_InvenzaRuntimeTexts` contains interaction captions, controls
help labels, crafting status messages and other runtime UI text. Edit/localize
that String Table or select a replacement through **Runtime Text Table**. Existing
save-system texts remain in `ST_InvenzaSaveTexts`; the two save/load page titles
and Back caption use `ST_InvenzaMenuPages`. Blueprint-authored captions remain
editable in their owning Blueprint. Do not put translated prose in runtime C++.

UI font/color defaults are in `Config/DefaultInventorySystemInvenzaPlugin.ini` and exposed in Project Settings.
Per-widget editable styles still take precedence when overridden in Blueprint.
Character bag/equipment templates use soft class references and resolve them when
access begins. Optional placeholder meshes are configurable there too. They reference plugin or Engine content; projects supply their own
character mesh and definitions as needed.

## Verification

The descriptor currently declares UE 5.6 / Win64 only. See
[Supported targets and module dependencies](SupportedTargets.md) for the build
matrix and public/private dependency boundaries.

Build the plugin with UAT `BuildPlugin -StrictIncludes` for each supported target.
This disables PCH and Unity Build, exposing missing direct includes. Test the
packaged result in a project that has no demo content or demo configuration.
Verify settings, gameplay tags, runtime UI and loading of all shipped assets.

Windows validation does not establish Linux or macOS compatibility. Validate those
platforms separately before advertising support. The included example contains
Epic template dependencies migrated to the plugin mount point; their existing
notices and provenance remain applicable.

## Blueprint-only packaging in UE 5.6

The plugin declares `EnabledByDefault=false`; explicitly enable it in the host's
`.uproject` or Plugins browser. This is intentional: UE 5.6 must recognize that
its prebuilt generic UnrealGame does not contain this project's code plugin and
create a temporary game target when packaging a Blueprint-only host. A successful
cook without that target does not prove that the packaged module can load.
A compatible C++ compiler is still needed to build that target. A project with
its own C++ game module links the plugin through its normal build.
