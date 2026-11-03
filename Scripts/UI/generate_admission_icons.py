# Nublin Studio 2026 All Rights Reserved.
"""Rasterize the three simple geometric admission symbols for import into Unreal."""
from pathlib import Path
from PIL import Image, ImageDraw

OUTPUT = Path(__file__).parent / 'Data/AdmissionIcons'
OUTPUT.mkdir(parents=True, exist_ok=True)
SCALE = 4
SIZE = 48


def symbol(name, points, color):
    image = Image.new('RGBA', (SIZE * SCALE, SIZE * SCALE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    for path in points:
        scaled = [(int(x * SCALE), int(y * SCALE)) for x, y in path]
        draw.line(scaled, fill=color, width=5 * SCALE, joint='curve')
        for x, y in scaled:
            radius = 2.5 * SCALE
            draw.ellipse((int(x - radius), int(y - radius), int(x + radius), int(y + radius)), fill=color)
    image.resize((SIZE, SIZE), Image.Resampling.LANCZOS).save(OUTPUT / (name + '.png'))


symbol('T_AdmissionAllowed', [[(9, 25), (19, 35), (39, 12)]], (40, 210, 65, 255))
symbol('T_AdmissionBlocked', [[(12, 12), (36, 36)], [(36, 12), (12, 36)]], (230, 48, 43, 255))
symbol('T_AdmissionPartial', [[(8, 27), (12, 22), (17, 20), (23, 22), (29, 26), (35, 27), (40, 22)]], (234, 188, 35, 255))
