import pygame
import numpy as np

# integrate the vectors to get velocity

G = 1#6.675 * 10**-11
force_stable_orbits = False
separate_barycenters_for_calculations = True

barycenters = []

class gravobject:
    counter = 0

    def __init__(self, mass, rval, starting_coordinates, starting_vel, color, parent=None, r_or_d="radius", stabilized = False):
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
        self.parent = parent
    
    def render(self, window, scale, view_offset):
        if self.radius * scale < 1:
            pygame.draw.line(window, self.color, ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 - 8, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 - 8), ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 + 8, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 + 8))
            pygame.draw.line(window, self.color, ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 + 8, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 - 8), ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 - 8, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 + 8))
        pygame.draw.circle(window, self.color, ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2), self.radius * scale)

class barycenter:
    def __init__(self, object1, object2):
        self.objects = [object1, object2]
        self.mass = self.objects[0].mass + self.objects[1].mass
        if self.objects[0].parent == self.objects[1]:
            self.parent = self.objects[1].parent
        elif self.objects[1].parent == self.objects[0]:
            self.parent = self.objects[0].parent
        barycenters.append(self)
        distance = np.sqrt((self.objects[1].coordinates[0] - self.objects[0].coordinates[0])**2 + (self.objects[1].coordinates[1] - self.objects[0].coordinates[1])**2) / (1 + (self.objects[0].mass / self.objects[1].mass))
        angle = np.arctan2(self.objects[1].coordinates[1] - self.objects[0].coordinates[1], self.objects[1].coordinates[0] - self.objects[0].coordinates[0])
        self.coordinates = np.array([self.objects[0].coordinates[0] + distance * np.cos(angle), self.objects[0].coordinates[1] + distance * np.sin(angle)])
        print((self.objects[0].mass * self.objects[0].coordinates + self.objects[1].mass * self.objects[1].coordinates) / (self.objects[0].mass + self.objects[1].mass))
        self.velocity = (self.objects[0].velocity + self.objects[1].velocity / 2)
    
    def velocity(self):
        self.velocity = self.objects[0].velocity + self.objects[1].velocity
    
    def update_coordinates(self):
        distance = np.sqrt((self.objects[1].coordinates[0] - self.objects[0].coordinates[0])**2 + (self.objects[1].coordinates[1] - self.objects[0].coordinates[1])**2) / (1 + (self.objects[0].mass / self.objects[1].mass))
        angle = np.arctan2(self.objects[1].coordinates[1] - self.objects[0].coordinates[1], self.objects[1].coordinates[0] - self.objects[0].coordinates[0])
        self.coordinates = np.array([self.objects[0].coordinates[0] + distance * np.cos(angle), self.objects[0].coordinates[1] + distance * np.sin(angle)])

# F = ma
# a = F/m
def gravinteract(object_list):
    archive = []
    for object1 in object_list:
        if force_stable_orbits:
            if not object1.parent == None:
                recursive_parent = True
                object2 = object1.parent
                while recursive_parent:
                    if isinstance(object2, barycenter):
                        for parent in object2.objects:
                            dif_x = object1.coordinates[0] - parent.coordinates[0]
                            dif_y = object1.coordinates[1] - parent.coordinates[1]
                            dif_hyp = np.sqrt(dif_x**2 + dif_y**2)
                            dif_angle = np.arctan2(dif_y, dif_x)
                            gravitational_force = (object1.mass * parent.mass) / dif_hyp**2 * G
                            accel1 = gravitational_force / object1.mass
                            accel2 = gravitational_force / parent.mass
                            if separate_barycenters_for_calculations:
                                object1.acceleration += np.array([np.cos(dif_angle), np.sin(dif_angle)]) * -accel1
                            
                            parent.acceleration += np.array([np.cos(dif_angle), np.sin(dif_angle)]) * accel2
                        if not separate_barycenters_for_calculations:
                            dif_x = object1.coordinates[0] - object2.coordinates[0]
                            dif_y = object1.coordinates[1] - object2.coordinates[1]
                            dif_hyp = np.sqrt(dif_x**2 + dif_y**2)
                            dif_angle = np.arctan2(dif_y, dif_x)
                            gravitational_force = (object1.mass * object2.mass) / dif_hyp**2 * G
                            accel1 = gravitational_force / object1.mass
                            object1.acceleration += np.array([np.cos(dif_angle), np.sin(dif_angle)]) * -accel1
                    else:
                        dif_x = object1.coordinates[0] - object2.coordinates[0]
                        dif_y = object1.coordinates[1] - object2.coordinates[1]
                        dif_hyp = np.sqrt(dif_x**2 + dif_y**2)
                        dif_angle = np.arctan2(dif_y, dif_x)
                        gravitational_force = (object1.mass * object2.mass) / dif_hyp**2 * G
                        accel1 = gravitational_force / object1.mass
                        accel2 = gravitational_force / object2.mass
                        object1.acceleration += np.array([np.cos(dif_angle), np.sin(dif_angle)]) * -accel1
                        object2.acceleration += np.array([np.cos(dif_angle), np.sin(dif_angle)]) * accel2
                    if object2.parent == None:
                        recursive_parent = False
                    else:
                        object2 = object2.parent
        else:
            for object2 in object_list:
                if (object1.id, object2.id) not in archive and (object2.id, object1.id) not in archive and not object1 == object2:
                    dif_x = object1.coordinates[0] - object2.coordinates[0]
                    dif_y = object1.coordinates[1] - object2.coordinates[1]
                    dif_hyp = np.sqrt(dif_x**2 + dif_y**2)
                    dif_angle = np.arctan2(dif_y, dif_x)
                    gravitational_force = (object1.mass * object2.mass) / dif_hyp**2 * G
                    accel1 = gravitational_force / object1.mass
                    accel2 = gravitational_force / object2.mass
                    object1.acceleration += np.array([np.cos(dif_angle), np.sin(dif_angle)]) * -accel1
                    object2.acceleration += np.array([np.cos(dif_angle), np.sin(dif_angle)]) * accel2

def apply_acceleration(object_list, delta, framerate=60):
    for object in object_list:
        if object.stabilized == False:
            object.velocity += object.acceleration
            object.coordinates += object.velocity * (delta / (1000 / framerate))
            object.acceleration = np.array([0.0, 0.0])
    
    for barycenter in barycenters:
        barycenter.update_coordinates()
    

