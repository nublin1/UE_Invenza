import asyncio
import os
from mcp import ClientSession, StdioServerParameters
from mcp.client.stdio import stdio_client
async def main():
    env = dict(os.environ, BLENDER_MCP_DISABLE_TELEMETRY='1', BLENDER_HOST='localhost', BLENDER_PORT='9876')
    params = StdioServerParameters(command=r'G:\UEProjects\UE_Invenza\Tools\BlenderMCP\.venv\Scripts\blender-mcp.exe', env=env)
    async with stdio_client(params) as (read, write):
        async with ClientSession(read, write) as session:
            result = await session.initialize()
            tools = await session.list_tools()
            print('MCP_OK', result.serverInfo.name, 'TOOLS', len(tools.tools))
            assert any(t.name == 'get_scene_info' for t in tools.tools)
asyncio.run(main())
