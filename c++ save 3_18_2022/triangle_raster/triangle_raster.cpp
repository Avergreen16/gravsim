#define FREEIMAGE_LIB
#include <iostream>
#include <ctime>
#include <cmath>
#include <vector>
#include <FreeImage\Dist\x64\FreeImage.h>

RGBQUAD set_color(float r, float g, float b) {
    RGBQUAD color;
    color.rgbRed = r;
    color.rgbGreen = g;
    color.rgbBlue = b;
    return color;
}

struct point {
    int x;
    int y;
    point () {};
    point(int p_x, int p_y) : x(p_x), y(p_y) {};
};

struct line {
    point* p1;
    point* p2;
    line(point* p_p1, point* p_p2) : p1(p_p1), p2(p_p2) {};
    float get_length();
    float get_angle();
    void rasterize(FIBITMAP* img, RGBQUAD color);
};

float get_value(float slope, int x1, int y1, int x);

void fill(FIBITMAP* img, int starting_x, int starting_y, RGBQUAD color);

int main() {
    time_t starting_time = std::time(NULL);

    char filename[] = "triangle_raster.png";
    RGBQUAD color = set_color(255, 0, 0);
    FreeImage_Initialise();
    FIBITMAP* img = FreeImage_Allocate(200, 200, 24);

    std::vector<point> pv = {point(7, 7), point(190, 110), point(100, 190)};
    std::vector<line> lv;
    for (int i = 0; i < pv.size(); i++) {
        int index;
        if (i == pv.size() - 1) {
            index = 0;
        } else {
            index = i + 1;
        }
        lv.push_back(line(&pv[i], &pv[index]));
    }

    std::cout << lv.size() << std::endl;

    for (line _line : lv) {
        _line.rasterize(img, color);
    }

    fill(img, 100, 100, color);

    if (FreeImage_Save(FIF_PNG, img, filename)) {
        std::cout << "Image saved as " << filename << std::endl;
    }

    FreeImage_DeInitialise();
    time_t ending_time = std::time(NULL);
    std::cout << ending_time - starting_time << std::endl;
    return 0;
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
    std::cout << p1->x << std::endl;
    if(fabs(slope) <= 1) {
        for(int i = p1->x; i != p2->x; i += round(slope / fabs(slope))) {
            int value = round(get_value(slope, p1->x, p1->y, i));
            FreeImage_SetPixelColor(img, i, value, &color);
        }
    } else {
        for(int i = p1->y; i != p2->y; i -= round(slope / fabs(slope))) {
            int value = round(get_value(1.0/slope, p1->y, p1->x, i));
            FreeImage_SetPixelColor(img, value, i, &color);
        }
    }
}

float get_value(float slope, int x1, int y1, int x) {
    return slope * (x - x1) + y1;
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