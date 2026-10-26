# Nublin Studio 2026 All Rights Reserved.
"""Create storage box bases and a five-coil rope bundle in a separate Blender scene."""
import bpy
import bmesh
import math
import json
from pathlib import Path
from mathutils import Vector

OUT=Path('G:/UEProjects/UE_Invenza/ArtSource/StorageLabels')
OUT.mkdir(parents=True,exist_ok=True)
previous_scene=bpy.context.window.scene
scene=bpy.data.scenes.new('InvenzaStorageLabels')
bpy.context.window.scene=scene
scene.unit_settings.system='METRIC'
scene.unit_settings.scale_length=1.0
scene.world=bpy.data.worlds.new('InvenzaStorageLabelsWorld')
scene.world.color=(0.1,0.1,0.1)


def material(name,color,roughness=0.8):
    m=bpy.data.materials.new(name)
    m.use_nodes=True
    m.diffuse_color=(*color,1)
    bs=m.node_tree.nodes.get('Principled BSDF')
    bs.inputs['Base Color'].default_value=m.diffuse_color
    bs.inputs['Roughness'].default_value=roughness
    return m

body=material('Invenza_StorageCardboard',(0.34,0.20,0.095))
label=material('Invenza_StorageLabel',(0.78,0.72,0.56))
rope=material('Invenza_RopeFiber',(0.46,0.31,0.14))
tie=material('Invenza_RopeTie',(0.13,0.075,0.027))
created=[]


def box(name,size):
    x,y,z=size
    mesh=bpy.data.meshes.new(name)
    bm=bmesh.new()
    bmesh.ops.create_cube(bm,size=1)
    for v in bm.verts:
        v.co.x*=x
        v.co.y*=y
        v.co.z=v.co.z*z+z*0.5
    bmesh.ops.bevel(bm,geom=list(bm.edges),offset=0.001,segments=1)
    bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))
    bm.to_mesh(mesh)
    bm.free()
    verts=[tuple(v.co) for v in mesh.vertices]
    faces=[tuple(poly.vertices) for poly in mesh.polygons]
    body_count=len(faces)
    side=min(x,y,z)*0.64
    h=side*0.5
    eps=0.0007
    plates=[
        [(-h,-y/2-eps,z/2-h),(h,-y/2-eps,z/2-h),(h,-y/2-eps,z/2+h),(-h,-y/2-eps,z/2+h)],
        [(h,y/2+eps,z/2-h),(-h,y/2+eps,z/2-h),(-h,y/2+eps,z/2+h),(h,y/2+eps,z/2+h)],
        [(x/2+eps,-h,z/2-h),(x/2+eps,h,z/2-h),(x/2+eps,h,z/2+h),(x/2+eps,-h,z/2+h)],
        [(-x/2-eps,h,z/2-h),(-x/2-eps,-h,z/2-h),(-x/2-eps,-h,z/2+h),(-x/2-eps,h,z/2+h)],
        [(-h,-h,z+eps),(h,-h,z+eps),(h,h,z+eps),(-h,h,z+eps)]
    ]
    for plate in plates:
        start=len(verts)
        verts.extend(plate)
        faces.append(tuple(range(start,start+4)))
    mesh.clear_geometry()
    mesh.from_pydata(verts,[],faces)
    mesh.update()
    mesh.materials.append(body)
    mesh.materials.append(label)
    uv=mesh.uv_layers.new(name='UVMap')
    for p in mesh.polygons:
        p.material_index=int(p.index>=body_count)
        if p.index>=body_count:
            for loop,xy in zip(p.loop_indices,[(0,0),(1,0),(1,1),(0,1)]):
                uv.data[loop].uv=xy
        else:
            axes=sorted(range(3),key=lambda a:abs(p.normal[a]))[:2]
            for loop in p.loop_indices:
                co=mesh.vertices[mesh.loops[loop].vertex_index].co
                uv.data[loop].uv=(co[axes[0]],co[axes[1]])
    obj=bpy.data.objects.new(name,mesh)
    scene.collection.objects.link(obj)
    created.append(obj)
    return obj


small=box('SM_StorageLabelBoxSmall',(0.20,0.20,0.25))
medium=box('SM_StorageLabelBoxMedium',(0.40,0.40,0.40))
large=box('SM_StorageLabelBoxLarge',(0.80,0.60,0.45))

# Five continuous low-poly coils; the holes and three turns remain visible.
vertices=[]
faces=[]
material_indices=[]


def tube(points,radius,sides=6,closed=False,material_index=0):
    start=len(vertices)
    for i,p in enumerate(points):
        prev=Vector(points[(i-1)%len(points)]) if closed or i else Vector(points[0])
        nex=Vector(points[(i+1)%len(points)]) if closed or i<len(points)-1 else Vector(points[-1])
        tangent=(nex-prev).normalized()
        helper=Vector((0,0,1)) if abs(tangent.z)<0.9 else Vector((0,1,0))
        u=tangent.cross(helper).normalized()
        v=tangent.cross(u).normalized()
        for j in range(sides):
            a=2*math.pi*j/sides
            vertices.append(tuple(Vector(p)+radius*(math.cos(a)*u+math.sin(a)*v)))
    segments=len(points) if closed else len(points)-1
    for i in range(segments):
        n=(i+1)%len(points)
        for j in range(sides):
            faces.append((start+i*sides+j,start+i*sides+(j+1)%sides,start+n*sides+(j+1)%sides,start+n*sides+j))
            material_indices.append(material_index)
    if not closed:
        faces.append(tuple(start+j for j in reversed(range(sides))))
        material_indices.append(material_index)
        faces.append(tuple(start+(len(points)-1)*sides+j for j in range(sides)))
        material_indices.append(material_index)


for cx,cz in [(-0.21,0.012),(0,0.012),(0.21,0.012),(-0.105,0.09),(0.105,0.09)]:
    points=[]
    for i in range(145):
        angle=6*math.pi*i/144
        points.append((cx+0.09*math.cos(angle),0.09*math.sin(angle),cz+0.021*i/48))
    tube(points,0.011)
# Two binding cords wrap the whole bundle, with short ends at the top.
for y in [-0.042,0.042]:
    path=[(-0.32,y,0.012),(-0.32,y,0.09),(-0.18,y,0.17),(0.18,y,0.17),(0.32,y,0.09),(0.32,y,0.012)]
    tube(path,0.007,closed=True,material_index=1)
tube([(-0.04,-0.05,0.18),(0,0,0.188),(0.05,0.07,0.18)],0.008,material_index=1)
mesh=bpy.data.meshes.new('SM_StorageRopeBundle')
mesh.from_pydata(vertices,[],faces)
mesh.materials.append(rope)
mesh.materials.append(tie)
mesh.update()
for p,i in zip(mesh.polygons,material_indices):
    p.material_index=i
    p.use_smooth=True
obj=bpy.data.objects.new('SM_StorageRopeBundle',mesh)
scene.collection.objects.link(obj)
created.append(obj)
min_z=min(v.co.z for v in mesh.vertices)
for v in mesh.vertices:
    v.co.z-=min_z

manifest=[]
for obj in created:
    for other in scene.objects:
        other.select_set(False)
    obj.select_set(True)
    bpy.context.view_layer.objects.active=obj
    bpy.ops.export_scene.fbx(filepath=str(OUT/(obj.name+'.fbx')),use_selection=True,object_types={'MESH'},apply_unit_scale=True,apply_scale_options='FBX_SCALE_UNITS',axis_forward='-Y',axis_up='Z',use_mesh_modifiers=True,mesh_smooth_type='FACE',bake_anim=False)
    obj.data.calc_loop_triangles()
    manifest.append({'name':obj.name,'dimensions_m':list(obj.dimensions),'triangles':len(obj.data.loop_triangles),'materials':[m.name for m in obj.data.materials]})
# Arrange only the preview; FBX pivots all remain at their individual bottom centers.
for i,obj in enumerate(created):
    obj.location=(i*0.95,0,0)
cam_data=bpy.data.cameras.new('InvenzaStoragePreviewCamera')
cam=bpy.data.objects.new(cam_data.name,cam_data)
scene.collection.objects.link(cam)
cam.location=(2.8,-3.8,3.0)
cam.rotation_euler=(Vector((1.4,0,0.15))-cam.location).to_track_quat('-Z','Y').to_euler()
scene.camera=cam
cam_data.type='ORTHO'
cam_data.ortho_scale=4.4
for name,pos,energy in [('Key',(1,-2,4),500),('Fill',(-1,1,3),300)]:
    data=bpy.data.lights.new('InvenzaStorage'+name,'AREA')
    light=bpy.data.objects.new(data.name,data)
    scene.collection.objects.link(light)
    light.location=pos
    light.rotation_euler=(Vector((1.3,0,0))-light.location).to_track_quat('-Z','Y').to_euler()
    data.energy=energy
    data.size=4
scene.render.engine='CYCLES'
scene.cycles.samples=16
scene.render.resolution_x=1280
scene.render.resolution_y=600
scene.render.resolution_percentage=100
scene.render.image_settings.file_format='PNG'
scene.render.filepath=str(OUT/'geometry-preview.png')
bpy.ops.render.render(write_still=True)
bpy.data.libraries.write(str(OUT/'StorageLabelsAndRope.blend'),{scene},fake_user=True)
(OUT/'geometry.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.context.window.scene=previous_scene
print('INVENZA_STORAGE_GEOMETRY_CREATED',manifest)
