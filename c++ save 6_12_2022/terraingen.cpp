#define _USE_MATH_DEFINES
#include <SFML-2.5.1\include\SFML\Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <ctime>

sf::Image biome_map_terra;

struct RNG {
    double factor;
    double product;

    void set_seed(double p_seed, double p_factor) {
        factor = p_factor;
        product = p_seed;
    }

    double retrieve() {
        product = product * factor;
        product -= floor(product);
        return product;
    }
};

//////////////////////////////////////
double clamp(double input, double min, double max) {
    if(input >= max) return max;
    if(input <= min) return min;
    return input;
}

double normalCDF(double input, double sd, double mean) {
    double value = (input - mean) / sd;
    return 0.5 * erfc(-value * M_SQRT1_2);
}

std::vector<std::vector<std::vector<int>>> terra_biome_vector2 = {
    {{1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {4, 4, 4, 4, 4, 4, 4, 4}},

    {{2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 2, 2},
    {4, 4, 4, 4, 4, 4, 4, 4}},

    {{2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 2, 2},
    {4, 4, 4, 4, 4, 4, 4, 4}},

    {{2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 2, 2},
    {4, 4, 4, 4, 4, 4, 4, 4}},

    {{10, 8, 11, 11, 5, 12, 12, 9},
    {8, 8, 11, 6, 5, 5, 5, 9},
    {8, 8, 6, 7, 7, 7, 5, 9},
    {14, 14, 13, 13, 13, 13, 13, 13}},

    {{15, 8, 11, 11, 5,  5,  5,  12},
    {15,  8, 11, 6,  7,  7,  7,  5},
    {15,  8, 6,  7,  7,  7,  7,  5},
    {14,  14, 13, 13, 13, 13, 13, 13}},

    {{16, 16, 16, 16, 18, 18, 18, 18},
    {16, 16, 16, 16, 16, 16, 16, 16},
    {16, 16, 16, 16, 16, 16, 16, 17},
    {16, 16, 17, 17, 17, 17, 17, 17}}
};

std::vector<std::vector<std::vector<int>>> terra_biome_vector = {
    {{1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {4, 4, 4, 4, 4, 4, 4, 4}},

    {{2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 2, 2},
    {4, 4, 4, 4, 4, 4, 4, 4}},

    {{2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 2, 2},
    {4, 4, 4, 4, 4, 4, 4, 4}},

    {{2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 3, 3},
    {2, 2, 2, 2, 2, 2, 2, 2},
    {4, 4, 4, 4, 4, 4, 4, 4}},

    {{10, 10, 11, 11, 5, 12, 12, 9},
    {8, 8, 11, 11, 5, 5, 5, 9},
    {8, 8, 6, 6, 7, 7, 5, 9},
    {14, 14, 13, 13, 13, 13, 13, 13}},

    {{15, 8, 11, 11, 5,  5,  5,  12},
    {15,  8, 11, 11,  7,  7,  7,  5},
    {15,  8, 6,  6,  7,  7,  7,  5},
    {14,  14, 13, 13, 13, 13, 13, 13}},

    {{16, 16, 16, 16, 18, 18, 18, 18},
    {16, 16, 16, 16, 16, 16, 16, 16},
    {16, 16, 16, 16, 16, 16, 16, 17},
    {16, 16, 17, 17, 17, 17, 17, 17}}
};

struct PerlinNoiseFactory {
    int main_vector_width;
    int main_vector_height;
    int octaves;
    double persistance;
    bool loop_x;
    bool loop_y;
    double maxvalue;
    RNG rng;

    double sd;
    double mean;

    std::vector<sf::Vector2<double>> vector_container;

    void set_parameters(double p_seed, double p_factor, int p_main_vector_width, int p_main_vector_height, int p_octaves, double p_persistance, double p_sd, double p_mean, bool p_loop_x, bool p_loop_y);

    void generate_vectors();

    double retrieve(int x, int y, int width, int height);
};

struct PlanetBiomeGenerator {
    PerlinNoiseFactory pnf1;
    PerlinNoiseFactory pnf2;
    PerlinNoiseFactory pnf3;
    
    std::map<int, sf::Color> biome_lookup_table = {
        //Terra
        {1, sf::Color(0x1418ADFF)}, // deep sea
        {2, sf::Color(0x191FD3FF)}, // sea
        {3, sf::Color(0x1942D3FF)}, // reef
        {4, sf::Color(0x90B1DBFF)}, // frozen sea
        {5, sf::Color(0x008944FF)}, // forest
        {6, sf::Color(0x8BCE50FF)}, // grassland
        {7, sf::Color(0x286059FF)}, // boreal forest
        {8, sf::Color(0xE2DC98FF)}, // desert
        {9, sf::Color(0x1B541FFF)}, // swamp
        {10, sf::Color(0xE59647FF)}, // scorched desert
        {11, sf::Color(0xC8D16AFF)}, // shrubland
        {12, sf::Color(0x00B539FF)}, // rainforest
        {13, sf::Color(0xF4ECFFFF)}, // ice cap
        {14, sf::Color(0x9E8F62FF)}, // tundra
        {15, sf::Color(0xC6833FFF)}, // canyons
        {16, sf::Color(0x706A6AFF)}, // mountains
        {17, sf::Color(0xAED3D3FF)}, // glacier
        {18, sf::Color(0x6A7C6AFF)}, // mossy mountains
        {19, sf::Color(60, 60, 200, 255)}, // river
        {20, sf::Color(0xE2DC98FF)} // beach
    };

    void set_parameters(double p_seed, int p_width1, int p_height1, int p_octaves1, double p_persistance1, int p_width2, int p_height2, int p_octaves2, double p_persistance2, int p_width3, int p_height3, int p_octaves3, double p_persistance3);

    sf::Color get_biome(int x, int y, int width, int height) {
        double v1 = pnf1.retrieve(x, y, width, height)/* * clamp(3.8 - hypot(double(x) / width - 0.5, double(y) / height - 0.5) * 8, 0, 1)*/ * 12; // * clamp(2.8 - 2 * hypot((double(x) / height - 1) * 2, (double(y) / height - 0.5) * 2), 0, 1.5)
        //int v2 = floor(clamp(((pnf2.retrieve(x, y, width, height) - 0.5) * 0.5 + double(height - y) / height * 4 + 0.5), 0, 3));
        double v2 = clamp(((pnf2.retrieve(x, y, width, height) - 0.5) * 0.125 + (0.5 - std::abs(double(y) / height - 0.5))) * 24, 0, 11.999);
        int v3 = floor(pnf3.retrieve(x, y, width, height) * 12);

        sf::Uint32 biome = biome_map_terra.getPixel(floor(v3) + floor(v1) * 12, floor(v2)).toInteger();
        if(v1 > 7 && v1 <= 7.04 && biome != 0xE59647FF && biome != 0xF4ECFFFF && biome != 0x2D541BFF && v2 >= 3) biome = 0xE2DC98FF;
        //if(biome == 4 && v2 < 3.3) biome = terra_biome_vector[floor(v1)][2][v3];
        return sf::Color(biome);//biome_lookup_table.at(biome);
    }
};

double dotproduct(sf::Vector2<double> v1, sf::Vector2<double> v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

double interpolate(double x1, double x2, double pos) {
    double returnvalue = (x2 - x1) * (6 * pos * pos * pos * pos * pos - 15 * pos * pos * pos * pos + 10 * pos * pos * pos) + x1;
    return returnvalue;
}

double pyth(double x, double y) {
    return std::sqrt(x * x + y * y);
}

void PerlinNoiseFactory::set_parameters(double p_seed, double p_factor, int p_main_vector_width, int p_main_vector_height, int p_octaves, double p_persistance, double p_sd, double p_mean, bool p_loop_x, bool p_loop_y) {
    main_vector_width = p_main_vector_width;
    main_vector_height = p_main_vector_height;
    octaves = p_octaves;
    persistance = p_persistance;
    loop_x = p_loop_x;
    loop_y = p_loop_y;
    rng.set_seed(p_seed, p_factor);
    
    sd = p_sd;
    mean = p_mean;

    maxvalue = 0;
    for(int i = 0; i < p_octaves; i++) {
        maxvalue += 1 * pow(p_persistance, -i);
    }
}

void PerlinNoiseFactory::generate_vectors() {
    vector_container.clear();
    int array_width = main_vector_width * pow(2, octaves);
    int array_height = main_vector_height * pow(2, octaves);
    for(int k = 0; k < array_width * array_height; k++) {
        double theta = rng.retrieve() * 2 * M_PI;
        sf::Vector2<double> insertvector(std::cos(theta), std::sin(theta));
        vector_container.push_back(insertvector);
    }
}

double PerlinNoiseFactory::retrieve(int x, int y, int p_width, int p_height) {
    double returnvalue = 0;
    for(int i = 0; i < octaves; i++) {
        //updated scale
        int i_power = 1 << i;
        double updated_xscale = double(p_width) / (main_vector_width * i_power) * 0.99999999;
        double updated_yscale = double(p_height) / (main_vector_height * i_power) * 0.99999999;

        // stores the point's 4 dot products for interpolation
        double dotproducts[4];

        // grabs the coordinate value of the top left vector of the 4 surrounding vectors in the "vector array", each vector gets an
        // integer coordinate. These values are used to determine the vector's key
        int topleft_x = double(x) * i_power * main_vector_width / p_width;
        int topleft_y = double(y) * i_power * main_vector_height / p_height;
        
        for(int j = 0; j < 4; j++) {
            // vector_x and vector_y are the x and y integer coordinates of the active vector, determined by j, in the "vector array"
            int vector_x = topleft_x + j % 2;
            int vector_y = topleft_y + floor(j / 2);

            // these are the true coordinates of the active vector so that the distance vector between the 
            // active vector's origin and the point's x and y can be calculated
            double pos_value_x = vector_x * updated_xscale;
            double pos_value_y = vector_y * updated_yscale;

            vector_x -= main_vector_width * i_power * (loop_x) * (vector_x >= main_vector_width * i_power);
            vector_y -= main_vector_height * i_power * (loop_y) * (vector_y >= main_vector_height * i_power);

            // creates vector key for lookup in the vector map
            int vector_key = vector_x + vector_y * main_vector_width * i_power;

            // vector of active point, current active point is one of the 4 closest vectors, this variable is assigned each of the
            // 4 vectors as the j loop runs.
            sf::Vector2<double> active_vector = vector_container[vector_key];
            // distance vector between true position of active vector's origin and x and y coordinate
            sf::Vector2<double> coordinate_distance_vector = {(double(x) - pos_value_x) / updated_xscale, (double(y) - pos_value_y) / updated_yscale};
            //if(coordinate_distance_vector[0] == 0 || coordinate_distance_vector[1] == 0) printf("check");

            dotproducts[j] = dotproduct(active_vector, coordinate_distance_vector);
        }

        // interpolates the dot products bilineraly and returns the result
        //(double)(coordinate % scale) / scale
        double x1 = interpolate(dotproducts[0], dotproducts[1], fmod(double(x), updated_xscale) / updated_xscale);
        double x2 = interpolate(dotproducts[2], dotproducts[3], fmod(double(x), updated_xscale) / updated_xscale);
        returnvalue += (interpolate(x1, x2 , fmod(double(y), updated_yscale) / updated_yscale) + M_SQRT1_2) / (M_SQRT1_2 * 2) * pow(persistance, -i);
    }
    return normalCDF(returnvalue / maxvalue, sd, mean);
}

void PlanetBiomeGenerator::set_parameters(double p_seed, int p_width1, int p_height1, int p_octaves1, double p_persistance1, int p_width2, int p_height2, int p_octaves2, double p_persistance2, int p_width3, int p_height3, int p_octaves3, double p_persistance3) {
    pnf1.set_parameters(p_seed, p_seed * 45.18754874623, p_width1, p_height1, p_octaves1, p_persistance1, 0.15, 0.5, true, true);
    pnf2.set_parameters(p_seed, p_seed * 21.1324254354, p_width2, p_height2, p_octaves2, p_persistance2, 0.2, 0.5, true, true);
    pnf3.set_parameters(p_seed, p_seed * 57.62634832648, p_width3, p_height3, p_octaves3, p_persistance3, 0.15, 0.5, true, true);
    pnf1.generate_vectors();
    pnf2.generate_vectors();
    pnf3.generate_vectors();
}
//////////////////////////////////////

bool point_sort_y(sf::Vector2<double> p1, sf::Vector2<double> p2) {
    if(p1.y < p2.y) return true;
    if(p1.y == p2.y) return (p1.x < p2.x);
    return false;
}

bool point_sort_x(sf::Vector2<double> p1, sf::Vector2<double> p2) {
    if(p1.x < p2.x) return true;
    if(p1.x == p2.x) return (p1.y < p2.y);
    return false;
}

double get_value_y(double slope, sf::Vector2<double> p, double y) {
    return (y - p.y) / slope + p.x;
}

double get_value_x(double slope, sf::Vector2<double> p, double x) {
    return slope * (x - p.x) + p.y;
}

double get_slope(sf::Vector2<double> p1, sf::Vector2<double> p2) {
    return (p2.y - p1.y) / (p2.x - p1.x);
}

template <typename T>
int sign(T input) {
    if(input > 0) return 1;
    if(input < 0) return -1;
    return 0;
}

void rasterize_line(sf::Vector2<double> p1, sf::Vector2<double> p2, sf::Image* img, sf::Color color) {
    double slope = get_slope(p1, p2);
    if(slope / sign(slope) <= 1) {
        for(int x = round(p1.x); x != round(p2.x); x += sign(p2.x - p1.x)) {
            img->setPixel(x, round(get_value_x(slope, p1, x)), color);
        }
    } else {
        for(int y = round(p1.y); y != round(p2.y); y += sign(p2.y - p1.y)) {
            img->setPixel(round(get_value_y(slope, p1, y)), y, color);
        }
    }
}

struct Triangle {
    std::vector<sf::Vector2<double>> points;
    bool is_on_edge = false;

    Triangle(sf::Vector2<double> p1, sf::Vector2<double> p2, sf::Vector2<double> p3, bool p_is_on_edge) {
        points.push_back(p1);
        points.push_back(p2);
        points.push_back(p3);

        is_on_edge = p_is_on_edge;
    }

    void rasterize(sf::Image* img, sf::Color color, sf::Vector2u img_size);
};

void Triangle::rasterize(sf::Image* img, sf::Color color, sf::Vector2u img_size) {
    if(is_on_edge) {
        sort(points.begin(), points.end(), point_sort_y);
        double slope1 = get_slope(points[0], points[1]);
        double slope2 = get_slope(points[0], points[2]);
        double slope3 = get_slope(points[2], points[1]);

        int x1, y1;
        int y = round(points[0].y);
        if(1.0 / slope1 < 1.0 / slope2) {
            for(; y < points[1].y; y++) {
                for(int x = round(get_value_y(slope1, points[1], y)); x < round(get_value_y(slope2, points[2], y)); x++) {
                    if(x < 0) {
                        x1 = x + img_size.x;
                    } else if(x >= img_size.x) {
                        x1 = x - img_size.x;
                    } else {
                        x1 = x;
                    }

                    if(y < 0) {
                        y1 = y + img_size.y;
                    } else if(y >= img_size.y) {
                        y1 = y - img_size.y;
                    } else {
                        y1 = y;
                    }

                    img->setPixel(x1, y1, color);
                }
            }
            for(; y < points[2].y; y++) {
                for(int x = round(get_value_y(slope3, points[1], y)); x < round(get_value_y(slope2, points[2], y)); x++) {
                    if(x < 0) {
                        x1 = x + img_size.x;
                    } else if(x >= img_size.x) {
                        x1 = x - img_size.x;
                    } else {
                        x1 = x;
                    }

                    if(y < 0) {
                        y1 = y + img_size.y;
                    } else if(y >= img_size.y) {
                        y1 = y - img_size.y;
                    } else {
                        y1 = y;
                    }

                    img->setPixel(x1, y1, color);
                }
            }
        } else {
            for(; y < points[1].y; y++) {
                for(int x = round(get_value_y(slope2, points[2], y)); x < round(get_value_y(slope1, points[1], y)); x++) {
                    if(x < 0) {
                        x1 = x + img_size.x;
                    } else if(x >= img_size.x) {
                        x1 = x - img_size.x;
                    } else {
                        x1 = x;
                    }

                    if(y < 0) {
                        y1 = y + img_size.y;
                    } else if(y >= img_size.y) {
                        y1 = y - img_size.y;
                    } else {
                        y1 = y;
                    }

                    img->setPixel(x1, y1, color);
                }
            }
            for(; y < points[2].y; y++) {
                for(int x = round(get_value_y(slope2, points[2], y)); x < round(get_value_y(slope3, points[1], y)); x++) {
                    if(x < 0) {
                        x1 = x + img_size.x;
                    } else if(x >= img_size.x) {
                        x1 = x - img_size.x;
                    } else {
                        x1 = x;
                    }

                    if(y < 0) {
                        y1 = y + img_size.y;
                    } else if(y >= img_size.y) {
                        y1 = y - img_size.y;
                    } else {
                        y1 = y;
                    }

                    img->setPixel(x1, y1, color);
                }
            }
        }
    } else {
        sort(points.begin(), points.end(), point_sort_y);
        double slope1 = get_slope(points[0], points[1]);
        double slope2 = get_slope(points[0], points[2]);
        double slope3 = get_slope(points[2], points[1]);

        int y = round(points[0].y);
        if(1 / slope1 < 1 / slope2) {
            for(; y < points[1].y; y++) {
                for(int x = round(get_value_y(slope1, points[1], y)); x < round(get_value_y(slope2, points[2], y)); x++) {
                    if(x >= 0 && x < img_size.x && y >= 0 && y < img_size.y) img->setPixel(x, y, color);
                }
            }
            for(; y < points[2].y; y++) {
                for(int x = round(get_value_y(slope3, points[1], y)); x < round(get_value_y(slope2, points[2], y)); x++) {
                    if(x >= 0 && x < img_size.x && y >= 0 && y < img_size.y) img->setPixel(x, y, color);
                }
            }
        } else {
            for(; y < points[1].y; y++) {
                for(int x = round(get_value_y(slope2, points[2], y)); x < round(get_value_y(slope1, points[1], y)); x++) {
                    if(x >= 0 && x < img_size.x && y >= 0 && y < img_size.y) img->setPixel(x, y, color);
                }
            }
            for(; y < points[2].y; y++) {
                for(int x = round(get_value_y(slope2, points[2], y)); x < round(get_value_y(slope3, points[1], y)); x++) {
                    if(x >= 0 && x < img_size.x && y >= 0 && y < img_size.y) img->setPixel(x, y, color);
                }
            }
        }
    }   
}

int main() {
    sf::Clock clock;

    PlanetBiomeGenerator pgen;
    
    biome_map_terra.loadFromFile("biome_map_terra.png");

    RNG rng;
    rng.set_seed(0.87468, 45.02578);

    for(int t = 0; t < 1; t++) {
        float seed = rng.retrieve();

        pgen.set_parameters(0.7707326, 6, 3, 7, 1.5, 4, 4, 3, 1.5, 4, 4, 4, 1.3);

        //sf::Vector2u hexagons(round(100 * 2 / sqrt(3)), 50);//100 * 0.375);
        //sf::Vector2u triangles(hexagons.x / 2 * 3, hexagons.y * 2);
        sf::Vector2u img_size(19600 / 8, 19600 / 16);

        sf::Image img;

        img.create(img_size.x, img_size.y);

        //
        //
        
        for(int x = 0; x < img_size.x; x++) {
            for(int y = 0; y < img_size.y; y++) {
                /*if(x < 200 && y < 200) {
                    float color = (noise.GetNoise(float(x), float(y)) + 1) / 2;
                    img.setPixel(x, y, sf::Color(color * 255, color * 255, color * 255));
                }*/

                img.setPixel(x, y, pgen.get_biome(x, y, (img_size.x), (img_size.y)));
            }
        }

        /*std::vector<sf::Vector2<double>> points;
        for(int j = 0; j < triangles.y; j++) {
            for(int i = 0; i < triangles.x; i++) {
                if((j % 2 == 1 && i % 3 == 1) || (j % 2 == 0 && i % 3 == 2)) {
                    points.push_back(sf::Vector2<double>((i - 0.25 + 0.5 * (j % 2 == 0)) * (double(img_size.x) / triangles.x), j * (double(img_size.y) / triangles.y)));
                } else {
                    points.push_back(sf::Vector2<double>((i + (0.1 + 0.8 * rng.retrieve()) - 0.25 + 0.5 * (j % 2 == 0)) * (double(img_size.x) / triangles.x), (j + (0.1 + 0.8 * rng.retrieve())) * (double(img_size.y) / triangles.y)));
                }
            }
        }

        std::vector<std::pair<Triangle, sf::Color>> tri_vec;
        
        int x = triangles.x;
        int y = triangles.y;

        sf::Vector2<double> flip_x(img_size.x, 0);
        sf::Vector2<double> flip_y(0, img_size.y);

        for(int i = 0; i < triangles.x * triangles.y; i++) {
            double num = rng.retrieve();
            double col = pgen.pnf1.retrieve(points[i].x, points[i].y, img_size.x, img_size.y);
            sf::Color color = pgen.get_biome(points[i].x, points[i].y, img_size.x, img_size.y);
            
            if(i % (triangles.x * 2) >= triangles.x && i % 3 == 1) {
                if(i >= triangles.x * (triangles.y - 1)) {
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x - 1], points[i - x], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x], points[i + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1], points[i % x] + flip_y, true), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i % x] + flip_y, points[i % x - 1] + flip_y, true), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i % x - 1] + flip_y, points[i - 1], true), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[i - x - 1], false), color));
                } else {
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x - 1], points[i - x], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x], points[i + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1], points[i + x], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i + x - 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x - 1], points[i - 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[i - x - 1], false), color));
                }
                
            } else if(i % (triangles.x * 2) < triangles.x && i % 3 == 2) {
                if(i >= triangles.x * (triangles.y - 1)) {

                } else if(i % triangles.x == triangles.x - 1) {
                    if(!(i == triangles.x - 1)) {*/
                        /*if(points[i].x >= img_size.x) {
                            color = pgen.get_biome(points[i].x - img_size.x, points[i].y, img_size.x, img_size.y);

                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i - x], points[i - (2 * x) + 1] - flip_x, true), color));
                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i - (2 * x) + 1] - flip_x, points[i - x + 1] - flip_x, true), color));
                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i - x + 1] - flip_x, points[i + 1] - flip_x, true), color));
                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i + 1] - flip_x, points[i + x], true), color));
                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i + x], points[i - 1], true), color));
                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i - 1], points[i - x], true), color));
                        } else {
                            color = pgen.get_biome(points[i].x, points[i].y, img_size.x, img_size.y);*/

                        /*tri_vec.push_back(std::pair(Triangle(points[i], points[i - x], points[i - (2 * x) + 1] + flip_x, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i - (2 * x) + 1] + flip_x, points[i - x + 1] + flip_x, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i - x + 1] + flip_x, points[i + 1] + flip_x, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1] + flip_x, points[i + x], true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i - 1], false), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[i - x], false), color));
                        //}
                    } else {
                        tri_vec.push_back(std::pair(Triangle(points[i], points[x * y - 1] - flip_y, points[x * (y - 1)] + flip_x - flip_y, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[x * (y - 1)] + flip_x - flip_y, points[0] + flip_x, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[0] + flip_x, points[x] + flip_x, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[x] + flip_x, points[i + x], true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i - 1], false), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[x * y - 1] - flip_y, true), color));
                    }
                } else if(i < triangles.x) {
                    tri_vec.push_back(std::pair(Triangle(points[i], points[x * y - (x - i)] - flip_y, points[x * y - (x - i) + 1] - flip_y, true), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[x * y - (x - i) + 1] - flip_y, points[i + 1], true), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1], points[i + x + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x + 1], points[i + x], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i - 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[x * y - (x - i)] - flip_y, true), color));
                } else {
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x], points[i - x + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x + 1], points[i + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1], points[i + x + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x + 1], points[i + x], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i - 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[i - x], false), color));
                }
                
            }
        }

        for(std::pair<Triangle, sf::Color> pair : tri_vec) {*/
            /*if(pair.first.points[0].x < img_size.x && pair.first.points[1].x < img_size.x, pair.first.points[2].x < img_size.x)*/ 
            /*pair.first.rasterize(&img, pair.second, img_size);
        }

        std::vector<int> adjacent_indices;*/

        /*for(int i = 0; i < 20; i++) {
            int index = floor(rng.retrieve() * triangles.x * triangles.y);
            while(!((index % (triangles.x * 2) < triangles.x && (index % 3 == 0 || index % 3 == 1)) || (index % (triangles.x * 2) >= triangles.x && (index % 3 == 0 || index % 3 == 2))) || !(pgen.pnf1.retrieve(points[index].x, points[index].y, img_size.x, img_size.y) > 4.0/7)) {
                index = floor(rng.retrieve() * triangles.x * triangles.y);
            }

            int former_index = -1;
            adjacent_indices.clear();

            for(int j = 0; j < 100; j++) {
                if(index % (triangles.x * 2) < triangles.x) {
                    if(index % 3 == 0) {
                        adjacent_indices.push_back(index + triangles.x);
                        adjacent_indices.push_back(index + 1);
                        if(index < triangles.x) {
                            //adjacent_indices.push_back((triangles.x - 1) * triangles.y + index);
                        } else {
                            adjacent_indices.push_back(index - triangles.x);
                        }
                    } else if(index % 3 == 1) {
                        adjacent_indices.push_back(index + triangles.x + 1);
                        adjacent_indices.push_back(index - 1);
                        if(index < triangles.x) {
                            //adjacent_indices.push_back((triangles.x - 1) * triangles.y + index + 1);
                        } else {
                            adjacent_indices.push_back(index - triangles.x + 1);
                        }
                    }
                } else {
                    if(index % 3 == 0) {
                        adjacent_indices.push_back(index - triangles.x);

                        if(index >= triangles.x * (triangles.y - 1)) {
                            adjacent_indices.push_back(index % triangles.x);
                        } else {
                            adjacent_indices.push_back(index + triangles.x);
                        }

                        if(index % triangles.x == 0) {
                            adjacent_indices.push_back(index + triangles.x - 1);
                        } else {
                            adjacent_indices.push_back(index - 1);
                        }
                    } else if(index % 3 == 2) {
                        adjacent_indices.push_back(index - triangles.x - 1);

                        if(index >= triangles.x * (triangles.y - 1)) {
                            adjacent_indices.push_back(index % triangles.x - 1);
                        } else {
                            adjacent_indices.push_back(index + triangles.x - 1);
                        }

                        if(index % triangles.x == triangles.x - 1) {
                            adjacent_indices.push_back(index - triangles.x + 1);
                        } else {
                            adjacent_indices.push_back(index + 1);
                        }
                    }
                }

                if(former_index != -1) std::remove(adjacent_indices.begin(), adjacent_indices.end(), former_index);

                double lowest_height = 1;
                int new_index = -1;
                double current_height;
                for(int adjacent_index : adjacent_indices) {
                    current_height = pgen.pnf1.retrieve(points[adjacent_index].x, points[adjacent_index].y, img_size.x, img_size.y);
                    
                    if(current_height < lowest_height) {
                        lowest_height = current_height;
                        new_index = adjacent_index;
                    }
                }

                double active_height = pgen.pnf1.retrieve(points[index].x, points[index].y, img_size.x, img_size.y);

                if(active_height < lowest_height) break;
                if(lowest_height < 5.0/9) break;
                rasterize_line(points[new_index], points[index], &img, pgen.biome_lookup_table.at(19));

                //for(int adj_index : adjacent_indices) {
                //    rasterize_line(points[adj_index], points[index], &img, sf::Color(255, 0, 0));
                //}
                //rasterize_line(points[adjacent_indices[1]], points[index], &img, sf::Color(255, 0, 0));
                //rasterize_line(points[adjacent_indices[2]], points[index], &img, sf::Color(255, 0, 0));
                former_index = index;
                index = new_index;
            }
        }*/

        //rasterize_line(sf::Vector2<double>(100, 100), sf::Vector2<double>(200, 400), &img, pgen.biome_lookup_table.at(19));

        /*for(int i = 0; i < 1000; i++) {
            int x = rng.retrieve() * img_size.x;
            int y = rng.retrieve() * img_size.y;
            float height = pgen.pnf1.retrieve(x, y, img_size.x, img_size.y);
            bool continue_ = true;

            for(int j = 0; j < 1000; j++) {
                if(height > (4/7) && continue_) {
                    img.setPixel(x, y, pgen.biome_lookup_table.at(19));
                    if(x <= 1 || x >= img_size.x - 1 || y <= 1 || y >= img_size.y - 1) {
                        continue_ = false;
                    } else {
                        float height_n = pgen.pnf1.retrieve(x, y - 1, img_size.x, img_size.y);
                        float height_s = pgen.pnf1.retrieve(x, y + 1, img_size.x, img_size.y);
                        float height_e = pgen.pnf1.retrieve(x + 1, y, img_size.x, img_size.y);
                        float height_w = pgen.pnf1.retrieve(x - 1, y, img_size.x, img_size.y);

                        if(height_n >= height_s && height_n >= height_e && height_n >= height_w) {
                            y -= 1;
                            height = height_n;
                        } else if(height_s >= height_n && height_s >= height_e && height_s >= height_w) {
                            y += 1;
                            height = height_s;
                        } else if(height_e >= height_n && height_e >= height_s && height_e >= height_w) {
                            x += 1;
                            height = height_e;
                        } else if(height_w >= height_n && height_w >= height_s && height_w >= height_e) {
                            x -= 1;
                            height = height_w;
                        } else {
                            continue_ = false;
                        }
                    }
                }
            }
        }*/
        
        img.saveToFile("map" + std::to_string(t + 203) + "x.png");
        std::cout << std::to_string(t) + " " + std::to_string(seed) << "\n";
    }
    
    printf("%f", clock.getElapsedTime().asSeconds());
}