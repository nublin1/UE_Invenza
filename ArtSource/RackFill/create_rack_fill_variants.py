#  Nublin Studio 2026 All Rights Reserved.

"""Derive partial shelf meshes from the original full mesh, preserving its frame and collision."""

from pathlib import Path
import json
import bpy
from mathutils import Vector

OUT = Path('G:/UEProjects/UE_Invenza/ArtSource/RackFill')
scene = bpy.data.scenes.get('RackFillVariants')
if scene is None:
    scene = bpy.data.scenes.new('RackFillVariants')
    bpy.context.window.scene = scene
    scene.unit_settings.system = 'METRIC'
    scene.unit_settings.scale_length = 1.
    bpy.ops.import_scene.fbx(filepath=str(OUT/'Rack_Filled_Source.fbx'),use_image_search=False)
else:
    bpy.context.window.scene = scene
source = scene.objects['Rack_Filled_UE']
collision_source = scene.objects['UCX_Rack_Filled_UE']
mesh = source.data
cardboard = next(i for i,m in enumerate(mesh.materials) if m and m.name == 'M_Cardboard')
box_faces = {p.index for p in mesh.polygons if p.material_index == cardboard}
neighbors = {}
for index in box_faces:
    for vi in mesh.polygons[index].vertices:
        key = tuple(round(c,6) for c in mesh.vertices[vi].co)
        neighbors.setdefault(key,set()).add(index)
groups = []
remaining = set(box_faces)
while remaining:
    stack = [remaining.pop()]
    group = set(stack)
    while stack:
        index = stack.pop()
        for vi in mesh.polygons[index].vertices:
            key = tuple(round(c,6) for c in mesh.vertices[vi].co)
            for candidate in neighbors[key]:
                if candidate in remaining:
                    remaining.remove(candidate)
                    group.add(candidate)
                    stack.append(candidate)
    coords = [mesh.vertices[vi].co for index in group for vi in mesh.polygons[index].vertices]
    center = [sum(v[k] for v in coords)/len(coords) for k in range(3)]
    groups.append((center,group))
groups.sort(key=lambda g:(round(g[0][2],2),g[0][0],g[0][1]))
assert len(groups) == 15, 'Source shelf geometry changed; inspect connected boxes.'
report = []
for display_index, (percent,count) in enumerate([(25,4),(50,8),(75,11)]):
    name = 'SM_Rack_Fill_'+str(percent)
    assert not scene.objects.get(name), 'Variant already exists; inspect before regenerating.'
    selected_faces = {p.index for p in mesh.polygons if p.material_index != cardboard}
    for _,group in groups[:count]: selected_faces.update(group)
    selected_faces = sorted(selected_faces)
    used = sorted({vi for index in selected_faces for vi in mesh.polygons[index].vertices})
    remap = {old:new for new,old in enumerate(used)}
    data = bpy.data.meshes.new(name)
    data.from_pydata([mesh.vertices[vi].co for vi in used],[],
        [tuple(remap[vi] for vi in mesh.polygons[index].vertices) for index in selected_faces])
    for mat in mesh.materials: data.materials.append(mat)
    for poly,index in zip(data.polygons,selected_faces):
        poly.material_index = mesh.polygons[index].material_index
    for uv in mesh.uv_layers:
        layer = data.uv_layers.new(name=uv.name)
        for poly,index in zip(data.polygons,selected_faces):
            for new_loop,old_loop in zip(poly.loop_indices,mesh.polygons[index].loop_indices):
                layer.data[new_loop].uv = uv.data[old_loop].uv
    data.update()
    obj = bpy.data.objects.new(name,data)
    scene.collection.objects.link(obj)
    obj.matrix_world = source.matrix_world.copy()
    hull = bpy.data.objects.new('UCX_'+name+'_00',collision_source.data.copy())
    scene.collection.objects.link(hull)
    hull.matrix_world = collision_source.matrix_world.copy()
    hull.hide_render = True
    for existing in scene.objects: existing.select_set(False)
    obj.select_set(True)
    hull.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,
        object_types={'MESH'},axis_forward='-Y',axis_up='Z',bake_anim=False,add_leaf_bones=False,use_triangles=True)
    hull.hide_set(True)
    hull.select_set(False)
    data.calc_loop_triangles()
    report.append(dict(name=name,percent=percent,boxes=count,triangles=len(data.loop_triangles),dimensions_m=list(obj.dimensions)))
    obj.location.x += display_index*4.
    hull.location.x += display_index*4.
source.location.x += 12.
collision_source.hide_render = True
collision_source.hide_set(True)
(OUT/'variants.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(OUT/'RackFillVariants.blend'),{scene})
print(json.dumps(report))
