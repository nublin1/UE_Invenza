"""Run with Blender --background --python. Generates only new BulkBins source assets."""
import bpy
import random
import math
from pathlib import Path
from mathutils import Vector

out = Path(__file__).resolve().parents[2] / 'ArtSource/StorageGrid/BulkBins'
out.mkdir(parents=True, exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene
scene.unit_settings.system = 'METRIC'
rng = random.Random(71839)
bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2, radius=0.13)
ore = bpy.context.object
ore.name = 'SM_BulkIronOre'
for v in ore.data.vertices:
    v.co *= rng.uniform(0.84, 1.14)
    v.co.y *= 0.84
    v.co.z *= 0.72
for name, color, metallic in [('OreStone', (0.19,0.225,0.255,1),0.12), ('OreVein', (0.36,0.21,0.105,1),0.42), ('OreHighlight',(0.29,0.315,0.33,1),0.25)]:
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = color
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get('Principled BSDF')
    bsdf.inputs['Base Color'].default_value = color
    bsdf.inputs['Metallic'].default_value = metallic
    bsdf.inputs['Roughness'].default_value = 0.8
    ore.data.materials.append(mat)
for p in ore.data.polygons:
    p.material_index = rng.choices([0,1,2],[7,2,1])[0]
bpy.ops.export_scene.fbx(filepath=str(out/'SM_BulkIronOre.fbx'), use_selection=True, object_types={'MESH'}, add_leaf_bones=False, bake_anim=False, axis_forward='-Y', axis_up='Z')

# Render a real mesh thumbnail for the inventory icon.
scene.render.engine = 'CYCLES'
scene.cycles.samples = 32
scene.render.resolution_x = 256
scene.render.resolution_y = 256
scene.render.resolution_percentage = 100
scene.render.film_transparent = True
scene.world = bpy.data.worlds.new('Studio')
scene.world.use_nodes = True
scene.world.node_tree.nodes['Background'].inputs[0].default_value = (0.2,0.22,0.26,1)
scene.world.node_tree.nodes['Background'].inputs[1].default_value = 0.6
for location, energy, size in [((0.3,-0.35,0.6),30,0.5),((-0.3,0.15,0.25),12,0.3)]:
    bpy.ops.object.light_add(type='AREA', location=location)
    light = bpy.context.object
    light.data.energy = energy
    light.data.shape = 'DISK'
    light.data.size = size
    light.rotation_euler = (-light.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=(0.34,-0.46,0.32))
camera = bpy.context.object
camera.rotation_euler = (-camera.location).to_track_quat('-Z','Y').to_euler()
camera.data.type = 'ORTHO'
camera.data.ortho_scale = 0.34
scene.camera = camera
scene.render.image_settings.file_format = 'PNG'
scene.render.filepath = str(out/'T_BulkIronOre.png')
bpy.ops.wm.save_as_mainfile(filepath=str(out/'BulkIronOre.blend'))
bpy.ops.render.render(write_still=True)
