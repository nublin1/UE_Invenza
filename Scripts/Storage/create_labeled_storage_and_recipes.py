# Nublin Studio 2026 All Rights Reserved.
"""Create item-labeled storage meshes and connected crafting examples.

Saves only the named data tables, new assets and two recipe-list Blueprints.
Run in the full editor with -ExecutePythonScript; no level or UI saves.
"""
import unreal as u
from pathlib import Path
import json
import re
import shutil
import hashlib

ROOT='/InventorySystemInvenzaPlugin/'
PROJECT=Path(u.Paths.project_dir()).resolve()
OUT=PROJECT/'Saved/StorageLabels'
ART=PROJECT/'ArtSource/StorageLabels'
DEST=ROOT+'Meshes/StorageLabels'
ICON_DEST=ROOT+'Textures/CraftExamples'
TABLE_PATH=ROOT+'Data/DT_ItemData'
RECIPE_PATH=ROOT+'Data/DT_Recipe'
TEXT_PATH=ROOT+'Data/Localization/ST_CraftingExamples'
BLUEPRINTS=[ROOT+'Blueprints/Examples/BP_Character',ROOT+'Blueprints/Examples/CraftStaton/BP_CraftStation']
OUT.mkdir(parents=True,exist_ok=True)
TOOLS=u.AssetToolsHelpers.get_asset_tools()
EDIT=u.InvenzaDataTableEditingLibrary
WIDGET_EDIT=u.InvenzaWidgetEditingLibrary


def preserve(package):
    relative=package.removeprefix(ROOT)+'.uasset'
    source=PROJECT/'Plugins/InventorySystemInvenzaPlugin/Content'/relative
    destination=OUT/'Before'/relative
    destination.parent.mkdir(parents=True,exist_ok=True)
    if source.exists() and not destination.exists():
        shutil.copy2(source,destination)


def save(asset):
    assert u.EditorAssetLibrary.save_loaded_asset(asset,only_if_is_dirty=False),asset.get_path_name()


def load_or_create(name,dest,cls,factory):
    path=dest+'/'+name
    return u.load_asset(path) if u.EditorAssetLibrary.does_asset_exist(path) else TOOLS.create_asset(name,dest,cls,factory)


def import_asset(name,source,dest,mesh=False):
    path=dest+'/'+name
    if u.EditorAssetLibrary.does_asset_exist(path):
        return u.load_asset(path)
    task=u.AssetImportTask()
    task.set_editor_properties(dict(filename=str(source),destination_path=dest,destination_name=name,automated=True,replace_existing=False,save=False))
    if mesh:
        options=u.FbxImportUI()
        options.set_editor_properties(dict(import_mesh=True,import_as_skeletal=False,import_materials=False,import_textures=False,automated_import_should_detect_type=False,mesh_type_to_import=u.FBXImportType.FBXIT_STATIC_MESH))
        options.static_mesh_import_data.set_editor_properties(dict(combine_meshes=True,auto_generate_collision=False,generate_lightmap_u_vs=True,convert_scene=True,convert_scene_unit=True,import_uniform_scale=1.0,normal_import_method=u.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS))
        task.set_editor_properties(dict(factory=u.FbxFactory(),options=options))
    TOOLS.import_asset_tasks([task])
    asset=u.load_asset(path)
    assert asset,path
    return asset


def solid(name,color,roughness=0.8):
    asset=load_or_create(name,DEST,u.Material,u.MaterialFactoryNew())
    if not u.MaterialEditingLibrary.get_num_material_expressions(asset):
        node=u.MaterialEditingLibrary.create_material_expression(asset,u.MaterialExpressionConstant3Vector,-300,0)
        node.set_editor_property('constant',u.LinearColor(*color,1))
        u.MaterialEditingLibrary.connect_material_property(node,'',u.MaterialProperty.MP_BASE_COLOR)
        node=u.MaterialEditingLibrary.create_material_expression(asset,u.MaterialExpressionConstant,-300,140)
        node.set_editor_property('r',roughness)
        u.MaterialEditingLibrary.connect_material_property(node,'',u.MaterialProperty.MP_ROUGHNESS)
    asset.set_editor_property('used_with_instanced_static_meshes',True)
    u.MaterialEditingLibrary.recompile_material(asset)
    save(asset)
    return asset


try:
    assert u.get_editor_subsystem(u.StaticMeshEditorSubsystem), 'Use the full editor, not a commandlet.'
    for path in [TABLE_PATH,RECIPE_PATH]+BLUEPRINTS:
        preserve(path)
    items=u.load_asset(TABLE_PATH)
    recipes=u.load_asset(RECIPE_PATH)
    item_before=json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(items))
    recipe_before=json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(recipes))
    if not (OUT/'items-before.json').exists():
        (OUT/'items-before.json').write_text(json.dumps(item_before,indent=2),encoding='utf-8')
    if not (OUT/'recipes-before.json').exists():
        (OUT/'recipes-before.json').write_text(json.dumps(recipe_before,indent=2),encoding='utf-8')
    original_bow=next(row for row in recipe_before if row['Name']=='Bow')
    text_table=load_or_create('ST_CraftingExamples',ROOT+'Data/Localization',u.StringTable,u.StringTableFactory())

    def text(key,source):
        WIDGET_EDIT.set_string_table_entry(text_table,key,source)
        return WIDGET_EDIT.string_table_text(text_table,key)

    icons={}
    for name in ['IronIngot','HerbalExtract']:
        icon=import_asset('T_'+name,ART/('T_'+name+'.png'),ICON_DEST)
        icon.set_editor_property('lod_group',u.TextureGroup.TEXTUREGROUP_UI)
        save(icon)
        icons[name]=icon
    body=solid('M_StorageCardboard',(0.34,0.20,0.095))
    fiber=solid('M_StorageRopeFiber',(0.46,0.31,0.14))
    tie=solid('M_StorageRopeTie',(0.13,0.075,0.027))
    label_material=load_or_create('M_StorageItemLabel',DEST,u.Material,u.MaterialFactoryNew())
    if not u.MaterialEditingLibrary.get_num_material_expressions(label_material):
        texture=u.MaterialEditingLibrary.create_material_expression(label_material,u.MaterialExpressionTextureSampleParameter2D,-500,0)
        texture.set_editor_property('parameter_name','ItemIcon')
        texture.set_editor_property('texture',icons['IronIngot'])
        paper=u.MaterialEditingLibrary.create_material_expression(label_material,u.MaterialExpressionConstant3Vector,-500,220)
        paper.set_editor_property('constant',u.LinearColor(0.78,0.72,0.56,1))
        lerp=u.MaterialEditingLibrary.create_material_expression(label_material,u.MaterialExpressionLinearInterpolate,-160,0)
        u.MaterialEditingLibrary.connect_material_expressions(paper,'',lerp,'A')
        u.MaterialEditingLibrary.connect_material_expressions(texture,'RGB',lerp,'B')
        u.MaterialEditingLibrary.connect_material_expressions(texture,'A',lerp,'Alpha')
        u.MaterialEditingLibrary.connect_material_property(lerp,'',u.MaterialProperty.MP_BASE_COLOR)
        rough=u.MaterialEditingLibrary.create_material_expression(label_material,u.MaterialExpressionConstant,-160,160)
        rough.set_editor_property('r',0.9)
        u.MaterialEditingLibrary.connect_material_property(rough,'',u.MaterialProperty.MP_ROUGHNESS)
    label_material.set_editor_property('used_with_instanced_static_meshes',True)
    u.MaterialEditingLibrary.recompile_material(label_material)
    save(label_material)
    bases={}
    for name in ['Small','Medium','Large']:
        mesh=import_asset('SM_StorageLabelBox'+name,ART/('SM_StorageLabelBox'+name+'.fbx'),DEST,True)
        assert len(mesh.get_editor_property('static_materials'))==2,mesh
        mesh.set_material(0,body)
        mesh.set_material(1,label_material)
        save(mesh)
        bases[name]=mesh
    rope=import_asset('SM_StorageRopeBundle',ART/'SM_StorageRopeBundle.fbx',DEST,True)
    rope.set_material(0,fiber)
    rope.set_material(1,tie)
    save(rope)
    rows={r['Name']:r for r in item_before}

    def make_labeled_box(row_name,icon,size):
        name='SM_StorageBox_'+row_name
        path=DEST+'/'+name
        mesh=u.load_asset(path) if u.EditorAssetLibrary.does_asset_exist(path) else u.EditorAssetLibrary.duplicate_asset(bases[size].get_path_name(),path)
        assert mesh,path
        mi=load_or_create('MI_StorageLabel_'+row_name,DEST,u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew())
        u.MaterialEditingLibrary.set_material_instance_parent(mi,label_material)
        u.MaterialEditingLibrary.set_material_instance_texture_parameter_value(mi,'ItemIcon',icon)
        save(mi)
        mesh.set_material(0,body)
        mesh.set_material(1,mi)
        save(mesh)
        return mesh

    def store_row(name,row):
        assert EDIT.set_row_from_text(items,name,row.export_text()),name

    assigned=[]
    for name,entry in rows.items():
        if name in ['Log','IronOreBulk']:
            continue
        row=u.ItemData()
        metadata=u.ItemMetaData()
        sanitized=entry['ItemMetaData'].replace('StorageMethod=(INVALID)','StorageMethod=SingleMesh')
        assert metadata.import_text(sanitized),name
        row.set_editor_property('id',entry['ID'])
        storage=metadata.get_editor_property('item_storage_data')
        if name=='Rope':
            mesh=rope
            method=u.StorageMethod.SINGLE_MESH
        else:
            icon=metadata.get_editor_property('item_asset_data').get_editor_property('icon')
            size='Small' if name in ['Paper','Money'] else ('Large' if name in ['Armor_1','Sword_1','Bow','Board'] else 'Medium')
            mesh=make_labeled_box(name,icon,size)
            method=u.StorageMethod.SMALL_BOXES if name in ['Paper','Money'] else u.StorageMethod.SINGLE_MESH
        storage.set_editor_property('storage_method',method)
        storage.set_editor_property('storage_mesh',mesh)
        metadata.set_editor_property('item_storage_data',storage)
        row.set_editor_property('item_meta_data',metadata)
        store_row(name,row)
        assigned.append({'item':name,'mesh':mesh.get_path_name(),'method':str(method)})

    for name,display,description,weight,max_stack,price in [
        ('IronIngot','Iron ingot','Smelted from iron ore. Used for the sword and armor crafting examples.',1.5,20,8),
        ('HerbalExtract','Herbal extract','A concentrated plant reagent used by the potion crafting examples.',0.25,10,3)]:
        mesh=make_labeled_box(name,icons[name],'Medium')
        row=u.ItemData()
        row.set_editor_property('id',name)
        meta=u.ItemMetaData()
        assets=meta.get_editor_property('item_asset_data')
        assets.set_editor_property('icon',icons[name])
        assets.set_editor_property('mesh',mesh)
        meta.set_editor_property('item_asset_data',assets)
        names=meta.get_editor_property('item_text_data')
        names.set_editor_property('display_name',text('Item.'+name+'.Name',display))
        names.set_editor_property('item_description',text('Item.'+name+'.Description',description))
        meta.set_editor_property('item_text_data',names)
        stats=meta.get_editor_property('item_numeratic_data')
        stats.set_editor_properties(dict(weight=weight,inventory_horizontal_slots=1,inventory_vertical_slots=1,max_stack_size_in_character=max_stack))
        meta.set_editor_property('item_numeratic_data',stats)
        trade=meta.get_editor_property('item_trade_data')
        trade.set_editor_property('base_price',float(price))
        meta.set_editor_property('item_trade_data',trade)
        storage=meta.get_editor_property('item_storage_data')
        storage.set_editor_properties(dict(storage_method=u.StorageMethod.SINGLE_MESH,storage_mesh=mesh))
        meta.set_editor_property('item_storage_data',storage)
        tag=u.GameplayTag()
        tag.import_text('(TagName="Item.Category.Ingredient")')
        meta.set_editor_property('item_category',tag)
        row.set_editor_property('item_meta_data',meta)
        store_row(name,row)
        assigned.append({'item':name,'mesh':mesh.get_path_name(),'method':'SingleMesh'})
    save(items)

    # Preserve the authored Bow row exactly; remove only the known invalid placeholders.
    removed=[]
    for entry in recipe_before:
        if entry['Name'].startswith('TestRecipe'):
            assert all('DataTable=None' in v and 'Amount=0' in v for v in entry['OutputItems']),entry['Name']
            removed.append(entry['Name'])
    assert EDIT.remove_rows(recipes,removed)==len(removed)
    after_items={r['Name']:r for r in json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(items))}

    def handle(name):
        return f"(DataTable=\"/Script/Engine.DataTable'{TABLE_PATH}.DT_ItemData'\",RowName=\"{name}\")"

    def required(name,amount,alternatives=()):
        data='(Item='+handle(name)+',Quantity='+str(amount)
        if alternatives:
            data+=',Alternatives=('+','.join('(Item='+handle(n)+',Quantity='+str(q)+')' for n,q in alternatives)+')'
        return data+')'

    def output(name,amount):
        return '(Item='+handle(name)+',Amount='+str(amount)+')'

    recipe_plan=[
        ('SawBoards','Saw boards','Turn one log into four boards.', [('Log',1,[])], [('Board',4)],3),
        ('SpinThread','Spin thread','Process vines into crafting thread.', [('Vine',3,[])],[('Thread',2)],2),
        ('BraidRope','Braid rope','Braid two units of thread into a rope.', [('Thread',2,[])],[('Rope',1)],3),
        ('AssembleBow','Assemble bow','A chained recipe: first make boards and rope, then assemble the bow.', [('Board',2,[]),('Rope',1,[])],[('Bow',1)],5),
        ('FieldBow','Field bow','Choose one wood option and one binding option.', [('Board',2,[('Log',1)]),('Rope',1,[('Thread',2),('Vine',3)])],[('Bow',1)],6),
        ('SmeltIron','Smelt iron ingot','Refine the existing bulk iron ore into an ingot.', [('IronOreBulk',3,[])],[('IronIngot',1)],6),
        ('CraftSword','Forge sword','Combine smelted iron with a wooden grip and a thread binding.', [('IronIngot',2,[]),('Board',1,[]),('Thread',1,[])],[('Sword_1',1)],8),
        ('CraftArmor','Make armor','Use iron ingots and rope bindings to make armor.', [('IronIngot',4,[]),('Rope',2,[])],[('Armor_1',1)],10),
        ('ExtractHerbs','Prepare herbal extract','A game alchemy example: extract two portions from vines using yellow potion.', [('Vine',3,[]),('YellowPotion',1,[])],[('HerbalExtract',2)],3),
        ('BrewRedPotion','Brew red potion','Filter herbal extract through paper to make two red potions.', [('HerbalExtract',1,[]),('Paper',1,[])],[('RedPotion',2)],4),
        ('BrewYellowPotion','Brew yellow potion','A game alchemy example: combine herbal extract with mineral reagent from iron ore.', [('HerbalExtract',2,[]),('IronOreBulk',1,[])],[('YellowPotion',2)],4),
    ]
    added=[]
    for name,display,description,requirements,outputs,volume in recipe_plan:
        recipe=u.ItemRecipeRow()
        recipe.set_editor_property('id',name)
        recipe.set_editor_property('display_name',text('Recipe.'+name+'.Name',display))
        recipe.set_editor_property('description',text('Recipe.'+name+'.Description',description))
        if outputs[0][0] in icons:
            icon=icons[outputs[0][0]]
        else:
            m=u.ItemMetaData()
            m.import_text(after_items[outputs[0][0]]['ItemMetaData'])
            icon=m.get_editor_property('item_asset_data').get_editor_property('icon')
        recipe.set_editor_property('recipe_icon',icon)
        recipe.set_editor_property('craft_volume',float(volume))
        values=[]
        for n,q,alternatives in requirements:
            value=u.RecipeItemRequirement()
            assert value.import_text(required(n,q,alternatives)),required(n,q,alternatives)
            values.append(value)
        recipe.set_editor_property('required_items',values)
        values=[]
        for n,q in outputs:
            value=u.InitItemsEntry()
            assert value.import_text(output(n,q)),output(n,q)
            values.append(value)
        recipe.set_editor_property('output_items',values)
        assert EDIT.set_row_from_text(recipes,name,recipe.export_text()),name
        added.append(name)
    save(text_table)
    save(recipes)
    recipe_after=json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(recipes))
    assert next(row for row in recipe_after if row['Name']=='Bow')==original_bow,'Authored Bow recipe changed'
    assert len(recipe_after)==12,len(recipe_after)
    assert len({row['ID'] for row in recipe_after})==12
    names=['Bow']+added
    handles=[]
    for name in names:
        row=u.DataTableRowHandle()
        row.set_editor_property('data_table',recipes)
        row.set_editor_property('row_name',name)
        handles.append(row)
    sub=u.get_engine_subsystem(u.SubobjectDataSubsystem)
    bp_changes=[]
    for path in BLUEPRINTS:
        bp=u.load_asset(path)
        assert bp,path
        bp.modify()
        visited=set()
        for h in sub.k2_gather_subobject_data_for_blueprint(bp):
            component=u.SubobjectDataBlueprintFunctionLibrary.get_object(u.SubobjectDataBlueprintFunctionLibrary.get_data(h))
            if isinstance(component,u.CraftingComponent) and component.get_path_name() not in visited:
                visited.add(component.get_path_name())
                component.modify()
                component.set_editor_property('starting_recipes',handles)
                bp_changes.append({'blueprint':path,'component':component.get_name(),'recipes':names})
        assert visited,path
        # Compile only the two explicitly task-scoped recipe-list owners.
        u.BlueprintEditorLibrary.compile_blueprint(bp)
        save(bp)
    # Verify other item data and the existing log/ore definitions were preserved.
    final_items={r['Name']:r for r in json.loads(u.DataTableFunctionLibrary.export_data_table_to_json_string(items))}
    for name,before in rows.items():
        after=final_items[name]
        assert before['ID']==after['ID'],name
        if name in ['Log','IronOreBulk']:
            assert before==after,name
        else:
            strip=lambda s: re.sub(r'ItemStorageData=\([^)]*\),ItemCategory=','ItemCategory=',s.replace('StorageMethod=(INVALID)','StorageMethod=SingleMesh'))
            assert strip(before['ItemMetaData'])==strip(after['ItemMetaData']),name
    (OUT/'items-after.json').write_text(json.dumps(list(final_items.values()),indent=2),encoding='utf-8')
    (OUT/'recipes-after.json').write_text(json.dumps(recipe_after,indent=2),encoding='utf-8')
    (OUT/'result.json').write_text(json.dumps({'assigned_meshes':assigned,'removed_placeholders':removed,'added_recipes':added,'preserved_bow':True,'recipe_count':len(recipe_after),'blueprint_changes':bp_changes,'no_level_or_ui_assets_saved':True},indent=2),encoding='utf-8')
    print('INVENZA_STORAGE_LABELS_AND_RECIPES_SAVED')
finally:
    u.SystemLibrary.quit_editor()
