from PIL import Image
from PIL import ImageDraw
import numpy as np
import noise

#create perlin noise

size = (256, 512)
scale = 100
octaves = 5
persistence = 0.5
lacunarity = 2

fillpercentage = 30

map = np.zeros(size)

for i in range(size[0]):
    for j in range(size[1]):
        map[i][j] = (noise.pnoise2(i/scale, j/scale, octaves=octaves, persistence=persistence, lacunarity=lacunarity, repeatx=size[0]/scale, repeaty=size[0]/scale, base=0) + (fillpercentage / 100)) * 255

"""img = Image.new(mode="RGBA", size=(512, 512), color=(0, 0, 0, 0))

pencil = ImageDraw.Draw(img)
pencil.ellipse((1, 1, 512, 512), (0, 0, 0, 255))"""
img = Image.fromarray(map)

img.show()