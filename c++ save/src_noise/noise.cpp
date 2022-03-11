#define FREEIMAGE_LIB
#include <fstream>
#include <iostream>
#include <map>
#include <cmath>
#include <FreeImage\Dist\x64\FreeImage.h>

#include "Noisefactory.hpp"

int main() {
    FreeImage_Initialise();
    FIBITMAP* img = FreeImage_Allocate(256, 256, 24);
    RGBQUAD color;

    Noisefactory nfactory(0.706684, 8, 8, 32, 4, 3, true, true);

    for(int x = 0; x < 256; x++) {
        for(int y = 0; y < 256; y++) {
            float value = std::floor(nfactory.pullvalue(x, y) * 255);
            color.rgbBlue = value;
            color.rgbGreen = value;
            color.rgbRed = value;
            FreeImage_SetPixelColor(img, x, y, &color);
        }
    }

    if (FreeImage_Save(FIF_PNG, img, "noise3.png")) {
        std::cout << "Image saved\n";
    }

    FreeImage_DeInitialise();

    return 0;
}