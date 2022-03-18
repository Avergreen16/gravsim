#define FREEIMAGE_LIB
#include <fstream>
#include <iostream>
#include <map>
#include <cmath>
#include <FreeImage\Dist\x64\FreeImage.h>
#include <string>
#include <ctime>
#include <bits/stdc++.h>

#include "Noisefactory.hpp"

// 0.865234
double seed = 0.865234;

double clamp(double input, double lower, double upper) {
    if (input <= lower) {
        return lower;
    } else if (input >= upper) {
        return upper;
    } else {
        return input;
    }
}

class VoronoiTerrainMapGenerator {
    private:
        int width;
        int height;
        int e_width;
        int e_height;
        int e_octaves;
        int e_persistance;
        int h_width;
        int h_height;
        int h_octaves;
        int h_persistance;
        int v_xsize;
        int v_ysize;
        float seed;
        RNG biome_rng;
        VoronoiFactory v_fac;
        PerlinNoiseFactory p_fac_e;
        PerlinNoiseFactory p_fac_h;
        PerlinNoiseFactory p_fac_t;

    public:
        void initialise_terrain_map(float p_seed, int p_width, int p_height, int p_v_xsize, int p_v_ysize, int p_e_width, int p_e_height, int p_e_octaves, int p_e_persistance, int p_h_width, int p_h_height, int p_h_octaves, int p_h_persistance, int p_t_width, int p_t_height, int p_t_octaves, int p_t_persistance) {
            width = p_width;
            height = p_height;
            v_xsize = p_v_xsize;
            v_ysize = p_v_ysize;
            biome_rng = RNG(p_seed, 20.827648);
            v_fac = VoronoiFactory(p_seed, p_width, p_height, p_v_xsize, p_v_ysize, true, true);
            p_fac_e = PerlinNoiseFactory(p_seed, p_seed * 23.663879, p_e_width, p_e_height, std::min(p_width, p_height) / std::min(p_e_width, p_e_height), p_e_octaves, p_e_persistance, true, true);
            p_fac_h = PerlinNoiseFactory(p_seed, p_seed * 48.679649, p_h_width, p_h_height, std::min(p_width, p_height) / std::min(p_h_width, p_h_height), p_h_octaves, p_h_persistance, true, true);
            p_fac_t = PerlinNoiseFactory(p_seed, p_seed * 56.343759, p_t_width, p_t_height, std::min(p_width, p_height) / std::min(p_t_width, p_t_height), p_t_octaves, p_t_persistance, true, true);
        }

        void generate(std::string p_filename) {
            char const* filename = p_filename.c_str();
            FIBITMAP* img = FreeImage_Allocate(width, height, 24);

            float value1;
            float value2;
            float value3;
            RGBQUAD color;
            VoronoiPoint* vpoint;
            v_fac.generate_points();
            p_fac_e.generate_vectors();
            p_fac_h.generate_vectors();
            p_fac_t.generate_vectors();
            for(int x = 0; x < width; x++) {
                for(int y = 0; y < height; y++) {
                    vpoint = &v_fac.voronoi_map.at(v_fac.pull_closest_point_key(x, y));
                    if(vpoint->biome == "x") {
                        value1 = p_fac_e.pullvalue(vpoint->x, vpoint->y);
                        value2 = p_fac_t.pullvalue(vpoint->x, vpoint->y);
                        value3 = clamp(1 - (std::abs((float)vpoint->y - (float)height / 2) / (height / 2) + (p_fac_t.pullvalue(vpoint->x, vpoint->y) * 2 - 1) * 0.25), 0, 1);
                        std::vector<float> new_values = {value1, value2, value3};
                        vpoint->change_biome(get_biome(new_values[0], new_values[1], new_values[2]));
                    }
                    
                    std::string biome = vpoint->biome;
                    color = biome_lookup_table.at(biome);

                    FreeImage_SetPixelColor(img, x, y, &color);
                }
            }

            if (FreeImage_Save(FIF_PNG, img, filename)) {
                std::cout << "Image saved as " << filename << std::endl;
            } else {
                std::cout << "File was not successfully saved." << std::endl;
            }
        }

        struct Arc;

        struct Vertex;

        struct HalfEdge;

        struct Face;

        struct Event;

        void fortunes_algorithm() {
            float sweep_line_y;
            v_fac.generate_points();
            std::vector<std::pair<int, int>> events;
            std::pair<int, int> active_event;
            for(std::pair<int, VoronoiPoint> pair : v_fac.voronoi_map) {
                VoronoiPoint v_pt = std::get<1>(pair);
                events.push_back({0, v_pt.y});
            }
            std::sort(events.begin(), events.end());
            while (events.size() != 0) {
                active_event = events[0];
                events.erase(events.begin());
                if(std::get<0>(active_event) == 0) {

                } else {

                }
            }
        }

        std::map<std::string, RGBQUAD> biome_lookup_table = {
            {"sea", set_color(30, 30, 180)},
            {"deep_sea", set_color(10, 10, 160)},
            {"reef", set_color(50, 50, 150)},
            {"frozen_sea", set_color(150, 180, 220)},
            {"grassland", set_color(120, 200, 120)},
            {"swamp", set_color(50, 145, 80)},
            {"forest", set_color(10, 180, 10)},
            {"rainforest", set_color(10, 250, 50)},
            {"boreal_forest", set_color(20, 100, 10)},
            {"shrubland", set_color(180, 200, 20)},
            {"desert", set_color(220, 200, 50)},
            {"ice_cap", set_color(240, 240, 250)},
            {"tundra", set_color(100, 100, 20)},
            {"river", set_color(60, 60, 200)},
            {"canyons", set_color(200, 140, 20)},
            {"mountains", set_color(150, 150, 150)},
            {"glacier", set_color(215, 215, 250)}
        };

        std::string get_biome(float elev, float humid, float temp) {
            if(elev > 0.82) {
                return "glacier";
            } else if(elev > 0.75) {
                if(temp < 0.2) {
                    return "glacier";
                } else {
                    return "mountains";
                }
            } else if(elev > 0.7) {
                if(temp < 0.2) {
                    return "ice_cap";
                } else {
                    if(humid < 0.45) {
                        return "grassland";
                    } else if(humid < 0.65) {
                        return "boreal_forest";
                    } else {
                        return "forest";
                    }
                }
            } else if(elev > 0.55) {
                if(temp < 0.2) {
                    if(humid < 0.4) {
                        if (biome_rng.retrieve() < 0.3) {
                            return "tundra";
                        }
                        return "ice_cap";
                    } else {
                        return "ice_cap";
                    }
                } else if(elev < 0.6 && humid > 0.45) {
                    if(biome_rng.retrieve() < 0.2) {
                        return "swamp";
                    }
                    if(temp < 0.45) {
                        goto switch_swamp_045_;
                    } else if(temp < 0.8) {
                        goto switch_swamp_08_;
                    } else {
                        goto switch_swamp_else_;
                    }
                } else if(temp < 0.45) {
                    switch_swamp_045_:
                    if(humid < 0.1) {
                        return "desert";
                    } else if(humid < 0.3) {
                        return "grassland";
                    } else {
                        return "boreal_forest";
                    }
                } else if(temp < 0.8) {
                    switch_swamp_08_:
                    if(humid < 0.35) {
                        return "desert";
                    } else if(humid < 0.45) {
                        return "grassland";
                    } else if(humid < 0.65) {
                        return "forest";
                    } else {
                        return "swamp";
                    }
                } else {
                    switch_swamp_else_:
                    if(humid < 0.3) {
                        return "canyons";
                    } else if(humid < 0.45) {
                        return "desert";
                    } else if(humid < 0.5) {
                        return "shrubland";
                    } else if(humid < 0.55) {
                        return "forest";
                    } else if(humid < 0.7) {
                        return "rainforest";
                    } else {
                        return "swamp";
                    }
                } 
            } else {
                if(temp < 0.2) {
                    return "frozen_sea";
                } else if(elev < 0.4) {
                    return "deep_sea";
                } else if (humid > 0.5 && temp > 0.5) {
                    return "reef";
                } else {
                    return "sea";
                }
            }
        }
};

int main() {
    time_t starting_time = std::time(NULL);
    FreeImage_Initialise();
    
    //std::vector<int> numbervector(20);
    /*for(int x = 0; x < ImageWidth; x++) {
        for(int y = 0; y < ImageHeight; y++) {

            color = set_color(nfactory1.pullvalue(x, y) * 255);
            //color = set_color(clamp(value, 0, 2) / 3 * 255, clamp(value - 3, 0, 2) / 3 * 255, clamp(value - 6, 0, 3) / 3 * 255);
            FreeImage_SetPixelColor(img, x, y, &color);
            //numbervector[value] += 1;
        }
    }*/

    //for(int i = 0; i < numbervector.size(); i++) {
    //    std::cout << std::to_string((float)i / 20) << " - " << std::to_string((float)(i + 1) / 20) << ": " << (float)numbervector[i] / (ImageWidth * ImageHeight) * 100 << "%" << std::endl;
    //}
    RNG rng(0.78634873, 12.876248);

    int iterations = 64;
    float sd_average;
    VoronoiTerrainMapGenerator vgen;
    for(int i = 0; i < iterations; i++) {
        float seed = rng.retrieve(6);
        vgen.initialise_terrain_map(seed, 1024, 512, 60, 30, 4, 2, 3, 2, 4, 2, 1, 1, 4, 2, 1, 1);
        vgen.generate("file" + std::to_string(i) + ".png");
        std::cout << std::to_string(seed) << "\n\n"; 
    }

    /*std::ofstream file0;
    file0.open("perlin_noise_vector_log.txt");
    if (file0.is_open()) {
        for (std::pair<int, std::vector<float>> _pair : nfactory1.origin_vector_map) {
            file0 << std::to_string(std::get<1>(_pair)[0]) << " " << std::to_string(std::get<1>(_pair)[1]) << "\n";
        }
        std::cout << nfactory1.origin_vector_map.size() << std::endl;
    }
    file0.close();*/

    //VoronoiTerrainMapGenerator vgen;
    //vgen.initialise_terrain_map(0.542735, 1024, 512, 60, 30, 8, 4, 2, 3, 4, 2, 1, 1, 4, 2, 1, 1);
    //vgen.generate("file0.png");

    time_t ending_time = std::time(NULL);
    std::cout << ending_time - starting_time << std::endl;

    FreeImage_DeInitialise();

    return 0;
}

struct VoronoiTerrainMapGenerator::Face {
    VoronoiPoint* voronoipoint;
    HalfEdge* halfedge;
};

struct VoronoiTerrainMapGenerator::Arc {
    Arc* parent;
    Arc* left;
    Arc* right;

    VoronoiPoint* site;
    HalfEdge* leftHalfEdge;
    HalfEdge* rightHalfEdge;
    Event* event;

    Arc* prev;
    Arc* next;
};

struct VoronoiTerrainMapGenerator::Vertex {
    float x;
    float y;
};

struct VoronoiTerrainMapGenerator::HalfEdge {
    Vertex* origin;
    Vertex* destination;
    HalfEdge* twin;
    Face* face;
    HalfEdge* prev;
    HalfEdge* next;
};

struct VoronoiTerrainMapGenerator::Event {

};