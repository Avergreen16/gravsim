import pygame
import numpy as np

# get orbital velocity of barycenter, then add it to each object's velocity to avoid binary planets breaking up.
# have binary systems' objects orbit the barycenter with a mass equation to determine the apparent mass to decrease instability of orbits

G = 1#6.675 * 10**-11
force_stable_orbits = False
separate_barycenters_for_calculations = True
render_barycenters = False
default_r_or_d = "radius"

barycenters = []
objects = []

def orb_vel(parent_mass, orbital_radius):
    return np.sqrt((G * parent_mass) / orbital_radius)

class gravobject:
    counter = 0

    def __init__(self, mass, rval, coordinates, velocity, color, parent=None, r_or_d=default_r_or_d, stabilized=False):
        global objects
        objects.append(self)
        
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
    def __init__(self, object1_mass, object1_rval, object1_color, object2_mass, object2_rval, object2_color, coordinates, separation, velocity, parent=None, stabilized=False, object1_r_or_d=default_r_or_d, object2_r_or_d=default_r_or_d):
        global objects
        barycenters.append(self)

        self.parent = parent
        self.stabilized = stabilized
        self.mass = object1_mass + object2_mass
        self.coordinates = np.array([float(coordinates[0]), float(coordinates[1])])
        starting_velocity = np.array([float(velocity[0]), float(velocity[1])])
        distance = separation / (1 + (object1_mass / object2_mass))
        object1_coordinates = self.coordinates + np.array([0.0, distance])
        object2_coordinates = self.coordinates - np.array([0.0, separation - distance]) 

        object1_velocity = np.array([np.sqrt((G * object2_mass**2) / (separation * self.mass)), 0.0])
        object2_velocity = np.array([-np.sqrt((G * object1_mass**2) / (separation * self.mass)), 0.0])

        self.objects = [gravobject(object1_mass, object1_rval, object1_coordinates, object1_velocity, object1_color, r_or_d=object1_r_or_d), gravobject(object2_mass, object2_rval, object2_coordinates, object2_velocity,object2_color, r_or_d=object2_r_or_d)]
        self.objects[0].parent = parent
        self.objects[1].parent = parent

        self.velocity = (self.objects[0].mass * self.objects[0].velocity + self.objects[1].mass * self.objects[1].velocity) / (self.objects[0].mass + self.objects[1].mass)
        self.acceleration = (self.objects[0].mass * self.objects[0].acceleration + self.objects[1].mass * self.objects[1].acceleration) / (self.objects[0].mass + self.objects[1].mass)
        self.objects[0].velocity -= self.velocity - starting_velocity
        self.objects[1].velocity -= self.velocity - starting_velocity
        self.velocity = (self.objects[0].mass * self.objects[0].velocity + self.objects[1].mass * self.objects[1].velocity) / (self.objects[0].mass + self.objects[1].mass)

    def update_attributes(self):
        self.velocity = (self.objects[0].mass * self.objects[0].velocity + self.objects[1].mass * self.objects[1].velocity) / (self.objects[0].mass + self.objects[1].mass)
        self.coordinates = (self.objects[0].mass * self.objects[0].coordinates + self.objects[1].mass * self.objects[1].coordinates) / (self.objects[0].mass + self.objects[1].mass)
        self.acceleration = (self.objects[0].mass * self.objects[0].acceleration + self.objects[1].mass * self.objects[1].acceleration) / (self.objects[0].mass + self.objects[1].mass)
        if self.stabilized:
            self.objects[0].velocity -= self.velocity
            self.objects[1].velocity -= self.velocity

    def render(self, window, scale, view_offset):
        pygame.draw.line(window, (127, 127, 127), ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 - 4, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 - 4), ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 + 4, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 + 4))
        pygame.draw.line(window, (127, 127, 127), ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 + 4, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 - 4), ((self.coordinates[0] - view_offset[0]) * scale + window.get_width() / 2 - 4, (self.coordinates[1] - view_offset[1]) * scale + window.get_height() / 2 + 4))

def add_satellite(parent, mass, rval, color, orbital_radius, r_or_d=default_r_or_d):
    global objects
    gravobject(mass, rval, [parent.coordinates[0], parent.coordinates[1] - orbital_radius], [parent.velocity[0] + orb_vel(parent.mass, orbital_radius), parent.velocity[1]], color, r_or_d=r_or_d, parent=parent)

def add_satellite_barycenter(parent, mass1, rval1, color1, mass2, rval2, color2, barycenter_orbital_radius, binary_orbital_radius, r_or_d1=default_r_or_d, r_or_d2=default_r_or_d):
    global objects
    barycenter(mass1, rval1, color1, mass2, rval2, color2, [parent.coordinates[0], parent.coordinates[1] - barycenter_orbital_radius], binary_orbital_radius, [parent.velocity[0] + orb_vel(parent.mass, barycenter_orbital_radius), parent.velocity[1]], parent=parent, object1_r_or_d=r_or_d1, object2_r_or_d=r_or_d2)

def gravequation(object1, object2, objects_affected="both"):
    dif_x = object1.coordinates[0] - object2.coordinates[0]
    dif_y = object1.coordinates[1] - object2.coordinates[1]
    dif_hyp = np.sqrt(dif_x**2 + dif_y**2)
    dif_angle = np.arctan2(dif_y, dif_x)
    gravitational_force = (object1.mass * object2.mass) / dif_hyp**2 * G
    if objects_affected == "object1" or objects_affected == "both":
        accel1 = gravitational_force / object1.mass
        object1.acceleration += np.array([np.cos(dif_angle), np.sin(dif_angle)]) * -accel1
    if objects_affected == "object2" or objects_affected == "both":
        accel2 = gravitational_force / object2.mass
        object2.acceleration += np.array([np.cos(dif_angle), np.sin(dif_angle)]) * accel2
    

def gravinteract():
    global objects
    global barycenter
    archive = []
    for object1 in objects:
        if force_stable_orbits:
            if not object1.parent == None:
                recursive_parent = True
                object2 = object1.parent
                while recursive_parent:
                    if isinstance(object2, barycenter):
                        if separate_barycenters_for_calculations:
                            for parent in object2.objects:
                                gravequation(object1, parent, objects_affected="object1")
                        else:
                            gravequation(object1, object2, objects_affected="object1")
                    else:
                        gravequation(object1, object2, objects_affected="object1")
                    if object2.parent == None:
                        recursive_parent = False
                    else:
                        object2 = object2.parent
        else:
            for object2 in objects:
                if (object1.id, object2.id) not in archive and (object2.id, object1.id) not in archive and not object1 == object2:
                    gravequation(object1, object2)
                    archive.append((object1.id, object2.id))
    
    if force_stable_orbits:
        for _barycenter in barycenters:
                gravequation(_barycenter.objects[0], _barycenter.objects[1])

def apply_acceleration(delta, framerate=60):
    global objects
    for object in objects:
        if object.stabilized == False:
            object.velocity += object.acceleration
            object.coordinates += object.velocity * (delta / (1000 / framerate))
            object.acceleration = np.array([0.0, 0.0])

def gravloop(window, scale, view_offset, delta, framerate=60):
    global objects
    gravinteract()
    apply_acceleration(delta, framerate=framerate)
    for _barycenter in barycenters:
        _barycenter.update_attributes()

    if render_barycenters:
        for _barycenter in barycenters:
            _barycenter.render(window, scale, view_offset)
    
    for object in objects.__reversed__():
        object.render(window, scale, view_offset)