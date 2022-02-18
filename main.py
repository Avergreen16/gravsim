import pygame
import numpy as np

import classes_functions as Gravsim_cf

Width = 1500
Height = 800
Window = pygame.display.set_mode((Width, Height))
pygame.display.set_caption("Gravsim 0.0.1")

Gravsim_cf.G = 0.1

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
    "hot_grey" : (250, 210, 180)
}

Scale = 2
Framerate = 60
view_offset = np.array([0.0, 0.0])
rfactor = 5

def orb_vel(parent_mass, orbital_radius):
    return np.sqrt(Gravsim_cf.G * parent_mass / orbital_radius)

def add_sattelite(parent, mass, rval, color, orbital_radius, r_or_d="radius"):
    objects.append(Gravsim_cf.gravobject(mass, rval, [parent.coordinates[0], parent.coordinates[1] - orbital_radius], [parent.velocity[0] + orb_vel(parent.mass, orbital_radius), parent.velocity[1]], color, r_or_d = r_or_d))

objects = [Gravsim_cf.gravobject(40000, 21.34, [0, 0], [0, 0], Colors["orange"], stabilized=True)]

add_sattelite(objects[0], 51.56, 0.78, Colors["red"], 290.8, r_or_d="density")
add_sattelite(objects[0], 13.30, 1.4, Colors["purple"], 1003.75, r_or_d="density")
add_sattelite(objects[0], 0.43, 4.57, Colors["hot_grey"], 2550.02, r_or_d="density")

add_sattelite(objects[2], 0.023, 5.89, Colors["hot_grey"], 16.8, r_or_d="density")

#add_sattelite(objects[0], 1.02, 1.13*rfactor, Colors["hot_grey"], 270.03)
#add_sattelite(objects[0], 1.16, 1.10*rfactor, Colors["light_grey"], 370.06)
#add_sattelite(objects[0], 0.30, 0.79*rfactor, Colors["light_grey"], 521.04)
#add_sattelite(objects[0], 0.77, 0.92*rfactor, Colors["blue"], 684.71)
#add_sattelite(objects[0], 0.93, 1.05*rfactor, Colors["light_blue"], 900.73)
#add_sattelite(objects[0], 1.15, 1.15*rfactor, Colors["light_blue"], 1096.57)
#add_sattelite(objects[0], 0.33, 0.78*rfactor, Colors["light_blue"], 1448.78)



def gravloop(delta, framerate=60):
    Gravsim_cf.gravinteract(objects)

    Gravsim_cf.apply_acceleration(objects, delta, framerate=framerate)

    for object in objects:
        object.render(Window, Scale, view_offset)

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
        
        gravloop(delta, framerate=Framerate)

        if mouse_pressed[2] and mouse_right_trigger == True:
            view_offset += np.array([(offset[0] - mouse_pos[0]) / Scale, (offset[1] - mouse_pos[1]) / Scale])
            offset = np.array([mouse_pos[0], mouse_pos[1]])
        
        #else:
        #    view_offset = objects[3].coordinates.copy()

        pygame.display.update()
        
    pygame.quit()

if __name__ == "__main__":
    main()