#  Nublin Studio 2026 All Rights Reserved.

"""Create a separate Blender scene and export a simple warehouse clerk stand.

Existing Blender scenes/files are preserved. The export pivot matches the old
100 cm placeholder cube: 50 cm above the base. Model front faces Blender -Y.
"""

from pathlib import Path
import bpy
import bmesh
import math
import json
from mathutils import Vector, Matrix, Euler

OUT = Path('G:/UEProjects/UE_Invenza/ArtSource/StorageAccess')
OUT.mkdir(parents=True, exist_ok=True)
assert not bpy.data.scenes.get('StorageAccessDesk'), 'A desk scene already exists; inspect it before regenerating.'
scene = bpy.data.scenes.new('StorageAccessDesk')
bpy.context.window.scene = scene
scene.unit_settings.system = 'METRIC'
scene.unit_settings.scale_length = 1.


def material(name, color, metallic=0., roughness=.8):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    mat.diffuse_color = (*color, 1.)
    bsdf = next(n for n in mat.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
    bsdf.inputs['Base Color'].default_value = (*color, 1.)
    bsdf.inputs['Metallic'].default_value = metallic
    bsdf.inputs['Roughness'].default_value = roughness
    return mat


materials = [
    material('M_StorageDesk_Wood', (.24, .12, .052)),
    material('M_StorageDesk_Frame', (.085, .039, .018)),
    material('M_StorageDesk_Paper', (.68, .57, .37), roughness=.95),
    material('M_StorageDesk_Metal', (.20, .16, .095), metallic=.65, roughness=.62),
    material('M_StorageDesk_Ledger', (.12, .032, .022), roughness=.9),
]
vertices, faces, indices = [], [], []


def append_geometry(verts, polys, mat_index, bevel=0.):
    data = bpy.data.meshes.new('DeskPart')
    data.from_pydata(verts, [], polys)
    bm = bmesh.new()
    bm.from_mesh(data)
    bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))
    if bevel:
        bmesh.ops.bevel(bm, geom=list(bm.edges), offset=bevel, segments=1, profile=.5)
        bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))
    bm.to_mesh(data)
    bm.free()
    offset = len(vertices)
    vertices.extend(tuple(v.co) for v in data.vertices)
    faces.extend(tuple(offset + i for i in p.vertices) for p in data.polygons)
    indices.extend([mat_index] * len(data.polygons))
    bpy.data.meshes.remove(data)


def box(center, size, mat_index=0, rotation=None, bevel=.002):
    x, y, z = (s / 2 for s in size)
    verts = [Vector(v) for v in [(-x,-y,-z),(x,-y,-z),(x,y,-z),(-x,y,-z),
                                (-x,-y,z),(x,-y,z),(x,y,z),(-x,y,z)]]
    if rotation is not None:
        verts = [rotation @ v for v in verts]
    verts = [v + Vector(center) for v in verts]
    append_geometry(verts, [(0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)], mat_index, bevel)


def ring(center, major=.026, minor=.004, mat_index=3, count=16, sides=6):
    verts, polys = [], []
    for i in range(count):
        a = 2 * math.pi * i / count
        for j in range(sides):
            b = 2 * math.pi * j / sides
            r = major + minor * math.cos(b)
            verts.append((center[0] + r * math.cos(a), center[1] + minor * math.sin(b), center[2] + r * math.sin(a)))
    for i in range(count):
        for j in range(sides):
            polys.append((i*sides+j, ((i+1)%count)*sides+j, ((i+1)%count)*sides+(j+1)%sides, i*sides+(j+1)%sides))
    append_geometry(verts, polys, mat_index)


# Substantial base, solid cabinet and contrasting structural rails.
box((0,0,.045), (.72,.47,.09), 1, bevel=.004)
box((0,0,.115), (.68,.43,.05), 0)
box((0,.193,.52), (.65,.035,.76), 1)
for x in (-.312,.312):
    box((x,0,.52), (.036,.40,.76), 0)
    for y in (-.193,.193):
        box((x,y,.52), (.052,.052,.79), 1, bevel=.003)
for z in (.17,.85):
    box((0,-.207,z), (.65,.045,.064), 1)
for x in (-.153,.153):
    box((x,-.205,.51), (.287,.035,.60), 0)
    # A recessed wooden center gives the doors shape without ornament.
    box((x,-.226,.51), (.227,.014,.48), 1, bevel=.002)
    box((x,-.236,.51), (.203,.012,.455), 0, bevel=.002)
for x in (-.035,.035):
    box((x,-.251,.64), (.012,.018,.065), 3, bevel=.002)

# Wedge below the tilted writing surface.
append_geometry([(-.34,-.22,.89),(.34,-.22,.89),(.34,.22,.89),(-.34,.22,.89),
                 (-.34,-.22,.964),(.34,-.22,.964),(.34,.22,1.044),(-.34,.22,1.044)],
                [(0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)], 1, .002)
angle = math.atan(.10 / .55)
slope = Matrix.Rotation(angle, 3, 'X')
top_center = Vector((0,0,1.015))
box(top_center, (.80,.55,.035), 0, slope, .004)


def on_top(local_center, size, material_index, bevel=.001):
    box(top_center + slope @ Vector(local_center), size, material_index, slope, bevel)


# A low lip and open accounting ledger, built entirely from flat colors.
on_top((0,-.251,.031), (.74,.018,.028), 1)
on_top((-.075,.015,.026), (.34,.28,.014), 4)
on_top((-.075,.015,.041), (.315,.253,.020), 2)
on_top((-.075,.015,.053), (.008,.253,.004), 1, .0005)
for x in (-.156,.008):
    for y in (-.062,-.028,.006,.040,.074):
        on_top((x,y,.052), (.105,.0025,.0015), 0, 0)

# A ring and two short keys, hanging just below the writing surface.
box((.23,-.247,.84), (.017,.022,.032), 3)
ring((.23,-.264,.805), .024, .0035)
for x, z, length in [(.214,.747,.072),(.246,.755,.058)]:
    box((x,-.268,z), (.009,.009,length), 3, bevel=.001)
    box((x+.012,-.268,z-length/2+.007), (.027,.009,.012), 3, bevel=.001)
    box((x+.021,-.268,z-length/2+.016), (.008,.009,.015), 3, bevel=.001)

# One render mesh, preserving material sections. Origin is compatible with the old cube.
pivot = Vector((0,0,.5))
mesh = bpy.data.meshes.new('SM_StorageAccessDesk')
mesh.from_pydata([Vector(v)-pivot for v in vertices], [], faces)
for mat in materials:
    mesh.materials.append(mat)
for poly, index in zip(mesh.polygons, indices):
    poly.material_index = index
mesh.update()
uvs = mesh.uv_layers.new(name='UVMap')
for poly in mesh.polygons:
    axis = max(range(3), key=lambda i: abs(poly.normal[i]))
    axes = [i for i in range(3) if i != axis]
    for li in poly.loop_indices:
        co = mesh.vertices[mesh.loops[li].vertex_index].co
        uvs.data[li].uv = ((co[axes[0]]+1.)/2., (co[axes[1]]+1.)/2.)
desk = bpy.data.objects.new('SM_StorageAccessDesk', mesh)
scene.collection.objects.link(desk)

# A single simple convex hull covers the stand for movement and interaction traces.
collision = bpy.data.meshes.new('UCX_SM_StorageAccessDesk_00')
collision.from_pydata([(-.4,-.275,-.5),(.4,-.275,-.5),(.4,.275,-.5),(-.4,.275,-.5),
                      (-.4,-.275,.49),(.4,-.275,.49),(.4,.275,.59),(-.4,.275,.59)], [],
                     [(0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)])
collision.update()
hull = bpy.data.objects.new('UCX_SM_StorageAccessDesk_00', collision)
scene.collection.objects.link(hull)
hull.hide_render = True

bpy.context.view_layer.objects.active = desk
desk.select_set(True)
hull.select_set(True)
bpy.ops.export_scene.fbx(filepath=str(OUT/'SM_StorageAccessDesk.fbx'), use_selection=True,
                         object_types={'MESH'}, add_leaf_bones=False, bake_anim=False,
                         axis_forward='-Y', axis_up='Z', use_triangles=True)
hull.hide_set(True)
hull.select_set(False)

for area in bpy.context.screen.areas:
    if area.type == 'VIEW_3D':
        region = area.spaces.active.region_3d
        region.view_location = Vector((0,0,.05))
        region.view_distance = 2.3
        region.view_rotation = Euler((math.radians(65),0,math.radians(30))).to_quaternion()

mesh.calc_loop_triangles()
report = {'mesh':desk.name,'dimensions_m':list(desk.dimensions),'triangles':len(mesh.loop_triangles),
          'materials':[m.name for m in materials],'pivot_height_cm':50,'collision_hulls':1}
(OUT/'mesh-info.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(OUT/'StorageAccessDesk.blend'), {scene})
print(json.dumps(report))
