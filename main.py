import pygame

import classes_functions as Gravsimcf

Width = 900
Height = 500
Window = pygame.display.set_mode((Width, Height))
pygame.display.set_caption("Gravsim 0.0.1")

Colors = {
    "light_grey" : (200, 200, 200)
}

objects = []
objects.append(Gravsimcf.gravobject(1, 1, [0, 0], [-1, 0], Colors['light_grey']))

def main():
    clock = pygame.time.Clock()
    run = True

    while run:
        delta = clock.tick(60)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False
    
    pygame.quit()



if __name__ == "__main__":
    main()