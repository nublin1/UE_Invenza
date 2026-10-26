# Nublin Studio 2026 All Rights Reserved.
"""Render original simple ingot and extract icons; no downloaded content."""
import bpy
from pathlib import Path
from mathutils import Vector
OUT=Path('G:/UEProjects/UE_Invenza/ArtSource/StorageLabels')
previous=bpy.context.window.scene
scene=bpy.data.scenes.new('InvenzaCraftExampleIcons')
bpy.context.window.scene=scene
scene.render.engine='CYCLES'
scene.cycles.samples=24
scene.render.resolution_x=512
scene.render.resolution_y=512
scene.render.resolution_percentage=100
scene.render.film_transparent=True
scene.render.image_settings.file_format='PNG'
scene.render.image_settings.color_mode='RGBA'
scene.world=bpy.data.worlds.new('InvenzaCraftIconsWorld')
scene.world.color=(0.25,0.25,0.25)


def mat(name,color,metal=0):
    m=bpy.data.materials.new(name)
    m.use_nodes=True
    bs=m.node_tree.nodes.get('Principled BSDF')
    bs.inputs['Base Color'].default_value=(*color,1)
    bs.inputs['Metallic'].default_value=metal
    bs.inputs['Roughness'].default_value=0.42
    return m

iron=mat('InvenzaIconIron',(0.30,0.34,0.38),0.65)
glass=mat('InvenzaIconExtract',(0.10,0.30,0.07))
cork=mat('InvenzaIconCork',(0.30,0.16,0.06))
label=mat('InvenzaIconBottleLabel',(0.72,0.65,0.42))
cam_data=bpy.data.cameras.new('InvenzaCraftIconCamera')
cam=bpy.data.objects.new(cam_data.name,cam_data)
scene.collection.objects.link(cam)
scene.camera=cam
cam_data.type='ORTHO'
cam_data.ortho_scale=0.40
cam.location=(0.6,-0.9,0.65)
cam.rotation_euler=(Vector((0,0,0.07))-cam.location).to_track_quat('-Z','Y').to_euler()
for pos,energy,size in [((0.1,-0.5,1),70,1),((-0.6,0.4,0.7),40,1)]:
    data=bpy.data.lights.new('InvenzaCraftIconLight','AREA')
    light=bpy.data.objects.new(data.name,data)
    scene.collection.objects.link(light)
    light.location=pos
    light.rotation_euler=(-light.location).to_track_quat('-Z','Y').to_euler()
    data.energy=energy
    data.size=size
verts=[(-.15,-.055,0),(.15,-.055,0),(.15,.055,0),(-.15,.055,0),(-.12,-.045,.06),(.12,-.045,.06),(.12,.045,.06),(-.12,.045,.06)]
faces=[(3,2,1,0),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7),(4,5,6,7)]
mesh=bpy.data.meshes.new('InvenzaIconIronIngot')
mesh.from_pydata(verts,[],faces)
mesh.materials.append(iron)
obj=bpy.data.objects.new(mesh.name,mesh)
scene.collection.objects.link(obj)
bevel=obj.modifiers.new('SmallEdgeBevel','BEVEL')
bevel.width=0.004
bevel.segments=1
scene.render.filepath=str(OUT/'T_IronIngot.png')
bpy.ops.render.render(write_still=True)
obj.hide_render=True
parts=[]
for radius,depth,z,material in [(.055,.12,.07,glass),(.025,.06,.15,glass),(.029,.025,.187,cork)]:
    bpy.ops.mesh.primitive_cylinder_add(vertices=12,radius=radius,depth=depth,location=(0,0,z))
    bottle=bpy.context.object
    bottle.name='InvenzaIconHerbalExtract'
    bottle.data.materials.append(material)
    parts.append(bottle)
bpy.ops.mesh.primitive_cube_add(size=1,location=(0,-.054,.075))
plate=bpy.context.object
plate.name='InvenzaIconExtractLabel'
plate.dimensions=(.068,.003,.05)
plate.data.materials.append(label)
parts.append(plate)
cam_data.ortho_scale=.28
cam.location=(.28,-.6,.3)
cam.rotation_euler=(Vector((0,0,.10))-cam.location).to_track_quat('-Z','Y').to_euler()
scene.render.filepath=str(OUT/'T_HerbalExtract.png')
bpy.ops.render.render(write_still=True)
bpy.data.libraries.write(str(OUT/'CraftExampleIcons.blend'),{scene},fake_user=True)
bpy.context.window.scene=previous
print('INVENZA_CRAFT_ICONS_CREATED')
