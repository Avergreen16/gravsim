#define FREEIMAGE_LIB
#include <fstream>
#include <iostream>
#include <map>
#include <cmath>
#include <FreeImage\Dist\x64\FreeImage.h>
#include <string>
#include <ctime>

#include "Noisefactory.hpp"

std::map<std::string, RGBQUAD> biome_lookup_table = {
    {"sea", set_color(30, 30, 180)},
    {"deep_sea", set_color(10, 10, 160)},
    {"reef", set_color(50, 50, 150)},
    {"frozen_sea", set_color(1300, 150, 220)},
    {"grassland", set_color(120, 200, 120)},
    {"swamp", set_color(50, 145, 50)},
    {"forest", set_color(10, 180, 10)},
    {"rainforest", set_color(10, 250, 50)},
    {"boreal_forest", set_color(20, 100, 10)},
    {"shrubland", set_color(180, 200, 20)},
    {"desert", set_color(220, 200, 50)},
    {"ice_cap", set_color(250, 250, 250)},
    {"tundra", set_color(100, 80, 20)},
    {"river", set_color(60, 60, 200)},
    {"canyons", set_color(200, 140, 20)},
    {"mountains", set_color(150, 150, 150)}
};

RNG biome_rng(0.764575, 321.987439);

std::string get_biome(float elev, float humid, float temp) {
    if(elev > 0.95) {
        if(temp < 0.3) {
            return "ice_cap";
        } else {
            return "mountains";
        }
    } else if(elev > 0.8) {
        if(temp < 0.3) {
            return "ice_cap";
        } else {
            if(humid < 0.2) {
                return "grassland";
            } else if(humid < 0.7) {
                return "boreal_forest";
            } else {
                return "forest";
            }
        }
    } else if(elev > 0.6) {
        if(temp < 0.3) {
            if(humid < 0.1) {
                if(biome_rng.retrieve() > 0.4) {
                    return "tundra";
                } else {
                    return "ice_cap";
                }
            } else {
                return "ice_cap";
            }
        } else if(temp < 0.6) {
            if(humid < 0.1) {
                return "desert";
            } else if(humid < 0.3) {
                return "grassland";
            } else if(humid < 0.8) {
                return "boreal_forest";
            } else {
                return "swamp";
            }
        } else if(temp < 0.8) {
            if(humid < 0.2) {
                return "desert";
            } else if(humid < 0.4) {
                return "grassland";
            } else if(humid < 0.8) {
                return "forest";
            } else {
                return "swamp";
            }
        } else {
            if(humid < 0.1) {
                if(biome_rng.retrieve() > 0.4) {
                    return "canyons";
                } else {
                    return "desert";
                }
            } else if(humid < 0.3) {
                return "desert";
            } else if(humid < 0.5) {
                return "shrubland";
            } else if(humid < 0.65) {
                return "forest";
            } else if(humid < 0.85) {
                return "rainforest";
            } else {
                return "swamp";
            }
        } 
    } else {
        if(temp < 0.3) {
            return "frozen_sea";
        } else if(elev < 0.3) {
            return "deep_sea";
        } else if (humid > 0.7) {
            return "reef";
        } else {
            return "sea";
        }
    }
}

int main() {
    time_t starting_time = std::time(NULL);
    const int ImageWidth = 512;
    const int ImageHeight = 512;

    FreeImage_Initialise();
    FIBITMAP* img = FreeImage_Allocate(ImageWidth, ImageHeight, 24);
    RGBQUAD color;
    //VoronoiPoint* vpoint;
    
    //VoronoiFactory vfactory(0.9238749, ImageWidth, ImageHeight, 30, 30, true, true);

    PerlinNoiseFactory nfactory1(0.456874, 8, 8, 64, 3, 3, true, true);
    //PerlinNoiseFactory nfactory2(0.872347, 4, 4, 128, 2, 1, true, true);

    /*for(int x = 0; x < ImageWidth; x++) {
        for(int y = 0; y < ImageHeight; y++) {
            float value = std::floor(nfactory.pullvalue(x, y) * 255);
            if (value < 255 * 0.7) {
                value = 0;
            } else if (value > 255) {
                value = 255;
            }

            color = set_color(value, value, value);
            FreeImage_SetPixelColor(img, x, y, &color);
        }
    }*/
    
    float value1;
    //float value2;
    //float value3;
    std::vector<float> values;
    for(int x = 0; x < ImageWidth; x++) {
        for(int y = 0; y < ImageHeight; y++) {
            //vpoint = &vfactory.voronoi_map.at(vfactory.pull_closest_point(x, y));
            /*if(vpoint->biome == "x") {
                value1 = nfactory1.pullvalue(vpoint->x / 4, vpoint->y / 4);
                value2 = nfactory2.pullvalue(vpoint->x / 4, vpoint->y / 4);
                value3 = 1 - std::abs((float)vpoint->y - (float)ImageHeight / 2) / (ImageHeight / 2);
                std::vector<float> new_values = {value1, value2, value3};
                vpoint->values = new_values;
                vpoint->change_biome(get_biome(new_values[0], new_values[1], new_values[2]));
            }*/
            /*if(vpoint->value == -100) {
                value1 = nfactory2.pullvalue(vpoint->x / 4, vpoint->y / 4);
                vpoint->value = value1;
            }*/

            value1 = nfactory1.pullvalue(x, y);
            color = set_color(value1 * 255);
            //std::string biome = vpoint->biome;
            //color = biome_lookup_table.at(biome);

            FreeImage_SetPixelColor(img, x, y, &color);
        }
    }

    /*color = set_color(0, 0, 0);
    for(std::pair<std::string, VoronoiPoint> pair : vfactory.voronoi_map) {
        VoronoiPoint voronoi_point = std::get<1>(pair);
        FreeImage_SetPixelColor(img, voronoi_point.x, voronoi_point.y, &color);
    }*/
    time_t ending_time = std::time(NULL);

    char filename[] = "noise2.png";
    if (FreeImage_Save(FIF_PNG, img, filename)) {
        std::cout << "Image saved as " << filename << std::endl;
        std::cout << ending_time - starting_time << std::endl;
    } else {
        std::cout << "File was not successfully saved." << std::endl;
    }

    FreeImage_DeInitialise();

    return 0;
}