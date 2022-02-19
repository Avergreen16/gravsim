import pygame
import numpy as np

# integrate the vectors to get velocity

G = 1#6.675 * 10**-11
force_stable_orbits = False
separate_barycenters_for_calculations = True
render_barycenters = False
default_r_or_d = "radius"

barycenters = []

def orb_vel(parent_mass, orbital_radius):
    return np.sqrt(G * parent_mass / orbital_radius)

class gravobject:
    counter = 0

    def __init__(self, mass, rval, coordinates, velocity, color, parent=None, r_or_d=default_r_or_d, stabilized=False):
        self.mass = mass
        if r_or_d == "radius":
            self.radius = rval
        elif r_or_d == "density":
            self.radius = np.cbrt((3*mass*5.97237*10**24)/(4*np.pi*(rval * 1000))) / 6371000
        self.velocity = np.array([float(velocity[0]), float(velocity[1])])
        self.coordinates = np.array([float(coordinates[0]), float(coordinates[1])])
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
    def __init__(self, object_list, object1_mass, object1_rval, object1_color, object2_mass, object2_rval, object2_color, coordinates, separation, velocity, parent=None, stabilized=False, object1_r_or_d=default_r_or_d, object2_r_or_d=default_r_or_d):
        barycenters.append(self)
        self.parent = parent
        self.stabilized = stabilized
        self.coordinates = np.array([float(coordinates[0]), float(coordinates[1])])
        self.velocity = np.array([float(velocity[0]), float(velocity[1])])
        distance = separation / (1 + (object1_mass / object2_mass))
        object1_coordinates = self.coordinates + np.array([0.0, distance])
        object2_coordinates = self.coordinates - np.array([0.0, separation - distance]) 

        object1_velocity = np.array([orb_vel(object2_mass, separation), 0])# + self.velocity
        object2_velocity = -(object1_velocity * object1_mass / object2_mass)#np.array([orb_vel(object1_mass, separation), 0])# + self.velocity

        self.objects = [gravobject(object1_mass, object1_rval, object1_coordinates, object1_velocity, object1_color, r_or_d=object1_r_or_d), gravobject(object2_mass, object2_rval, object2_coordinates, object2_velocity,object2_color, r_or_d=object2_r_or_d)]
        object_list += self.objects
        self.mass = self.objects[0].mass + self.objects[1].mass
        self.objects[0].parent = self.objects[1]
        self.objects[1].parent = self.objects[0]
        self.acceleration = (self.objects[0].mass * self.objects[0].acceleration + self.objects[1].mass * self.objects[1].acceleration) / (self.objects[0].mass + self.objects[1].mass)
        #self.objects[0].velocity -= velocity
        #self.objects[1].velocity -= velocity

    def update_attributes(self):
        self.velocity = (self.objects[0].mass * self.objects[0].velocity + self.objects[1].mass * self.objects[1].velocity) / (self.objects[0].mass + self.objects[1].mass)
        self.coordinates = (self.objects[0].mass * self.objects[0].coordinates + self.objects[1].mass * self.objects[1].coordinates) / (self.objects[0].mass + self.objects[1].mass)
        self.acceleration = (self.objects[0].mass * self.objects[0].acceleration + self.objects[1].mass * self.objects[1].acceleration) / (self.objects[0].mass + self.objects[1].mass)

    def render(self, window, scale, view_offset):
        pygame.draw.line(window, (127, 127, 127), ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 - 4, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 - 4), ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 + 4, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 + 4))
        pygame.draw.line(window, (127, 127, 127), ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 + 4, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 - 4), ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 - 4, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 + 4))
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

def gravloop(window, scale, view_offset, object_list, delta, framerate=60):
    gravinteract(object_list)
    apply_acceleration(object_list, delta, framerate=framerate)

    for barycenter in barycenters:
        barycenter.update_attributes()

    for object in object_list.__reversed__():
        object.render(window, scale, view_offset)
    
    if render_barycenters:
        for barycenter in barycenters:
            barycenter.render(window, scale, view_offset)
    

