import pygame
import numpy as np

import classes_functions as Gravsim_cf

Width = 1500
Height = 800
Window = pygame.display.set_mode((Width, Height))
pygame.display.set_caption("Gravsim 0.0.1")

Colors = {
    "light_grey" : (200, 200, 200),
    "orange" : (255, 100, 0),
    "blue" : (25, 25, 255),
    "green" : (25, 255, 25)
}

Scale = 2
view_offset = np.array([0, 0])

objects = []
objects += [Gravsim_cf.gravobject(0.01, 10, [0, 1000], [1, 0], Colors['green']), Gravsim_cf.gravobject(0.1, 15, [0, 450], [1, 0], Colors['light_grey']), Gravsim_cf.gravobject(12, 40, [0, 0], [0, 0], Colors['orange'], stabilized=True)]

def gravloop(delta):
    Gravsim_cf.gravinteract(objects)

    Gravsim_cf.apply_acceleration(objects, delta)

    for object in objects:
        object.render(Window, Scale, view_offset)

def main():
    global Scale
    global view_offset
    clock = pygame.time.Clock()
    run = True
    highest_vel = 0

    while run:
        delta = clock.tick(60)
        mouse_pos = pygame.mouse.get_pos()

        Window.fill((0, 0, 0))

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False
            
            if event.type == pygame.MOUSEWHEEL:
                if event.y == 1:
                    Scale *= 1.15

                elif event.y == -1:
                    Scale /= 1.15
        
        if highest_vel < objects[1].velocity[1]:
            highest_vel = objects[1].velocity[1]
            print(highest_vel)

        gravloop(delta)
        pygame.display.update()
        
    pygame.quit()

if __name__ == "__main__":
    main()