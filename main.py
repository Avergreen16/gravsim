import pygame
import numpy as np

import classes_functions as Gravsim_cf
from classes_functions import orb_vel

Width = 1500
Height = 800
Window = pygame.display.set_mode((Width, Height))
pygame.display.set_caption("Gravsim 0.0.1")

Gravsim_cf.G = 50
Gravsim_cf.force_stable_orbits = True
Gravsim_cf.separate_barycenters_for_calculations = False
Gravsim_cf.render_barycenters = True

Colors = {
    "light_grey" : (200, 200, 200),
    "orange" : (255, 100, 0),
    "blue" : (25, 25, 255),
    "green" : (25, 255, 25),
    "red" : (255, 25, 25),
    "dark_grey" : (100, 100, 100),
    "light_blue" : (125, 125, 200),
    "purple" : (175, 25, 100),
    "white" : (255, 255, 255),
    "hot_grey" : (250, 180, 120)
}

Scale = 2
Framerate = 60
view_offset = np.array([0.0, 0.0])

def add_sattelite(parent, mass, rval, color, orbital_radius, r_or_d="radius"):
    objects.append(Gravsim_cf.gravobject(mass, rval, [parent.coordinates[0], parent.coordinates[1] - orbital_radius], [parent.velocity[0] + orb_vel(parent.mass, orbital_radius), parent.velocity[1]], color, r_or_d=r_or_d, parent=parent))

objects = []
objects.append(Gravsim_cf.barycenter(objects, 8, 3.6, Colors["blue"], 5, 2.7, Colors["purple"], [0.0, 0.0], 100, [0.0, 0.0]))

add_sattelite(objects[0], 0.03, 0.3, Colors["light_blue"], 1000)

def main():
    global Scale
    global view_offset
    clock = pygame.time.Clock()
    run = True
    mouse_right_trigger = False
    offset = np.array([pygame.mouse.get_pos()[0], pygame.mouse.get_pos()[1]])

    while run:
        delta = clock.tick(Framerate)
        mouse_pos = pygame.mouse.get_pos()
        mouse_pressed = pygame.mouse.get_pressed()

        Window.fill((0, 0, 0))

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False
            
            elif event.type == pygame.MOUSEWHEEL:
                if event.y == 1:
                    Scale *= 1.15

                elif event.y == -1:
                    Scale /= 1.15
            
            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 3:
                mouse_right_trigger = True
                offset = np.array([mouse_pos[0], mouse_pos[1]])
            
            elif event.type == pygame.MOUSEBUTTONUP and event.button == 3:
                mouse_right_trigger = False
        
        Gravsim_cf.gravloop(Window, Scale, view_offset, objects, delta, framerate=Framerate)

        if mouse_pressed[2] and mouse_right_trigger == True:
            view_offset += np.array([(offset[0] - mouse_pos[0]) / Scale, (offset[1] - mouse_pos[1]) / Scale])
            offset = np.array([mouse_pos[0], mouse_pos[1]])
        
        #else:
        #    view_offset = objects[3].coordinates.copy()

        pygame.display.update()
        
    pygame.quit()

if __name__ == "__main__":
    main()