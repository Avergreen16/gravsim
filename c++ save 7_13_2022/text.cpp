#pragma once
#include <unordered_map>
#include <string>
#include <array>
#include <sstream>

#include "stbdec.cpp"
#include "render.cpp"

std::unordered_map<char, std::array<int, 2>> standard_chars = {
    {'A', {0, 6}},
    {'B', {6, 6}},
    {'C', {12, 6}},
    {'D', {18, 6}},
    {'E', {24, 6}},
    {'F', {30, 6}},
    {'G', {36, 6}},
    {'H', {42, 6}},
    {'I', {48, 6}},
    {'J', {54, 6}},
    {'K', {60, 6}},
    {'L', {66, 6}},
    {'M', {72, 8}},
    {'N', {80, 6}},
    {'O', {86, 6}},
    {'P', {92, 6}},
    {'Q', {98, 6}},
    {'R', {104, 6}},
    {'S', {110, 6}},
    {'T', {116, 6}},
    {'U', {122, 6}},
    {'V', {128, 6}},
    {'W', {134, 8}},
    {'X', {142, 6}},
    {'Y', {148, 6}},
    {'Z', {154, 6}},

    {'a', {160, 6}},
    {'b', {166, 6}},
    {'c', {172, 6}},
    {'d', {178, 6}},
    {'e', {184, 6}},
    {'f', {190, 6}},
    {'g', {196, 6}},
    {'h', {202, 6}},
    {'i', {208, 4}},
    {'j', {212, 6}},
    {'k', {218, 6}},
    {'l', {224, 4}},
    {'m', {228, 8}},
    {'n', {236, 6}},
    {'o', {242, 6}},
    {'p', {248, 6}},
    {'q', {254, 7}},
    {'r', {261, 6}},
    {'s', {267, 6}},
    {'t', {273, 6}},
    {'u', {279, 6}},
    {'v', {285, 6}},
    {'w', {291, 8}},
    {'x', {299, 6}},
    {'y', {305, 6}},
    {'z', {311, 6}},

    {' ', {455, 4}},
    {'_', {317, 6}},
    {'-', {323, 5}},
    {'+', {328, 5}},
    {'=', {333, 5}},
    {'@', {338, 8}},
    {'.', {346, 2}},
    {'!', {348, 2}},
    {'?', {350, 6}}, 
    {'(', {356, 3}},
    {')', {359, 3}},
    {'[', {362, 3}},
    {']', {365, 3}},
    {':', {368, 2}},
    {';', {370, 2}},
    {',', {372, 2}},
    {'\\', {374, 4}},
    {'/', {378, 4}},
    {'<', {382, 4}},
    {'>', {386, 4}},
    {'*', {390, 5}},

    {'0', {395, 6}},
    {'1', {401, 6}},
    {'2', {407, 6}},
    {'3', {413, 6}},
    {'4', {419, 6}},
    {'5', {425, 6}},
    {'6', {431, 6}},
    {'7', {437, 6}},
    {'8', {443, 6}},
    {'9', {449, 6}}
};

/*std::unordered_map<char, std::array<int, 2>> hex_chars = {
    {'0', {395, 6}},
    {'1', {459, 4}},
    {'2', {463, 6}},
    {'3', {469, 7}},
    {'4', {476, 4}},
    {'5', {480, 6}},
    {'6', {486, 7}},
    {'7', {493, 6}},
    {'8', {499, 6}},
    {'9', {505, 7}},
    {'a', {512, 6}},
    {'b', {518, 6}},
    {'c', {524, 7}},
    {'d', {531, 7}},
    {'e', {538, 6}},
    {'f', {544, 6}}
};*/

std::unordered_map<char, std::array<int, 2>> hex_chars = {
    {'0', {395, 6}},
    {'1', {459, 6}},
    {'2', {465, 6}},
    {'3', {471, 6}},
    {'4', {477, 6}},
    {'5', {483, 6}},
    {'6', {489, 6}},
    {'7', {495, 6}},
    {'8', {501, 6}},
    {'9', {507, 6}},
    {'a', {513, 6}},
    {'b', {519, 6}},
    {'c', {525, 6}},
    {'d', {531, 6}},
    {'e', {537, 6}},
    {'f', {543, 6}}
};

int get_string_length(std::string str, int text_scale) {
    int collector = -1;
    for(char c : str) {
        if(standard_chars.contains(c)) {
            collector += standard_chars[c][2] + 1;
        }
    }
    return collector * text_scale;
}

/*void render_text(unsigned int shader, unsigned int source_img, std::array<int, 3> source_img_info, std::array<int, 2> window_size, std::array<float, 2> lower_left, std::string str, int text_scale) {
    std::array<float, 2> current_pos = lower_left;
    for(char c : str) {
        if(standard_chars.contains(c)) {
            printf("check\n");
            std::array<int, 3> char_data = standard_chars[c];
            int text_width = char_data[2] * text_scale;
            draw_tile(shader, source_img, {current_pos[0], current_pos[1], float(text_width), float(source_img_info[1]) * text_scale}, {char_data[0], char_data[1], char_data[2], source_img_info[1], source_img_info[0], source_img_info[1]}, window_size, 0.1f);
            current_pos[0] += text_width + text_scale;
        }
    }
}*/

struct text_struct {
    std::string str;
    int len;
    bool length_generated = false;

    void set_str(std::string str) {
        this->str = str;
        length_generated = false;
    }

    int get_len() {
        bool hex_mode = false;
        if(length_generated == false) {
            length_generated = true;
            len = -1;
            for(int i = 0; i < str.size(); i++) {
                char c = str[i];
                if(i < str.size() - 2 && c == '\\') {
                    switch(str[i + 1]) {
                        case 'x':
                            hex_mode = true;
                            i++;
                            continue;

                        case 'd':
                            hex_mode = false;
                            i++;
                            continue;
                    }
                }
                if(hex_mode && hex_chars.contains(c)) {
                    len += hex_chars[c][1] + 1;
                } else if(standard_chars.contains(c)) {
                    hex_mode = false;
                    len += standard_chars[c][1] + 1;
                }
            }
        }
        return len;
    }
};

void render_text(unsigned int shader, unsigned int source_img, std::array<int, 3> source_img_info, std::array<int, 2> window_size, std::array<float, 2> lower_left, std::string str, double text_scale, double wrap_around) {
    bool hex_mode = false;
    std::array<float, 2> current_pos = lower_left;
    for(int i = 0; i < str.size(); i++) {
        char c = str[i];
        if(i < str.size() - 2 && c == '\\') {
            switch(str[i + 1]) {
                case 'x':
                    hex_mode = true;
                    i++;
                    continue;
                
                case 'n':
                    current_pos[0] = lower_left[0];
                    current_pos[1] -= 13 * text_scale;
                    i++;
                    continue;

                case 'd':
                    hex_mode = false;
                    i++;
                    continue;
            }
        }
        if(hex_mode && hex_chars.contains(c)) {
            std::array<int, 2> char_data = hex_chars[c];
            double text_width = char_data[1] * text_scale;
            draw_tile(shader, source_img, {current_pos[0], current_pos[1], float(text_width), float(12 * text_scale)}, {char_data[0], 0, char_data[1], 1, source_img_info[0], 1}, window_size, 0.0f);
            current_pos[0] += text_width + text_scale;
            if(current_pos[0] - lower_left[0] > wrap_around) {
                current_pos[0] = lower_left[0];
                current_pos[1] -= 13 * text_scale;
            }
        } else if(standard_chars.contains(c)) {
            hex_mode = false;
            std::array<int, 2> char_data = standard_chars[c];
            double text_width = char_data[1] * text_scale;
            draw_tile(shader, source_img, {current_pos[0], current_pos[1], float(text_width), float(12 * text_scale)}, {char_data[0], 0, char_data[1], 1, source_img_info[0], 1}, window_size, 0.0f);
            current_pos[0] += text_width + text_scale;
            if(current_pos[0] - lower_left[0] > wrap_around) {
                current_pos[0] = lower_left[0];
                current_pos[1] -= 13 * text_scale;
            }
        }
    }
}

std::string to_hex_string(int i) {
    std::stringstream s;
    s << "\\x" << std::hex << i;
    return s.str();
}

int get_len(std::string str) {
    bool hex_mode = false;
    int len = -1;
    for(int i = 0; i < str.size(); i++) {
        char c = str[i];
        if(i < str.size() - 2 && c == '\\') {
            switch(str[i + 1]) {
                case 'x':
                    hex_mode = true;
                    i++;
                    continue;

                case 'd':
                    hex_mode = false;
                    i++;
                    continue;
            }
        }
        if(hex_mode && hex_chars.contains(c)) {
            len += hex_chars[c][1] + 1;
        } else if(standard_chars.contains(c)) {
            hex_mode = false;
            len += standard_chars[c][1] + 1;
        }
    }
    return len;
}

