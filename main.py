import pygame
import numpy as np

import classes_functions as Gravsim
from classes_functions import au, ld, lm, jm, sm, objects, barycenters, gravobject, barycenter, add_satellite, add_satellite_barycenter

Width = 1500
Height = 800
Window = pygame.display.set_mode((Width, Height))
pygame.display.set_caption("Gravsim 0.0.1")

Gravsim.force_stable_orbits = True
Gravsim.separate_barycenters_for_calculations = False
Gravsim.render_barycenters = True
Gravsim.default_r_or_d = "density"

rng = np.random.default_rng()

Color = {
    "light_grey" : (200, 200, 200),
    "orange" : (255, 100, 0),
    "blue" : (25, 25, 255),
    "green" : (25, 255, 25),
    "red" : (255, 25, 25),
    "dark_grey" : (100, 100, 100),
    "light_blue" : (125, 125, 200),
    "purple" : (175, 25, 100),
    "white" : (255, 255, 255),
    "hot_grey" : (250, 180, 120),
    "yellow" : (255, 255, 25),
    "dull_yellow" : (150, 150, 25),
    "light_purple" : (210, 125, 200),
    "cyan" : (25, 140, 175)
}

Scale = 0.1
Framerate = 60
view_offset = np.array([0.0, 0.0])

gravobject(1000000, 2, [0, 0], [0, 0], Color["light_grey"])
print(objects[0].radius)

'''#ravobject(140000, 40, [0, 0], [0, 0], Color["yellow"], fixed=True)
gravobject(sm(2.3), 2.9, [0, 0], [0, 0], Color["light_blue"], fixed=True)

add_satellite_barycenter(objects[0], sm(0.43), 1.7, Color["orange"], sm(0.14), 1.5, Color["red"], au(100000), ld(31), eccentricity1=0.05)
#barycenter(sm(0.43), 1.7, Color["orange"], sm(0.14), 1.5, Color["red"], [0, 0], ld(31), [0, 0], fixed=True, eccentricity=0.05, argument_of_periapsis=rng.random() * 360)'''

'''add_satellite(barycenters[0], 0.13, 5.2, Color["light_grey"], au(0.34), eccentricity=0.01, argument_of_periapsis=rng.random() * 360) #0
add_satellite(barycenters[0], 0.67, 3.9, Color["blue"], au(1.02), eccentricity=0.009, argument_of_periapsis=rng.random() * 360) #0
add_satellite(barycenters[0], lm(0.2), 4.1, Color["light_blue"], au(1.52), eccentricity=0.03, argument_of_periapsis=rng.random() * 360) #0
add_satellite(barycenters[0], lm(0.13), 2.8, Color["light_blue"], au(1.81), eccentricity=0.09, argument_of_periapsis=rng.random() * 360) #0
add_satellite(barycenters[0], jm(0.73), 1.3, Color["dull_yellow"], au(2.82), eccentricity=0.005, argument_of_periapsis=rng.random() * 360) #4
add_satellite(barycenters[0], 47, 1.5, Color["blue"], au(5.44), eccentricity=0.015, argument_of_periapsis=rng.random() * 360) #2
add_satellite(barycenters[0], 23, 0.8, Color["purple"], au(8.03), eccentricity=0.023, argument_of_periapsis=rng.random() * 360) #5
add_satellite(barycenters[0], lm(0.31), 2.1, Color["light_blue"], au(14.3), eccentricity=0.078, argument_of_periapsis=rng.random() * 360) #0
add_satellite(barycenters[0], lm(0.21), 1.6, Color["light_purple"], au(17.87), eccentricity=0.04, argument_of_periapsis=rng.random() * 360) #1
add_satellite(barycenters[0], lm(0.08), 1.8, Color["light_blue"], au(24.09), eccentricity=0.34, argument_of_periapsis=rng.random() * 360) #0'''

'''add_satellite(objects[6], lm(0.24), 4.8, Color["light_grey"], ld(0.56), eccentricity=0.008, argument_of_periapsis=rng.random() * 360)
add_satellite(objects[6], lm(0.34), 2.8, Color["light_blue"], ld(1.14), eccentricity=0.006, argument_of_periapsis=rng.random() * 360)
add_satellite(objects[6], lm(0.09), 1.7, Color["light_blue"], ld(3.81), eccentricity=0.005, argument_of_periapsis=rng.random() * 360)
add_satellite(objects[6], lm(3.2), 2.7, Color["cyan"], ld(6.16), eccentricity=0.012, argument_of_periapsis=rng.random() * 360)

add_satellite(objects[7], lm(0.21), 1.8, Color["light_blue"], ld(0.32), eccentricity=0.007, argument_of_periapsis=rng.random() * 360)
add_satellite(objects[7], lm(0.06), 1.8, Color["light_blue"], ld(1.4), eccentricity=0.003, argument_of_periapsis=rng.random() * 360)

add_satellite(objects[8], lm(0.3), 2.2, Color["light_blue"], ld(0.45), eccentricity=0.008, argument_of_periapsis=rng.random() * 360)
add_satellite(objects[8], lm(0.04), 1.6, Color["light_purple"], ld(1.27), eccentricity=0.007, argument_of_periapsis=rng.random() * 360)
add_satellite(objects[8], lm(0.05), 1.4, Color["light_blue"], ld(1.57), eccentricity=0.007, argument_of_periapsis=rng.random() * 360)
add_satellite(objects[8], lm(0.09), 1.7, Color["light_blue"], ld(2.57), eccentricity=0.004, argument_of_periapsis=rng.random() * 360)
add_satellite(objects[8], lm(0.13), 1.8, Color["light_grey"], ld(3.61), eccentricity=0.012, argument_of_periapsis=rng.random() * 360)

add_satellite(objects[10], lm(0.04), 1.4, Color["white"], ld(0.2), eccentricity=0.007, argument_of_periapsis=rng.random() * 360)'''


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
        
        Gravsim.gravloop(Window, Scale, view_offset, delta, framerate=Framerate)

        if mouse_pressed[2] and mouse_right_trigger == True:
            view_offset += np.array([(offset[0] - mouse_pos[0]) / Scale, (offset[1] - mouse_pos[1]) / Scale])
            offset = np.array([mouse_pos[0], mouse_pos[1]])

        pygame.display.update()
        
    pygame.quit()

if __name__ == "__main__":
    main()