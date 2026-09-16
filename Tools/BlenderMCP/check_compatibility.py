import bpy
import importlib.util
import sys
spec = importlib.util.spec_from_file_location('blender_mcp', r'G:\UEProjects\UE_Invenza\Tools\BlenderMCP\blender_mcp.py')
addon = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = addon
spec.loader.exec_module(addon)
addon.register()
assert hasattr(bpy.types.Scene, 'blendermcp_port')
print('MCP_COMPATIBILITY_OK', bpy.app.version_string, addon.bl_info['version'])
addon.unregister()
