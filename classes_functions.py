import numpy as np

G = 6.675 * 10**-11

class gravobject:
    def __init__(self, mass, rval, starting_coordinates, starting_vel, color, r_or_d = "radius"):
        self.mass = mass
        if r_or_d == "radius":
            self.radius = rval
        elif r_or_d == "density":
            self.radius = np.cbrt((3*mass)/(4*np.pi*rval))
        self.velocity = starting_vel
        self.coordinates = np.array([starting_coordinates[0], starting_coordinates[1]])
        self.color = color
        self.acceleration = np.array([0, 0])

# F = ma
# a = F/m
def gravinteract(object1, object2, delta):
    gravitational_force = (object1.mass * object2.mass) / (np.sqrt((object1.cooordinates[0] - object2.cooordinates[0])**2 + (object1.coordinates[1] - object2.coordinates[1])**2)) * G
    accel1 = gravitational_force / object1.mass
    accel2 = gravitational_force / object2.mass
    accel1 = np.array([])