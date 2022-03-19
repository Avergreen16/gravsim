#define FREEIMAGE_LIB
#define _USE_MATH_DEFINES
#include <iostream>
#include <ctime>
#include <cmath>
#include <vector>
#include <FreeImage\Dist\x64\FreeImage.h>
#include <bits/stdc++.h>
#include <algorithm>

RGBQUAD set_color(float r, float g, float b) {
    RGBQUAD color;
    color.rgbRed = r;
    color.rgbGreen = g;
    color.rgbBlue = b;
    return color;
}

float get_value_y(float slope, int x1, int y1, int y) {
    return (float)(y - y1) / slope + x1;
}

float get_value(float slope, int x1, int y1, int x) {
    return slope * (x - x1) + y1;
}

struct point {
    int x;
    int y;
    point () {};
    point(int p_x, int p_y) : x(p_x), y(p_y) {};
};

struct triangle {
    point* p1;
    point* p2;
    point* p3;
    triangle(point* p_p1, point* p_p2, point* p_p3) : p1(p_p1), p2(p_p2), p3(p_p3) {};
    void fill(FIBITMAP* img, RGBQUAD color);
};

struct line {
    point* p1;
    point* p2;
    line(point* p_p1, point* p_p2) : p1(p_p1), p2(p_p2) {};
    float get_length();
    float get_angle();
    void rasterize(FIBITMAP* img, RGBQUAD color);
};

struct {
    bool operator() (point* a, point* b) const {
        return a->y < b->y;
    }
} point_sort_y;

struct {
    bool operator() (point* a, point* b) const {
        return a->x < b->x;
    }
} point_sort_x;

float get_value(float slope, int x1, int y1, int x);

void fill(FIBITMAP* img, int starting_x, int starting_y, RGBQUAD color);

class RNG {
    public:
        float seed;
        float factor;
        float rng_number;

        RNG() {}
        RNG(float p_seed, float p_factor);

        float retrieve(int decimal_digits=0);
};

class PerlinNoise1 {
    public:
        int array_length;
        int scale;
        int octaves;
        float persistance;
        float maxvalue;
        bool loop;

        std::vector<float> slope_array;
        RNG rng;
        PerlinNoise1(float p_seed, float p_factor, int p_array_length, int p_scale, int p_octaves, float p_persistance, bool p_loop);
        void generate_slopes();
        float pullvalue(float x);
        float interpolate(float x1, float x2, int coordinate, int scale);
};

template <typename T>
int signum(T input) {
    if (input > 0) {
        return 1;
    }
    if (input < 0) {
        return -1;
    }
    return 0;
}




//----------------------------

int main() {
    time_t starting_time = std::time(NULL);

    char filename[] = "triangle_raster.png";
    RGBQUAD color = set_color(255, 0, 0);
    FreeImage_Initialise();
    FIBITMAP* img = FreeImage_Allocate(200, 200, 24);

    //std::vector<point> pv = {point(7, 7), point(190, 110), point(100, 190)};

    //triangle _triangle(&pv[0], &pv[1], &pv[2]);

    //_triangle.fill(img, color);

    std::vector<point> pv;
    std::vector<line> lv;

    PerlinNoise1 pn1(0.537263, 45.617239, 4, 40, 4, 1, true);
    pn1.generate_slopes();

    color = set_color(255, 255, 255);
    point p2(180, 100);
    
    for (int i = 0; i < 9; i++) {
        int value = round(pn1.pullvalue(i * 20 + 20)) + 100;
        pv.push_back(point(i * 20 + 20, value));
    }

    for (int i = 0; i < pv.size() - 1; i++) {
        point* p1 = &pv[i];
        point* p2 = &pv[i+1];
        lv.push_back(line(p1, p2));
    }

    for (line _line : lv) {
        _line.rasterize(img, color);
    }
    

    if (FreeImage_Save(FIF_PNG, img, filename)) {
        std::cout << "Image saved as " << filename << std::endl;
    }

    FreeImage_DeInitialise();
    time_t ending_time = std::time(NULL);
    std::cout << ending_time - starting_time << std::endl;
    return 0;
}

//-----------------------------



PerlinNoise1::PerlinNoise1(float p_seed, float p_factor, int p_array_length, int p_scale, int p_octaves, float p_persistance, bool p_loop) 
    : array_length(p_array_length), scale(p_scale), octaves(p_octaves), persistance(p_persistance), loop(p_loop) {
    rng = RNG(p_seed, p_factor);

    //for (int i = 0; i < p_octaves; i++) {
        //maxvalue += 1 * pow(persistance, -i);
    //}
}

void PerlinNoise1::generate_slopes() {
    for(int i = 0; i < array_length * pow(2, octaves); i++) {
        float slope = rng.retrieve() * 2 - 1;
        slope_array.push_back(slope);
    }
}

float PerlinNoise1::interpolate(float x1, float x2, int coordinate, int scale) {
    float scaled_coordinate = (float)(coordinate % scale) / scale;
    float returnvalue = (x2 - x1) * (6 * pow(scaled_coordinate, 5) - 15 * pow(scaled_coordinate, 4) + 10 * pow(scaled_coordinate, 3)) + x1;
    return returnvalue;
}

float PerlinNoise1::pullvalue(float x) {
    float returnvalue;
    for (int i = 0; i < octaves; i++) {
        int o_lsk = floor(x / (scale / pow(2, i)));
        int o_rsk = o_lsk + 1;
        int left_slope_key = o_lsk;
        int right_slope_key = o_rsk;
        if (loop) {
            while (left_slope_key < array_length * pow(2, i)) {
                left_slope_key += array_length * pow(2, i);
            }
            while (left_slope_key > array_length * pow(2, i)) {
                left_slope_key -= array_length * pow(2, i);
            }
            while (right_slope_key < array_length * pow(2, i)) {
                right_slope_key += array_length * pow(2, i);
            }
            while (right_slope_key > array_length * pow(2, i)) {
                right_slope_key -= array_length * pow(2, i);
            }
        }
        float left_slope = slope_array[left_slope_key];
        float right_slope = slope_array[right_slope_key];
        float left_value = get_value(left_slope, o_lsk * (scale / pow(2, i)), 0, x);
        float right_value = get_value(right_slope, o_rsk * (scale / pow(2, i)), 0, x);

        returnvalue += interpolate(left_value, right_value, x, scale / pow(2, i));// * pow(persistance, -i);
    }
    return returnvalue; /// maxvalue;
}

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

void triangle::fill(FIBITMAP* img, RGBQUAD color) {
    std::vector<point*> y_vec = {p1, p2, p3};
    std::sort(y_vec.begin(), y_vec.end(), point_sort_y);
    std::vector<point*> x_vec;
    bool first = true;
    bool second = true;
    float slope1;
    float slope2;
    int value1;
    int value2;
    for(int y = y_vec[0]->y; y <= y_vec[2]->y; y++) {
        if (y < y_vec[1]->y) {
            if(first) {
                x_vec = {y_vec[1], y_vec[2]};
                std::sort(x_vec.begin(), x_vec.end(), point_sort_x);
                slope1 = (float)(y_vec[0]->y - x_vec[0]->y) / (y_vec[0]->x - x_vec[0]->x);
                slope2 = (float)(y_vec[0]->y - x_vec[1]->y) / (y_vec[0]->x - x_vec[1]->x);
                first = false;
            }
            value1 = round(get_value_y(slope1, y_vec[0]->x, y_vec[0]->y, y));
            value2 = round(get_value_y(slope2, y_vec[0]->x, y_vec[0]->y, y));
            for (int x = value1; x <= value2; x++) {
                FreeImage_SetPixelColor(img, x, y, &color);
            }
        } else {
            if(second) {
                x_vec = {y_vec[0], y_vec[1]};
                std::sort(x_vec.begin(), x_vec.end(), point_sort_x);
                slope1 = (float)(y_vec[2]->y - x_vec[0]->y) / (y_vec[2]->x - x_vec[0]->x);
                slope2 = (float)(y_vec[2]->y - x_vec[1]->y) / (y_vec[2]->x - x_vec[1]->x);
                second = false;
            }
            value1 = round(get_value_y(slope1, y_vec[2]->x, y_vec[2]->y, y));
            value2 = round(get_value_y(slope2, y_vec[2]->x, y_vec[2]->y, y));
            for (int x = value1; x <= value2; x++) {
                FreeImage_SetPixelColor(img, x, y, &color);
            }
        }
    }
}

float line::get_length() {
    return std::sqrt(pow(p1->x - p2->x, 2) + pow(p1->y - p2->y, 2));
}

float line::get_angle() {
    float x = p1->x - p2->x;
    float y = p1->y - p2->y;
    return std::atan2(y, x);
}

void line::rasterize(FIBITMAP* img, RGBQUAD color) {
    float slope = (float)(p1->y - p2->y) / (p1->x - p2->x);
    if(fabs(slope) <= 1) {
        for(int i = p1->x; i != p2->x + signum(p2->x - p1->x); i += signum(p2->x - p1->x)) {
            int value = round(get_value(slope, p1->x, p1->y, i));
            FreeImage_SetPixelColor(img, i, value, &color);
        }
    } else {
        for(int i = p1->y; i != p2->y + signum(p2->y - p1->y); i += signum(p2->y - p1->y)) {
            int value = round(get_value_y(slope, p1->x, p1->y, i));
            FreeImage_SetPixelColor(img, value, i, &color);
        }
    }
}

void fill(FIBITMAP* img, int x, int y, RGBQUAD color) {
    RGBQUAD check_color;
    FreeImage_GetPixelColor(img, x, y, &check_color);
    if(check_color.rgbRed == 0 && check_color.rgbGreen == 0 && check_color.rgbBlue == 0) {
        FreeImage_SetPixelColor(img, x, y, &color);
        fill(img, x + 1, y, color);
        fill(img, x, y + 1, color);
        fill(img, x - 1, y, color);
        fill(img, x, y - 1, color);
    }
}