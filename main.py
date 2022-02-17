import pygame
import numpy as np

import classes_functions as Gravsim_cf

Width = 900
Height = 500
Window = pygame.display.set_mode((Width, Height))
pygame.display.set_caption("Gravsim 0.0.1")

Colors = {
    "light_grey" : (200, 200, 200),
    "orange" : (255, 100, 0),
    "blue" : (25, 25, 255),
    "green" : (25, 255, 25)
}

Scale = 2

objects = []
objects += [Gravsim_cf.gravobject(0.005, 1, [220.0, 25.0], np.array([1, 0.0]), Colors['green']), Gravsim_cf.gravobject(0.001, 2, [200.0, 50.0], np.array([1, 0.0]), Colors['light_grey']), Gravsim_cf.gravobject(1, 6, [200.0, 125.0], np.array([0.0, 0.0]), Colors['blue'])]

def gravloop(delta):
    Gravsim_cf.gravinteract(objects)

    Gravsim_cf.apply_acceleration(objects, delta)

    for object in objects:
        object.render(Window, Scale)

def main():
    clock = pygame.time.Clock()
    run = True

    while run:
        delta = clock.tick(60)

        Window.fill((0, 0, 0))

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False

        gravloop(delta)
        pygame.display.update()
        
    pygame.quit()



if __name__ == "__main__":
    main()