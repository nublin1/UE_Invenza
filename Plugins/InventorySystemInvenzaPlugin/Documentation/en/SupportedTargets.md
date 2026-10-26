# Supported targets and module dependencies

This distribution declares **Unreal Engine 5.6 / Win64**. `EngineVersion` uses
`5.6.0` for the engine release line; validation uses the installed UE **5.6.1**.
The plugin descriptor's `SupportedTargetPlatforms` and the runtime module's
`PlatformAllowList` both contain only `Win64`. Linux and macOS require separate
build and runtime validation before adding them to either list.

| Target | Configuration |
|---|---|
| UnrealEditor / Win64 | Development |
| UnrealGame / Win64 | Development |
| UnrealGame / Win64 | Shipping |

The module is `Runtime` and loads at `Default`. The engine's **Enhanced Input**
plugin is its explicit, enabled plugin dependency. No demo-project or editor
module is a runtime dependency.

## Dependencies exposed to C++ consumers

| Public dependency | Why it is public |
|---|---|
| Core | Containers, strings, math and module interfaces |
| CoreUObject | UObject types, interfaces and reflection |
| Engine | Components, actors, world/subsystems and asset types |
| InputCore | Keys and input types used by public settings |
| UMG | User widgets, widget interfaces and component types |
| SlateCore | Public brushes, fonts, geometry and input event types |
| EnhancedInput | Public input actions and templated input-binding helpers |
| GameplayTags | Tags and containers in public inventory/crafting/equipment APIs |
| DeveloperSettings | Public settings classes |
| NetCore | Fast-array replication types and serializers |

The private dependencies are **Slate** (application/widget implementation),
**CoreOnline** (persistent player identity), and **Projects** (plugin discovery
and configuration directory registration). AI and JSON modules are not used
directly by Invenza and are not declared as its dependencies. The engine may
still load them for its own functionality.

When your module uses the public Invenza API, declare
`InventorySystemInvenzaPlugin` in its dependency list. Keep it public when your
own public headers expose Invenza types; otherwise keep it private. Declare any
additional engine modules that your own implementation uses directly.

## Packaging checks

Use UAT `BuildPlugin -StrictIncludes` to validate independent translation units
without Unity Build or precompiled headers. The repository helper
`Scripts/Packaging/Build-StandalonePlugin.ps1` rejects platforms and engine
release lines outside the descriptor before starting UAT. Passing compilation
does not establish runtime support on untested platforms.

The existing Fab product identifier is retained. `DocsURL`, `SupportURL` and
`CreatedByURL` are intentionally empty at the owner's request until real public
addresses are available. Bundled documentation is under `Documentation`.
