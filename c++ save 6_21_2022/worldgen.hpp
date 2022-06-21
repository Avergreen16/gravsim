#define _USE_MATH_DEFINES
//#include <SFML-2.5.1\include\SFML\Graphics.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cmath>
#include <vector>

int width, height, nrChannels;
unsigned char *biome_map_terra = stbi_load("biome_map_terra.png", &width, &height, &nrChannels, 4);

uint32_t get_pixel(unsigned char* data, std::array<int, 2> pixel, int image_width) {
    int pixel_index = pixel[0] + pixel[1] * image_width;
    return 0x01000000 * data[pixel_index * 4] + 0x00010000U * data[pixel_index * 4 + 1] + 0x00000100U * data[pixel_index * 4 + 2] + 0xFFU;
}

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

double clamp(double input, double min, double max) {
    return std::max(std::min(input, max), min);
}

double normalCDF(double input, double sd, double mean) {
    double value = (input - mean) / sd;
    return 0.5 * erfc(-value * M_SQRT1_2);
}

double dotproduct(std::array<double, 2> v1, std::array<double, 2> v2) {
    return v1[0] * v2[0] + v1[1] * v2[1];
}

double interpolate(double x1, double x2, double pos) {
    double returnvalue = (x2 - x1) * (6 * pos * pos * pos * pos * pos - 15 * pos * pos * pos * pos + 10 * pos * pos * pos) + x1;
    return returnvalue;
}

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

    std::vector<std::array<double, 2>> vector_container;

    void set_parameters(double p_seed, double p_factor, int p_main_vector_width, int p_main_vector_height, int p_octaves, double p_persistance, double p_sd, double p_mean, bool p_loop_x, bool p_loop_y);

    void generate_vectors();

    double retrieve(double x, double y, double width, double height);
};

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
        std::array<double, 2> insertvector = {std::cos(theta), std::sin(theta)};
        vector_container.push_back(insertvector);
    }
}

double PerlinNoiseFactory::retrieve(double x, double y, double p_width, double p_height) {
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
            std::array<double, 2> active_vector = vector_container[vector_key];
            // distance vector between true position of active vector's origin and x and y coordinate
            std::array<double, 2> coordinate_distance_vector = {(double(x) - pos_value_x) / updated_xscale, (double(y) - pos_value_y) / updated_yscale};
            //if(coordinate_distance_vector[0] == 0 || coordinate_distance_vector[1] == 0) printf("check");

            dotproducts[j] = dotproduct(active_vector, coordinate_distance_vector);
        }

        // interpolates the dot products bilineraly and returns the result
        //(double)(coordinate % scale) / scale
        double x1 = interpolate(dotproducts[0], dotproducts[1], fmod(double(x), updated_xscale) / updated_xscale);
        double x2 = interpolate(dotproducts[2], dotproducts[3], fmod(double(x), updated_xscale) / updated_xscale);
        returnvalue += (interpolate(x1, x2 , fmod(double(y), updated_yscale) / updated_yscale) + M_SQRT1_2) / (M_SQRT1_2 * 2) * pow(persistance, -i);
    }
    if(sd != 0) return normalCDF(returnvalue / maxvalue, sd, mean);
    return returnvalue / maxvalue;
}

struct Worldgen {
    PerlinNoiseFactory pfac1;
    PerlinNoiseFactory pfac2;
    PerlinNoiseFactory pfac3;
    RNG rng;

    Worldgen(double p_seed, int p_width1, int p_height1, int p_octaves1, double p_persistance1, int p_width2, int p_height2, int p_octaves2, double p_persistance2, int p_width3, int p_height3, int p_octaves3, double p_persistance3) {
        pfac1.set_parameters(p_seed, p_seed * 45.18754874623, p_width1, p_height1, p_octaves1, p_persistance1, 0.15, 0.5, true, true);
        pfac2.set_parameters(p_seed, p_seed * 21.1324254354, p_width2, p_height2, p_octaves2, p_persistance2, 0.2, 0.5, true, true);
        pfac3.set_parameters(p_seed, p_seed * 57.62634832648, p_width3, p_height3, p_octaves3, p_persistance3, 0.15, 0.5, true, true);
        pfac1.generate_vectors();
        pfac2.generate_vectors();
        pfac3.generate_vectors();
        rng.set_seed(p_seed, p_seed * 60.5564301);
    }

    uint32_t retrieve(std::array<int, 2> location, std::array<int, 2> map_size) {
        double elevation = pfac1.retrieve(location[0], location[1], map_size[0], map_size[1]) * 12;
        double temperature = clamp((pfac2.retrieve(location[0], location[1], map_size[0], map_size[1]) - 0.5) * 0.125 + (0.5 - std::abs(double(location[1]) / map_size[1] - 0.5)) * 24, 0, 11.999);
        double humidity = pfac3.retrieve(location[0], location[1], map_size[0], map_size[1]) * 12;

        uint32_t biome = get_pixel(biome_map_terra, {floor(elevation) * 12 + floor(humidity), floor(temperature)}, 144);
        if(elevation > 7 && elevation <= 7.03 && biome != 0xE59647FF && biome != 0xF4ECFFFF && biome != 0x2D541BFF && temperature >= 3) biome = 0xFFFBB3FF;
        //printf("%u ", biome);
        return biome;
    }
};

std::array<int, 2> get_floor_tile(uint32_t biome, Worldgen* worldgen) {
    double rand;
    int block_type;
    int texture;
    switch(biome) {
        case 0xfffbb3ff:
            block_type = 1;
            rand = worldgen->rng.retrieve();
            if(rand < 0.25) texture = 24;
            else texture = floor(rand * 5.333 + 23.667);
            break;
        
        case 0x8bce50ff:
            block_type = 2;
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) texture = floor(rand * 4) + 1;
            else texture = floor(rand * 10) - 2;
            break;

        case 0x008944ff:
            block_type = 2;
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) texture = floor(rand * 4) + 9;
            else texture = floor(rand * 10) + 6;
            break;
        
        case 0x286059ff:
            block_type = 2;
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) texture = floor(rand * 4) + 16;
            else texture = floor(rand * 10) + 13;
            break;
        
        case 0xe2dc98ff:
            block_type = 1;
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) texture = 32;
            if(rand < 0.9) texture = floor(rand * 5 + 30.5);
            else texture = floor(rand * 20) + 17;
            break;
        
        case 0xe59647ff:
            block_type = 1;
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) texture = 40;
            if(rand < 0.9) texture = floor(rand * 5 + 38.5);
            else texture = floor(rand * 20) + 25;
            break;
        
        case 0xc8d16aff:
            block_type = 2;
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) texture = floor(rand * 4) + 48;
            else texture = floor(rand * 10) + 45;
            break;
        
        case 0x00b539ff:
            block_type = 2;
            texture = floor(worldgen->rng.retrieve() * 5) + 10;
            break;
        
        case 0x191fd3ff:
            block_type = 3;
            texture = 56;
            break;
        
        default:
            block_type = 0;
            texture = 8;
            break;
    }
    return {block_type, texture};
}

std::array<int, 2> get_object_tile(uint32_t biome, Worldgen* worldgen) {
    double rand;
    switch(biome) {
        
        case 0x8bce50ff:
        case 0xc8d16aff:
            rand = worldgen->rng.retrieve();
            if(rand < 0.2) return {4, floor(rand * 15) + 45};
            return {0, 0};
        
        default:
            return {0, 0};
    }
}