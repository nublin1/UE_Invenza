import bpy
import bmesh
from pathlib import Path
from mathutils import Vector, Quaternion

OUT = Path(__file__).resolve().parent
scene = bpy.context.scene
# This script is run only in an isolated --factory-startup background process.
for obj in list(bpy.data.objects):
    bpy.data.objects.remove(obj, do_unlink=True)
scene.unit_settings.scale_length = 1.0

mesh = bpy.data.meshes.new('SM_StorageGridPlatform')
bm = bmesh.new()
bmesh.ops.create_cube(bm, size=1.0)
for v in bm.verts:
    v.co.z = v.co.z * 0.03 + 0.015
bmesh.ops.bevel(bm, geom=list(bm.edges), offset=0.002, segments=1)
bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))
bm.to_mesh(mesh)
bm.free()
mesh.update()
obj = bpy.data.objects.new('SM_StorageGridPlatform', mesh)
scene.collection.objects.link(obj)
bpy.context.view_layer.objects.active = obj
obj.select_set(True)

mat = bpy.data.materials.new('M_StorageGrid_Gray')
mat.use_nodes = True
mat.diffuse_color = (0.32, 0.32, 0.32, 1.0)
shader = next(n for n in mat.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
shader.inputs['Base Color'].default_value = mat.diffuse_color
shader.inputs['Roughness'].default_value = 0.78
shader.inputs['Metallic'].default_value = 0.0
mesh.materials.append(mat)

# A unique UV island for each face, with padding; no texture is required.
uv = mesh.uv_layers.new(name='UVMap')
for poly in mesh.polygons:
    col, row = poly.index % 6, poly.index // 6
    normal = poly.normal
    axes = sorted(range(3), key=lambda i: abs(normal[i]))[:2]
    coords = [mesh.vertices[mesh.loops[i].vertex_index].co for i in poly.loop_indices]
    lo = [min(v[a] for v in coords) for a in axes]
    hi = [max(v[a] for v in coords) for a in axes]
    for li, v in zip(poly.loop_indices, coords):
        xy = [(v[a]-lo[j])/max(hi[j]-lo[j], 1e-8) for j,a in enumerate(axes)]
        uv.data[li].uv = ((col+0.08+xy[0]*0.84)/6, (row+0.08+xy[1]*0.84)/6)

for screen in bpy.data.screens:
    for area in screen.areas:
        if area.type == 'VIEW_3D':
            space = area.spaces.active
            space.region_3d.view_location = Vector((0, 0, 0.015))
            space.region_3d.view_distance = 1.8
            space.region_3d.view_rotation = Vector((1.2,-1.5,1.1)).to_track_quat('Z','Y')

bpy.ops.export_scene.fbx(filepath=str(OUT/'SM_StorageGridPlatform.fbx'),
    use_selection=True, object_types={'MESH'}, apply_unit_scale=True,
    apply_scale_options='FBX_SCALE_UNITS', axis_forward='-Y', axis_up='Z',
    use_mesh_modifiers=True, mesh_smooth_type='FACE', bake_anim=False)
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'SM_StorageGridPlatform.blend'))
mesh.calc_loop_triangles()
print('ASSET:', tuple(obj.dimensions), 'triangles:', len(mesh.loop_triangles), 'pivot:', tuple(obj.location))

# Render a preview after saving: camera and lights are not in the asset file.
cam_data = bpy.data.cameras.new('PreviewCamera')
cam = bpy.data.objects.new('PreviewCamera', cam_data)
scene.collection.objects.link(cam)
cam.location = (1.3,-1.6,1.25)
cam.rotation_euler = (Vector((0,0,0.015))-cam.location).to_track_quat('-Z','Y').to_euler()
scene.camera = cam
cam_data.lens = 48
for name, pos, energy, size in [('Key',(0,-1,3),220,3),('Fill',(-2,1,2),110,2)]:
    data = bpy.data.lights.new(name, 'AREA')
    light = bpy.data.objects.new(name, data)
    scene.collection.objects.link(light)
    light.location = pos
    light.rotation_euler = (-light.location).to_track_quat('-Z','Y').to_euler()
    data.energy = energy
    data.size = size
scene.world.color = (0.18,0.18,0.18)
scene.render.resolution_x = 900
scene.render.resolution_y = 700
scene.render.resolution_percentage = 100
scene.render.image_settings.file_format = 'PNG'
scene.render.filepath = str(OUT/'preview.png')
bpy.ops.render.render(write_still=True)
