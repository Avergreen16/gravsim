#include <SDL2-w64\include\SDL2\SDL.h>
#include <SDL2-w64\include\SDL2\SDL_image.h>
#include <Cimg\Cimg.h>
#include <iostream>

#include "RenderWindow.hpp"
#include "Entity.hpp"

int main(int argv, char* args[]) {
    if(SDL_Init(SDL_INIT_VIDEO) > 0) {
        cout << "SDL_Init HAS FAILED. SDL ERROR: " << SDL_GetError() << endl;
    }
    if(!IMG_Init(IMG_INIT_PNG)) {
        cout << "IMG_Init has failed. ERROT: " << SDL_GetError() << endl;
    }

    int width = 512;
    int height = 512;
    const unsigned char blue_green[] = {0, 170, 255};

    cimg_library::CImg<unsigned char> BG(width, height, 1, 3, 255);

    BG.draw_circle(width / 2, height / 2, 100, blue_green);

    BG.display();

    /*
    RenderWindow window("GAME v1.0", 1280, 720);

    bool game_running = true;

    SDL_Texture* grass_texture = window.load_texture("res\\rpg-pack\\tiles\\generic-rpg-tile25.png");

    SDL_Event event;

    while(game_running) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                game_running = false;
            }
        }
        window.clear();
        window.render(grass_texture);
        window.display();
    }
    */

    return 0;
}