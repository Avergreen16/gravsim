#define _USE_MATH_DEFINES
#include <SFML-2.5.1\include\SFML\Graphics.hpp>
#include <cmath>
#include <vector>

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

double clamp(double input, double min, double max) {
    return std::max(std::min(input, max), min);
}

double normalCDF(double input, double sd, double mean) {
    double value = (input - mean) / sd;
    return 0.5 * erfc(-value * M_SQRT1_2);
}

double dotproduct(sf::Vector2<double> v1, sf::Vector2<double> v2) {
    return v1.x * v2.x + v1.y * v2.y;
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

    std::vector<sf::Vector2<double>> vector_container;

    void set_parameters(double p_seed, double p_factor, int p_main_vector_width, int p_main_vector_height, int p_octaves, double p_persistance, double p_sd, double p_mean, bool p_loop_x, bool p_loop_y);

    void generate_vectors();

    double retrieve(double x, double y, double width, double height);
};

struct PlanetBiomeGenerator {
    PerlinNoiseFactory pnf1;
    PerlinNoiseFactory pnf2;
    PerlinNoiseFactory pnf3;

    void set_parameters(double p_seed, int p_width1, int p_height1, int p_octaves1, double p_persistance1, int p_width2, int p_height2, int p_octaves2, double p_persistance2, int p_width3, int p_height3, int p_octaves3, double p_persistance3);

    sf::Uint32 get_biome(int x, int y, int width, int height) {
        double v1 = pnf1.retrieve(x, y, width, height)/* * clamp(3.8 - hypot(double(x) / width - 0.5, double(y) / height - 0.5) * 8, 0, 1)*/ * 12; // * clamp(2.8 - 2 * hypot((double(x) / height - 1) * 2, (double(y) / height - 0.5) * 2), 0, 1.5)
        //int v2 = floor(clamp(((pnf2.retrieve(x, y, width, height) - 0.5) * 0.5 + double(height - y) / height * 4 + 0.5), 0, 3));
        double v2 = clamp(((pnf2.retrieve(x, y, width, height) - 0.5) * 0.125 + (0.5 - std::abs(double(y) / height - 0.5))) * 24, 0, 11.999);
        int v3 = floor(pnf3.retrieve(x, y, width, height) * 12);

        sf::Uint32 biome = biome_map_terra.getPixel(floor(v3) + floor(v1) * 12, floor(v2)).toInteger();
        if(v1 > 7 && v1 <= 7.05 && biome != 0xE59647FF && biome != 0xF4ECFFFF && biome != 0x2D541BFF && v2 >= 3) biome = 0xFFFBB3FF;
        //if(biome == 4 && v2 < 3.3) biome = terra_biome_vector[floor(v1)][2][v3];
        /*if(v1 > 4 && v1 <= 4.05 && biome != 0xECF4FFFF && v2 >= 3) {
            biome = 0xFFFBB3FF;
            printf("check");
        }*/
        return biome;//biome_lookup_table.at(biome);
    }
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
        sf::Vector2<double> insertvector(std::cos(theta), std::sin(theta));
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
    if(sd != 0) return normalCDF(returnvalue / maxvalue, sd, mean);
    return returnvalue / maxvalue;
}

void PlanetBiomeGenerator::set_parameters(double p_seed, int p_width1, int p_height1, int p_octaves1, double p_persistance1, int p_width2, int p_height2, int p_octaves2, double p_persistance2, int p_width3, int p_height3, int p_octaves3, double p_persistance3) {
    pnf1.set_parameters(p_seed, p_seed * 45.18754874623, p_width1, p_height1, p_octaves1, p_persistance1, 0.15, 0.5, true, true);
    pnf2.set_parameters(p_seed, p_seed * 21.1324254354, p_width2, p_height2, p_octaves2, p_persistance2, 0.2, 0.5, true, true);
    pnf3.set_parameters(p_seed, p_seed * 57.62634832648, p_width3, p_height3, p_octaves3, p_persistance3, 0.15, 0.5, true, true);
    pnf1.generate_vectors();
    pnf2.generate_vectors();
    pnf3.generate_vectors();
}

struct Worldgen {
    PlanetBiomeGenerator* biomegen;
    PerlinNoiseFactory* tilegen;
    sf::Vector2<double> tilegen_repeat;
    RNG rng;

    Worldgen(PlanetBiomeGenerator* p_biomegen, PerlinNoiseFactory* p_tilegen, sf::Vector2<double> p_tilegen_repeat) : biomegen(p_biomegen), tilegen(p_tilegen), tilegen_repeat(p_tilegen_repeat) {
        rng.set_seed(0.4330217, 47.9782349);
    }
};

int get_floor_tile(sf::Uint32 biome, Worldgen* worldgen, int x, int y) {
    double rand;
    switch(biome) {
        case 0xfffbb3ff:
            rand = worldgen->rng.retrieve();
            if(rand < 0.25) return 24;
            else return floor(rand * 5.333 + 23.667);
        
        case 0x8bce50ff:
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) return floor(rand * 4) + 1;
            else return floor(rand * 10) - 2;

        case 0x008944ff:
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) return floor(rand * 4) + 8;
            else return floor(rand * 10) + 5;
        
        case 0x286059ff:
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) return floor(rand * 4) + 16;
            else return floor(rand * 10) + 13;
        
        case 0xe2dc98ff:
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) return 32;
            if(rand < 0.9) return floor(rand * 5 + 30.5);
            else return floor(rand * 20) + 17;
        
        case 0xe59647ff:
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) return 40;
            if(rand < 0.9) return floor(rand * 5 + 38.5);
            else return floor(rand * 20) + 25;
        
        case 0xc8d16aff:
            rand = worldgen->rng.retrieve();
            if(rand < 0.5) return floor(rand * 4) + 48;
            else return floor(rand * 10) + 45;
        
        case 0x00b539ff:
            return floor(worldgen->rng.retrieve() * 5) + 10;
        
        case 0x191fd3ff:
            return 56;
        
        default:
            return 0;
    }
}

int get_floor_tile_overlay(sf::Uint32 biome, Worldgen* worldgen, int x, int y) {
    double rand;
    switch(biome) {
        
        case 0x8bce50ff:
        case 0xc8d16aff:
            rand = worldgen->rng.retrieve();
            if(rand < 0.2) return floor(rand * 15) + 45;
            return 15;
        
        default:
            return 15;
    }
}