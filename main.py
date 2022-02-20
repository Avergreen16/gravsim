import pygame
import numpy as np

import classes_functions as Gravsim

Width = 1500
Height = 800
Window = pygame.display.set_mode((Width, Height))
pygame.display.set_caption("Gravsim 0.0.1")

Gravsim.force_stable_orbits = True
Gravsim.separate_barycenters_for_calculations = False
Gravsim.render_barycenters = True

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
    "dull_yellow" : (150, 150, 25)
}

Scale = 1
Framerate = 60
view_offset = np.array([0.0, 0.0])

#Gravsim.gravobject(140000, 40, [0, 0], [0, 0], Color["yellow"], stabilized=True)
Gravsim.barycenter(100000, 1.7, Color["orange"], 80000, 1.5, Color["red"], [0, 0], 1000, [0, 0], object1_r_or_d="density", object2_r_or_d="density")

"""#Gravsim.add_satellite(Gravsim.objects[0], 13, 4, Color["dull_yellow"], 50000)

Gravsim.add_satellite_barycenter(Gravsim.barycenters[0], 7, 3, Color["blue"], 6, 2.5, Color["dull_yellow"], 50000, 120)"""
'''Gravsim.barycenter(8, 3.6, Color["blue"], 5, 2.7, Color["purple"], [0.0, 0.0], 100, [0.0, 0.0], stabilized=True)

#Gravsim.add_satellite(Gravsim.objects[0], 0.03, 0.3, Color["light_blue"], 700)
'''
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
        
        """else:
            view_offset = Gravsim.barycenters[1].coordinates.copy()"""

        pygame.display.update()
        
    pygame.quit()

if __name__ == "__main__":
    main()