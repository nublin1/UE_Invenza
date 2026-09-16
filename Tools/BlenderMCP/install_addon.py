import bpy
import shutil
from pathlib import Path
source = Path(r'G:\UEProjects\UE_Invenza\Tools\BlenderMCP\blender_mcp.py')
target = Path(bpy.utils.user_resource('SCRIPTS', path='addons', create=True)) / 'blender_mcp.py'
if target.exists():
    shutil.copy2(target, str(target) + '.before-mcp.bak')
shutil.copy2(source, target)
bpy.utils.refresh_script_paths()
bpy.ops.preferences.addon_enable(module='blender_mcp')
bpy.ops.wm.save_userpref()
print('INSTALLED_AND_ENABLED', target)
