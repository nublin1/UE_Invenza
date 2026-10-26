# Content provenance

This inventory records creation evidence; it does not grant a new license or
claim an independent legal clearance. The purchaser's license is the license
attached to the published product. Existing Epic copyright notices are retained.

## Project-created content

The owner confirmed on 22 October 2026 that the questioned chest, shelf and icon
assets were generated for this project. Their old import paths alone are not
proof of third-party authorship. The source inventory records that confirmation
separately from reproducible creation evidence; it does not invent a purchased
asset-pack license or a CC0/public-domain attribution.

Blender source/scripts retained in the Example Project include the storage
platform, small boxes, storage access desk, bulk-ore mesh and rack fill variants.
Several older meshes reference the project's previous Codex/Blender output folder.
Those older creation sessions are not archived in this plugin distribution.
`ContentProvenance.csv` lists the shipped visual assets and available evidence.

The labeled storage boxes and tied rope bundle use ordinary Blender geometry.
Their retained sources are `ArtSource/StorageLabels/create_storage_labels.py`
and `StorageLabelsAndRope.blend` in the working/Example Project sources.
`Scripts/Storage/create_labeled_storage_and_recipes.py` records the Unreal
material setup. Label material instances reuse the corresponding item icons;
the upstream icon provenance still applies. The herbal-extract and iron-ingot
icons were rendered from Blender geometry with `create_craft_icons.py` and
`CraftExampleIcons.blend`. These source files are evidence retained outside the
standalone plugin, rather than additional runtime dependencies.
`T_RT_Character` is an in-editor TextureRenderTarget2D resource, not an imported
image. These assets are recorded individually in the CSV inventory.

## Generative imagery

The owner's confirmation does not identify the exact image-generation service or
which individual icons used it. Before publishing, record those details and set
the listing's Created With AI information to match the included visual content.
AI-assisted scripting of ordinary Blender geometry and generative raster imagery
are recorded as different production methods; neither is inferred from a filename.
No Fab listing settings have been changed by this preparation task.

OpenAI's terms describe input/output ownership subject to their stated conditions:
[OpenAI Terms of Use](https://openai.com/policies/row-terms-of-use/).
Fab exposes separate AI content preferences and listing metadata:
[Fab documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/fab-window-in-unreal-engine).
These links are reference information, not an assertion that every historical
asset was produced with a particular service.

## Engine and example content

The standalone module references Engine fonts/basic shapes supplied by the
customer's Unreal installation. It does not bundle copies of those engine files.
The plugin now includes Epic template character/animation and Level Prototyping
dependencies used by `Map_Ex`, migrated from the Example Project into the plugin
mount point. Their source package paths are recorded in `ContentProvenance.csv`.
Retain their existing notices and follow the applicable Unreal terms. Project-
created materials do not replace the provenance of any textures they reference.

## Remaining owner confirmation

Keep the generation/source record for visual assets whose creation method is not
archived, and complete the Fab AI declaration before submission. The preparation
report distinguishes owner-confirmed origin from locally reproducible sources.
