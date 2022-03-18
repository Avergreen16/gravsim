#define _USE_MATH_DEFINES
#include <string>
#include <cmath>
#include <map>
#include <iostream>
#include <FreeImage\Dist\x64\FreeImage.h>

#include "Noisefactory.hpp"

RNG::RNG(float p_seed, float p_factor) {
    seed = p_seed;
    factor = p_factor;
    rng_number = p_seed;
}

float RNG::retrieve(int decimal_digits) {
    rng_number = rng_number * factor;
    rng_number -= floor(rng_number);
    if(decimal_digits == 0) {
        return rng_number;
    }
    return floor(rng_number * pow(10, decimal_digits)) / pow(10, decimal_digits);
}

void ValueNoiseFactory::generate_values() {
    int array_width = width * pow(2, octaves);
    int array_height = height * pow(2, octaves);
    for(int k = 0; k < array_width * array_height; k++) {
        int value_key = k;
        origin_value_map.insert({value_key, rng.retrieve()});  
    }
}

ValueNoiseFactory::ValueNoiseFactory(float p_seed, int p_width, int p_height, int p_scale, int p_octaves, int p_persistance, bool p_loop_x, bool p_loop_y, float p_multiplier, float p_offset) {
    seed = p_seed;
    width = p_width;
    height = p_height;
    scale = p_scale;
    octaves = p_octaves;
    persistance = p_persistance;
    loop_x = p_loop_x;
    loop_y = p_loop_y;
    multiplier = p_multiplier;
    offset = p_offset;
    rng = RNG(p_seed, 43.376587);

    maxvalue = 0;
    // sets maxvalue to the paximum possible value that can be generated, given the octaves and persistance
    // this is used to normalize return values to between 0 and 1
    for(int i = 0; i < octaves; i++) {
        maxvalue += (1 * multiplier - offset) * pow(persistance, -i);
    }
}

float ValueNoiseFactory::pullvalue(int x, int y) {
    float returnvalue = 0;
    for(int i = 0; i < octaves; i++) {
        // stores the 4 surrounding values
        float values[4];

        // retrieves the x and y coordinates of the top left of the nearest 4 values in the "value array"
        // similar to the "vector array" in PerlinNoiseFactory, only it stores values instead of vectors
        // the value array isn't actually real, but the coordinates are used to retrieve the value from the map where it is stored
        // Each value is paired with a string key that identifies its position in the "value array"
        int topleft_x = floor(x / (scale / pow(2, i)));
        int topleft_y = floor(y / (scale / pow(2, i)));

        for(int j = 0; j < 4; j++) {
            // calculates the x and y position in the "value array" of the active value
            // the active value changes with the j counter variable, for each of the 4 closest values
            int value_x = topleft_x + j % 2;
            int value_y = topleft_y + floor(j / 2);

            // these loops move value_x and value_y into the width and height range so the pattern can loop
            // it only runs if the loop variables are true
            // it's separate for x and y so that noise that loops in only one axis is possible
            if(loop_x) {
                while(value_x >= width * pow(2, i)) {
                    value_x -= width * pow(2, i);
                }
                while(value_x < 0) {
                    value_x += width * pow(2, i);
                }
            }
            if(loop_y) {
                while(value_y >= height * pow(2, i)) {
                    value_y -= height * pow(2, i);
                }
                while(value_y < 0) {
                    value_y += height * pow(2, i);
                }
            }

            // creates value key for lookup in the value map
            int value_key = value_x + value_y * width;

            // will create a value if there isn't already one with the same key, and will store it in the 
            // value map with that key. The key corresponds the the x and y position of the value in the "value array"
            if(origin_value_map.find(value_key) == origin_value_map.end()) {
                origin_value_map.insert({value_key, rng.retrieve()});
            }

            // retrieves active value and puts it in storage array, will be each of the 4 closest vectors 
            // to point x, y for each j loop iteration.
            values[j] = origin_value_map[value_key];
        }
        // bilinearly interpolates the values and adds the result to returnvalue
        float x1 = interpolate(values[0], values[1], x, scale / pow(2, i));
        float x2 = interpolate(values[2], values[3], x, scale / pow(2, i));
        returnvalue += interpolate(x1, x2 , y, scale / pow(2, i)) * pow(persistance, -i);
    }
    // divides returnvalue by maxvalue before returning to normalize the resulting value to between 0 and 1 
    returnvalue = returnvalue * multiplier - offset;
    returnvalue /= maxvalue;
    if(returnvalue < 0) {
        return 0;
    } else if(returnvalue > 1) {
        return 1;
    } else {
        return returnvalue;
    }
}

float ValueNoiseFactory::interpolate(float x1, float x2, int coordinate, int scale) {
    float scaled_coordinate = (float)(coordinate % scale) / scale;
    float returnvalue = (x2 - x1) * (6 * pow(scaled_coordinate, 5) - 15 * pow(scaled_coordinate, 4) + 10 * pow(scaled_coordinate, 3)) + x1;
    return returnvalue;
}

PerlinNoiseFactory::PerlinNoiseFactory(float p_seed, float p_factor, int p_width, int p_height, int p_scale, int p_octaves, int p_persistance, bool p_loop_x, bool p_loop_y, float p_multiplier, float p_offset) {
    seed = p_seed;
    width = p_width;
    height = p_height;
    scale = p_scale;
    octaves = p_octaves;
    persistance = p_persistance;
    loop_x = p_loop_x;
    loop_y = p_loop_y;
    offset = p_offset;
    multiplier = p_multiplier;
    rng = RNG(p_seed, p_factor);

    maxvalue = 0;
    for(int i = 0; i < octaves; i++) {
        maxvalue += (1 * multiplier - offset) * pow(persistance, -i);
    }
}

void PerlinNoiseFactory::generate_vectors() {
    int array_width = width * pow(2, octaves);
    int array_height = height * pow(2, octaves);
    for(int k = 0; k < array_width * array_height; k++) {
        int vector_key = k;
        float theta = rng.retrieve() * 2 * M_PI;
        std::vector<float> insertvector = {std::cos(theta), std::sin(theta)};
        origin_vector_map.insert({vector_key, insertvector});   
    }
}

void PerlinNoiseFactory::generate_vectors2() {
    int array_width = width * pow(2, octaves);
    int array_height = height * pow(2, octaves);
    for(int k = 0; k < array_width * array_height; k++) {
        int vector_key = k;
        float theta = rng.retrieve() * 2 * M_PI;
        if (k == 1 || k == 17) {
            theta = 0.75 * M_PI;
        } else if (k == 2 || k == 18) {
            theta = 1.75 * M_PI;
        } else if (k == 9 || k == 25) {
            theta = 0.25 * M_PI;
        } else if (k == 10 || k == 26) {
            theta = 0.5 * M_PI;
        }/*else if (k == 18) {
            theta = 1.75 * M_PI;
        } else if (k == 9) {
            theta = 0.25 * M_PI;
        } else if (k == 10) {
            theta = 0.75 * M_PI;
        } else if (k == 1) {
            theta = 1.75 * M_PI;
        } else if (k == 2) {
            theta = 1.25 * M_PI;
        }*/
        std::vector<float> insertvector = {std::cos(theta), std::sin(theta)};
        origin_vector_map.insert({vector_key, insertvector});   
    }
}

float PerlinNoiseFactory::pullvalue(int x, int y) {
    float returnvalue = 0;
    for(int i = 0; i < octaves; i++) {
        // stores the point's 4 dot products for interpolation
        float dotproducts[4];

        // grabs the coordinate value of the top left vector of the 4 surrounding vectors in the "vector array", each vector gets an
        // integer coordinate. These values are used to determine the vector's key
        int topleft_x = floor(x / (scale / pow(2, i)));
        int topleft_y = floor(y / (scale / pow(2, i)));
        
        for(int j = 0; j < 4; j++) {
            // vector_x and vector_y are the x and y integer coordinates of the active vector, determined by j, in the "vector array"
            int vector_x = topleft_x + j % 2;
            int vector_y = topleft_y + floor(j / 2);

            // these are the true coordinates of the active vector so that the distance vector between the 
            // active vector's origin and the point's x and y can be calculated
            int pos_value_x = vector_x * (scale / pow(2, i));
            int pos_value_y = vector_y * (scale / pow(2, i));

            // these loops move vector_x and vector_y into the width and height range so the pattern can loop.
            // it only runs if the loop variables are true
            // it's separate for x and y so that noise that loops in only one axis is possible
            if(loop_x) {
                while(vector_x >= width * pow(2, i)) {
                    vector_x -= width * pow(2, i);
                }
                while(vector_x < 0) {
                    vector_x += width * pow(2, i);
                }
            }
            if(loop_y) {
                while(vector_y >= height * pow(2, i)) {
                    vector_y -= height * pow(2, i);
                }
                while(vector_y < 0) {
                    vector_y += height * pow(2, i);
                }
            }

            // creates vector key for lookup in the vector map
            int vector_key = vector_x + vector_y * width * pow(2, i);

            // will create a vector if there isn't already one with the same key, and will store it in the 
            // vector map with that key. The key corresponds the the x and y position of the vector in the "vector array"
            if(origin_vector_map.find(vector_key) == origin_vector_map.end()) {
                float theta = rng.retrieve() * 2 * M_PI;
                std::vector<float> insertvector = {std::cos(theta), std::sin(theta)};
                origin_vector_map.insert({vector_key, insertvector});
            }

            // vector of active point, current active point is one of the 4 closest vectors, this variable is assigned each of the
            // 4 vectors as the j loop runs.
            std::vector<float> active_vector = origin_vector_map[vector_key];
            // distance vector between true position of active vector's origin and x and y coordinate
            std::vector<float> coordinate_distance_vector = {((float)x - pos_value_x) / (float)(scale / pow(2, i)), ((float)y - pos_value_y) / (float)(scale / pow(2, i))};
            //coordinate_distance_vector = normalize(coordinate_distance_vector);

            dotproducts[j] = dotproduct(active_vector, coordinate_distance_vector);
        }

        // interpolates the dot products bilineraly and returns the result
        float x1 = interpolate(dotproducts[0], dotproducts[1], x, scale / pow(2, i));
        float x2 = interpolate(dotproducts[2], dotproducts[3], x, scale / pow(2, i));
        float sqrt_half = std::sqrt(0.5);
        returnvalue += (interpolate(x1, x2 , y, scale / pow(2, i)) + sqrt_half) / (sqrt_half * 2) * pow(persistance, -i);
    }
    return returnvalue / maxvalue;
}

float PerlinNoiseFactory::interpolate(float x1, float x2, int coordinate, int scale) {
    float scaled_coordinate = (float)(coordinate % scale) / scale;
    float returnvalue = (x2 - x1) * (6 * pow(scaled_coordinate, 5) - 15 * pow(scaled_coordinate, 4) + 10 * pow(scaled_coordinate, 3)) + x1;
    return returnvalue;
}

float PerlinNoiseFactory::dotproduct(std::vector<float> v1, std::vector<float> v2) {
    return v1[0] * v2[0] + v1[1] * v2[1];
}

VoronoiPoint::VoronoiPoint(int p_x, int p_y) {
    x = p_x;
    y = p_y;
}

VoronoiFactory::VoronoiFactory(float p_seed, int p_width, int p_height, int p_voronoi_map_width, int p_voronoi_map_height, bool p_loop_x, bool p_loop_y) {
    seed = p_seed;
    width = p_width;
    height = p_height;
    voronoi_map_width = p_voronoi_map_width;
    voronoi_map_height = p_voronoi_map_height;
    loop_x = p_loop_x;
    loop_y = p_loop_y;
    rng = RNG(p_seed, 23.376587);
}

void VoronoiFactory::generate_points() {
    for(int x = 0; x < voronoi_map_width; x++) {
        for(int y = 0; y < voronoi_map_height; y++) {
            float offset;
            float offsety = (float)height/voronoi_map_height/2;
            if (y % 2 == 0) {
                offset = (float)width/voronoi_map_width/2;
            } else {
                offset = 0;
            }
            int vp_x = floor(rng.retrieve() * (width / voronoi_map_width) + (width / voronoi_map_width) * x);
            int vp_y = floor(rng.retrieve() * (height / voronoi_map_height) + (height / voronoi_map_height) * y);

            VoronoiPoint pt(vp_x, vp_y);
            voronoi_map.insert({x + y * voronoi_map_width, pt});
        }
    }
}

int VoronoiFactory::pull_closest_point_key(int x, int y) {
    int topleft_x = floor(x / (width / voronoi_map_width));
    int topleft_y = floor(y / (height / voronoi_map_height));
    float dist = width * height;
    int closest_point_key;
    for(int i = -1; i < 2; i++) {
        for(int j = -1; j < 2; j++) {
            int point_x = topleft_x + i;
            int point_y = topleft_y + j;

            int edge_x = 0;
            int edge_y = 0;
            if (loop_x) {
                while (point_x < 0) {
                    point_x += voronoi_map_width;
                    edge_x -= width;
                }
                while (point_x >= voronoi_map_width) {
                    point_x -= voronoi_map_width;
                    edge_x += width;
                }
            }
            if (loop_y) {
                while (point_y < 0) {
                    point_y += voronoi_map_height;
                    edge_y -= height;
                }
                while (point_y >= voronoi_map_height) {
                    point_y -= voronoi_map_height;
                    edge_y += height;
                }
            }

            int point_key = point_x + point_y * voronoi_map_width;
            
            if (voronoi_map.find(point_key) == voronoi_map.end()) {
                int vp_x = floor(rng.retrieve() * (width / voronoi_map_width) + (width / voronoi_map_width) * point_x);
                int vp_y = floor(rng.retrieve() * (height / voronoi_map_height) + (height / voronoi_map_height) * point_y);
                VoronoiPoint pt(vp_x, vp_y);
                voronoi_map.insert({point_key, pt});
            }

            float active_point_dist = pyth(voronoi_map.at(point_key).x - x + edge_x, voronoi_map.at(point_key).y - y + edge_y);

            if (active_point_dist < dist) {
                dist = active_point_dist;
                closest_point_key = point_key;
            }
        }
    }

    return closest_point_key;
}



float VoronoiFactory::pyth(int x, int y) {
    return std::sqrt(pow(x, 2) + pow(y, 2));
}

RGBQUAD set_color(float r, float g, float b) {
    RGBQUAD color;
    color.rgbRed = r;
    color.rgbGreen = g;
    color.rgbBlue = b;
    return color;
}

RGBQUAD set_color(float v) {
    RGBQUAD color;
    color.rgbRed = v;
    color.rgbGreen = v;
    color.rgbBlue = v;
    return color;
}