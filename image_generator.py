from ast import If
from tkinter import Scale
from PIL import Image
import numpy as np

def interpolate(x1, x2, w, scale):
    t = (w % scale) / scale
    return (x2 - x1) * (6 * t**5 - 15 * t**4 + 10 * t**3) + x1

def dot_product(a, b):
    return a[0] * b[0] + a[1] * b[1]

def perlinnoise(size, scale):
    global largest_value
    interpolated_map = np.zeros((size[0] * scale, size[1] * scale))
    vectors = []
    for i in range(size[0]):
        for j in range(size[1]):
            rand = np.random.rand() * 2 * np.pi
            vectors.append(np.array([np.cos(rand), np.sin(rand)]))
    for i in range(size[1] * scale):
        for j in range(size[0] * scale):
            dot_products = []
            for m in range(4):
                corner_vector_x = np.floor(i / scale) + (m % 2)
                corner_vector_y = np.floor(j / scale) + int(m / 2)
                x = i - corner_vector_x * scale
                y = j - corner_vector_y * scale
                if corner_vector_x >= size[0]:
                    corner_vector_x = 0
                if corner_vector_y >= size[1]:
                    corner_vector_y = 0
                corner_vector = vectors[int(corner_vector_x + corner_vector_y * size[0])]
                distance_vector = np.array([x, y])
                dot_products.append(dot_product(corner_vector, distance_vector))
            x1 = interpolate(dot_products[0], dot_products[1], i, scale)
            x2 = interpolate(dot_products[2], dot_products[3], i, scale)
            interpolated_map[i][j] = (interpolate(x1, x2, j, scale))
    return (interpolated_map + 11) / 22 * 255

def generatenoise(size, startingscale, octaves, persistance):
    global valuelist
    map = np.zeros((size[0] * startingscale, size[1] * startingscale))
    layers = []
    maxvalue = 0
    for i in range(octaves):
        layers.append(perlinnoise((size[0] * 2**i, size[1] * 2**i), startingscale // 2**i))
        maxvalue += 1 / persistance**i
    
    for i in range(size[0] * startingscale):
        for j in range(size[1] * startingscale):
            for k in range(octaves):
                map[i][j] += layers[k][i][j] / persistance**k
    map *= 255 / maxvalue

    return map

class rng:
    def __init__(self, seed, factor=297, digits=3):
        self.startingseed = seed
        self.seed = seed
        self.factor = factor
        self.digits = digits
    
    def __call__(self):
        self.seed = (self.seed * self.factor) - np.floor(self.seed * self.factor)
        return round(self.seed, self.digits)
    
    def input(self, seed, factor, digits=None):
        if digits == None:
            digits = self.digits
        result = (seed * factor) - np.floor(seed * factor)
        return round(result, digits)

class noisefactory:
    def __init__(self, seed, scale, starting_size, octaves, persistance, torus=True):
        self.starting_size = starting_size
        self.scale = scale
        self.vectors = {}
        self.rng = rng(seed)
        self.torus = torus
        self.octaves = octaves
        self.persistance = persistance
        self.maxvalue = 0
        for i in range(octaves):
            self.maxvalue += 1 / persistance**i
    
    def __call__(self, x, y):
        returnvalue = 0
        for j in range(self.octaves):
            dot_products = []
            for i in range(4):
                vp_x = np.floor(x / (self.scale / 2**j)) + (i % 2)
                vp_y = np.floor(y / (self.scale / 2**j)) + np.floor(i / 2)
                v_x = vp_x
                v_y = vp_y
                if self.torus:
                    while v_x >= size[0] * 2**j:
                        v_x -= size[0] * 2**j
                    while v_y >= size[1] * 2**j:
                        v_y -= size[1] * 2**j
                    """while v_x > size * 2**j:
                        v_x - size * 2**j
                    while v_y > size * 2**j:
                        v_y - size * 2**j"""
                v_key = "{}x{}".format(v_x, v_y)
                if v_key not in self.vectors:
                    v_angle = self.rng() * 2 * np.pi
                    self.vectors[v_key] = np.array([np.cos(v_angle), np.sin(v_angle)])
                active_vector = self.vectors[v_key]
                dv_x = x - vp_x * (self.scale / 2**j)
                dv_y = y - vp_y * (self.scale / 2**j)
                distance_vector = np.array([dv_x, dv_y])
                dot_products.append(dot_product(distance_vector, active_vector))
            #i_x = np.floor(x % self.scale) / self.scale
            #i_y = np.floor(x % self.scale) / self.scale
            x1 = interpolate(dot_products[0], dot_products[1], x, self.scale / 2**j)
            x2 = interpolate(dot_products[2], dot_products[3], x, self.scale / 2**j)
            returnvalue += (interpolate(x1, x2, y, self.scale / 2**j) + 11) / 22 / self.persistance**j
        return returnvalue * (255 / self.maxvalue)

class valuenoisefactory:
    def __init__(self, seed, scale, starting_size, octaves, persistance, torus=True):
        self.starting_size = starting_size
        self.scale = scale
        self.values = {}
        self.rng = rng(seed)
        self.torus = torus
        self.octaves = octaves
        self.persistance = persistance
        self.maxvalue = 0
        for i in range(octaves):
            self.maxvalue += 1 / persistance**i
    
    def __call__(self, x, y):
        returnvalue = 0
        for j in range(self.octaves):
            values = []
            for i in range(4):
                v_x = np.floor(x / (self.scale / 2**j)) + (i % 2)
                v_y = np.floor(y / (self.scale / 2**j)) + np.floor(i / 2)
                if self.torus:
                    while v_x >= size[0] * 2**j:
                        v_x -= size[0] * 2**j
                    while v_y >= size[1] * 2**j:
                        v_y -= size[1] * 2**j
                v_key = "{}x{}".format(v_x, v_y)
                if v_key not in self.values:
                    v_value = (self.rng() - 0.5) * 2
                    self.values[v_key] = v_value
                values.append(self.values[v_key])
            x1 = interpolate(values[0], values[1], x, self.scale / 2**j)
            x2 = interpolate(values[2], values[3], x, self.scale / 2**j)
            returnvalue += (interpolate(x1, x2, y, self.scale / 2**j) * 0.5 + 0.5) * 255 / self.persistance**j
        return returnvalue / self.maxvalue

size = [8, 8]
scale = 32
levels = 6
# perlin noise works best with a persistance of 1, value noise works best with a persistance of 2 or 3
factory = valuenoisefactory(0.5623, scale, size, 4, 3)
noise = np.zeros([size[0] * scale, size[1] * scale])
for i in range(size[0] * scale):
    for j in range(size[1] * scale):
        noise[j][i] = factory(i, j)
        """value = factory(i, j)
        noise[j][i] = np.floor(value / 255 * levels) / levels * 255"""

img = Image.fromarray(noise)
img.show()