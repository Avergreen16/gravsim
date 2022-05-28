#define _USE_MATH_DEFINES
#include <SFML-2.5.1\include\SFML\Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>

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

    std::vector<std::vector<double>> vector_container;

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
        {7, sf::Color(0x287049FF)}, // boreal forest
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
        //{, sf::Color(60, 60, 200, 255)}, // river
    };

    void set_parameters(double p_seed, int p_width1, int p_height1, int p_octaves1, double p_persistance1, int p_width2, int p_height2, int p_octaves2, double p_persistance2, int p_width3, int p_height3, int p_octaves3, double p_persistance3);

    sf::Color get_biome(int x, int y, int width, int height) {
        int v1 = floor(pnf1.retrieve(x, y, width, height) * 7);
        int v2 = floor(clamp(((pnf2.retrieve(x, y, width, height) - 0.5) * 0.125 + std::abs((double)y / height - 0.5)) * 8, 0, 3));
        int v3 = floor(pnf3.retrieve(x, y, width, height) * 8);;

        return biome_lookup_table.at(terra_biome_vector[v1][v2][v3]);
    }
};

double dotproduct(std::vector<double> v1, std::vector<double> v2) {
    return v1[0] * v2[0] + v1[1] * v2[1];
}

double interpolate(double x1, double x2, double position) {
    double returnvalue = (x2 - x1) * (6 * pow(position, 5) - 15 * pow(position, 4) + 10 * pow(position, 3)) + x1;
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
        std::vector<double> insertvector = {std::cos(theta), std::sin(theta)};
        vector_container.push_back(insertvector);
    }
}

double PerlinNoiseFactory::retrieve(int x, int y, int p_width, int p_height) {
    double returnvalue = 0;
    for(int i = 0; i < octaves; i++) {
        //updated scale
        int i_power = pow(2, i);
        double updated_xscale = ((double)p_width / main_vector_width) / i_power;
        double updated_yscale = ((double)p_height / main_vector_height) / i_power;

        // stores the point's 4 dot products for interpolation
        double dotproducts[4];

        // grabs the coordinate value of the top left vector of the 4 surrounding vectors in the "vector array", each vector gets an
        // integer coordinate. These values are used to determine the vector's key
        int topleft_x = floor((double)x / updated_xscale);
        int topleft_y = floor((double)y / updated_yscale);
        
        for(int j = 0; j < 4; j++) {
            // vector_x and vector_y are the x and y integer coordinates of the active vector, determined by j, in the "vector array"
            int vector_x = topleft_x + j % 2;
            int vector_y = topleft_y + floor(j / 2);

            // these are the true coordinates of the active vector so that the distance vector between the 
            // active vector's origin and the point's x and y can be calculated
            int pos_value_x = vector_x * updated_xscale;
            int pos_value_y = vector_y * updated_yscale;

            // these loops move vector_x and vector_y into the width and height range so the pattern can loop.
            // it only runs if the loop variables are true
            // it's separate for x and y so that noise that loops in only one axis is possible
            if(loop_x) {
                while(vector_x >= main_vector_width * i_power) {
                    vector_x -= main_vector_width * i_power;
                }
                while(vector_x < 0) {
                    vector_x += main_vector_width * i_power;
                }
            }
            if(loop_y) {
                while(vector_y >= main_vector_height * i_power) {
                    vector_y -= main_vector_height * i_power;
                }
                while(vector_y < 0) {
                    vector_y += main_vector_height * i_power;
                }
            }

            // creates vector key for lookup in the vector map
            int vector_key = vector_x + vector_y * main_vector_width * pow(2, i);

            // will create a vector if there isn't already one with the same key, and will store it in the 
            // vector map with that key. The key corresponds the the x and y position of the vector in the "vector array"
            /*if(vector_container.find(vector_key) == vector_container.end()) {
                double theta = rng.retrieve() * 2 * M_PI;
                std::vector<double> insertvector = {std::cos(theta), std::sin(theta)};
                vector_container.insert({vector_key, insertvector});
            }*/

            // vector of active point, current active point is one of the 4 closest vectors, this variable is assigned each of the
            // 4 vectors as the j loop runs.
            std::vector<double> active_vector = vector_container[vector_key];
            // distance vector between true position of active vector's origin and x and y coordinate
            std::vector<double> coordinate_distance_vector = {((double)x - pos_value_x) / updated_xscale, ((double)y - pos_value_y) / updated_yscale};

            dotproducts[j] = dotproduct(active_vector, coordinate_distance_vector);
        }

        // interpolates the dot products bilineraly and returns the result
        //(double)(coordinate % scale) / scale
        double x1 = interpolate(dotproducts[0], dotproducts[1], fmod((double)x, updated_xscale) / updated_xscale);
        double x2 = interpolate(dotproducts[2], dotproducts[3], fmod((double)x, updated_xscale) / updated_xscale);
        returnvalue += (interpolate(x1, x2 , fmod((double)y, updated_yscale) / updated_yscale) + M_SQRT1_2) / (M_SQRT1_2 * 2) * pow(persistance, -i);
    }
    return normalCDF(returnvalue / maxvalue, sd, mean);
}

void PlanetBiomeGenerator::set_parameters(double p_seed, int p_width1, int p_height1, int p_octaves1, double p_persistance1, int p_width2, int p_height2, int p_octaves2, double p_persistance2, int p_width3, int p_height3, int p_octaves3, double p_persistance3) {
    pnf1.set_parameters(p_seed, p_seed * 45.18754874623, p_width1, p_height1, p_octaves1, p_persistance1, 0.15, 0.5, true, true);
    pnf2.set_parameters(p_seed, p_seed * 21.1324254354, p_width2, p_height2, p_octaves2, p_persistance2, 0.2, 0.5, true, true);
    pnf3.set_parameters(p_seed, p_seed * 57.62634832648, p_width3, p_height3, p_octaves3, p_persistance3, 0.2, 0.5, true, true);
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

struct Triangle {
    std::vector<sf::Vector2<double>> points;

    Triangle(sf::Vector2<double> p1, sf::Vector2<double> p2, sf::Vector2<double> p3) {
        points.push_back(p1);
        points.push_back(p2);
        points.push_back(p3);
    }

    void rasterize(sf::Image* img, sf::Color color);
};

void Triangle::rasterize(sf::Image* img, sf::Color color) {
    sort(points.begin(), points.end(), point_sort_y);
    double slope1 = get_slope(points[0], points[1]);
    double slope2 = get_slope(points[0], points[2]);
    double slope3 = get_slope(points[2], points[1]);

    int y = round(points[0].y);
    if(1 / slope1 < 1 / slope2) {
        for(; y < points[1].y; y++) {
            for(int x = round(get_value_y(slope1, points[1], y)); x < round(get_value_y(slope2, points[2], y)); x++) {
                img->setPixel(x, y, color);
            }
        }
        for(; y < points[2].y; y++) {
            for(int x = round(get_value_y(slope3, points[1], y)); x < round(get_value_y(slope2, points[2], y)); x++) {
                img->setPixel(x, y, color);
            }
        }
    } else {
        for(; y < points[1].y; y++) {
            for(int x = round(get_value_y(slope2, points[2], y)); x < round(get_value_y(slope1, points[1], y)); x++) {
                img->setPixel(x, y, color);
            }
        }
        for(; y < points[2].y; y++) {
            for(int x = round(get_value_y(slope2, points[2], y)); x < round(get_value_y(slope3, points[1], y)); x++) {
                img->setPixel(x, y, color);
            }
        }
    }
}

int main() {
    PlanetBiomeGenerator pgen;

    RNG rng;
    rng.set_seed(0.565869, 54.843648732);

    pgen.set_parameters(rng.retrieve(), 6, 3, 5, 1.6, 4, 4, 1, 1, 4, 4, 1, 1);

    sf::Vector2u triangles(120, 60);
    sf::Vector2u img_size(1000, 500);

    sf::Image img;

    img.create(img_size.x, img_size.y);

    std::vector<sf::Vector2<double>> points;
    for(int j = 0; j < triangles.y; j++) {
        for(int i = 0; i < triangles.x; i++) {
            if((j % 2 == 1 && i % 3 == 1) || (j % 2 == 0 && i % 3 == 2)) {
                points.push_back(sf::Vector2<double>((i + 0.5 * (j % 2 == 0)) * (double(img_size.x) / triangles.x), j * (double(img_size.y) / triangles.y)));
            } else {
                points.push_back(sf::Vector2<double>((i + (0.1 + 0.8 * rng.retrieve()) - 0.25 + 0.5 * (j % 2 == 0)) * (double(img_size.x) / triangles.x), (j + (0.1 + 0.8 * rng.retrieve())) * (double(img_size.y) / triangles.y)));
            }
        }
    }

    std::vector<std::pair<Triangle, sf::Color>> tri_vec;
    int x = triangles.x;
    for(int i = triangles.x; i < triangles.x * triangles.y; i++) {
        sf::Color color = pgen.get_biome(points[i].x, points[i].y, img_size.x, img_size.y);
        
        if((i % (triangles.x * 2) >= triangles.x && i % 3 == 1) && i < triangles.x * (triangles.y - 1)) {
            tri_vec.push_back(std::pair(Triangle(points[i], points[i - x - 1], points[i - x]), color));
            tri_vec.push_back(std::pair(Triangle(points[i], points[i - x], points[i + 1]), color));
            tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1], points[i + x]), color));
            tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i + x - 1]), color));
            tri_vec.push_back(std::pair(Triangle(points[i], points[i + x - 1], points[i - 1]), color));
            tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[i - x - 1]), color));
        }

        if((i % (triangles.x * 2) < triangles.x && i % 3 == 2 && i % triangles.x != (triangles.x - 1) && i < triangles.x * (triangles.y - 1))) {
            tri_vec.push_back(std::pair(Triangle(points[i], points[i - x], points[i - x + 1]), color));
            tri_vec.push_back(std::pair(Triangle(points[i], points[i - x + 1], points[i + 1]), color));
            tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1], points[i + x + 1]), color));
            tri_vec.push_back(std::pair(Triangle(points[i], points[i + x + 1], points[i + x]), color));
            tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i - 1]), color));
            tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[i - x]), color));
        }
    }

    for(std::pair<Triangle, sf::Color> pair : tri_vec) {
        /*if(pair.first.points[0].x < img_size.x && pair.first.points[1].x < img_size.x, pair.first.points[2].x < img_size.x)*/ pair.first.rasterize(&img, pair.second);
    }
    
    img.saveToFile("triangle0.png");
    
    printf("0");
}