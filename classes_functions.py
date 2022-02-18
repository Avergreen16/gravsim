import pygame
import numpy as np

G = 1#6.675 * 10**-11

class gravobject:
    counter = 0

    def __init__(self, mass, rval, starting_coordinates, starting_vel, color, r_or_d="radius", stabilized = False):
        self.mass = mass
        if r_or_d == "radius":
            self.radius = rval
        elif r_or_d == "density":
            self.radius = np.cbrt((3*mass*5.97237*10**24)/(4*np.pi*(rval * 1000))) / 6371000
        self.velocity = np.array([float(starting_vel[0]), float(starting_vel[1])])
        self.coordinates = np.array([float(starting_coordinates[0]), float(starting_coordinates[1])])
        self.color = color
        self.acceleration = np.array([0.0, 0.0])
        self.id = gravobject.counter
        self.stabilized = stabilized
        gravobject.counter += 1
    
    def render(self, window, scale, view_offset):
        pygame.draw.circle(window, self.color, (self.coordinates[0] * scale - view_offset[0] * scale  + window.get_width() / 2, self.coordinates[1] * scale - view_offset[1] * scale + window.get_height() / 2), self.radius * scale)

# F = ma
# a = F/m
def gravinteract(object_list):
    archive = []
    for object1 in object_list:
        for object2 in object_list:
            if (object1.id, object2.id) not in archive and (object2.id, object1.id) not in archive and not object1 == object2:
                dif_x = object1.coordinates[0] - object2.coordinates[0]
                dif_y = object1.coordinates[1] - object2.coordinates[1]
                dif_hyp = np.sqrt(dif_x**2 + dif_y**2)
                dif_angle = np.arctan2(dif_x, dif_y)
                gravitational_force = (object1.mass * object2.mass) / dif_hyp**2 * G
                accel1 = gravitational_force / object1.mass
                accel2 = gravitational_force / object2.mass
                object1.acceleration += np.array([np.sin(dif_angle), np.cos(dif_angle)]) * -accel1
                object2.acceleration += np.array([np.sin(dif_angle), np.cos(dif_angle)]) * accel2
                #np.sqrt(object1.acceleration[1]**2 + object1.acceleration[1]**2))
                archive.append((object1.id, object2.id))

def apply_acceleration(object_list, delta, framerate=60):
    for object in object_list:
        if object.stabilized == False:
            object.velocity += object.acceleration
            object.coordinates += object.velocity * (delta / (1000 / framerate))
            object.acceleration = np.array([0.0, 0.0])
