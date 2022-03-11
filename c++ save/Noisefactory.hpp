#include <map>
#include <vector>

class ValueNoiseFactory {
    private:
        float seed;
        int factor = 313;
        int width;
        int height;
        int scale;
        float rng_number;
        int octaves;
        float persistance;
        bool loop_x;
        bool loop_y;
        float maxvalue;

    public:
        std::map<std::string, float> origin_value_map;

        ValueNoiseFactory(float p_seed, int p_width, int p_height, int p_scale, int p_octaves, int p_persistance, bool p_loop_x, bool p_loop_y);

        float RNG(int decimal_digits);

        float interpolate(float x1, float x2, int coordinate, int scale);

        float pullvalue(int x, int y);
};

class PerlinNoiseFactory {
    private:
        float seed;
        int factor = 313;
        int width;
        int height;
        int scale;
        float rng_number;
        int octaves;
        float persistance;
        bool loop_x;
        bool loop_y;
        float maxvalue;
    
    public:
        std::map<std::string, std::vector<float>> origin_vector_map;

        PerlinNoiseFactory(float p_seed, int p_width, int p_height, int p_scale, int p_octaves, int p_persistance, bool p_loop_x, bool p_loop_y);

        float RNG(int decimal_digits);

        float interpolate(float x1, float x2, int coordinate, int scale);

        float pullvalue(int x, int y);

        float dotproduct(std::vector<float> v1, std::vector<float> v2);
};