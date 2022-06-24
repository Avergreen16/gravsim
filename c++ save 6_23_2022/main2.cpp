#define GLFW_INCLUDE_NONE
#define _USE_MATH_DEFINES
#include <glfw\glfw3.h>
#include <glad\glad.h>
#include <iostream>
#include <cmath>
#include <array>
#include <map>
#include <ctime>
#include <string>
#include <unordered_map>

#include "worldgen.hpp"

//shaders

const char* vertex_shader = R"""(
#version 460 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex_coord;

out vec2 tex_coord_f;

layout(location = 0) uniform float z_value;
layout(location = 1) uniform vec2 window_size;
layout(location = 3) uniform vec4 display_rect;
layout(location = 7) uniform vec4 texture_rect;

void main() {
    gl_Position = vec4((pos * display_rect.zw + display_rect.xy) / window_size * 2, z_value, 1.0);
    tex_coord_f = tex_coord * texture_rect.zw + texture_rect.xy;
}
)""";

const char* fragment_shader = R"""(
#version 460 core

out vec4 frag_color;

in vec2 tex_coord_f;

layout(location = 10) uniform sampler2D input_texture;

void main() {
    frag_color = texture(input_texture, tex_coord_f);
    if(frag_color.w == 0.0) discard;
}
)""";

const char* vertex_shader_chunk = R"""(
#version 460 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex_coord;

out vec2 tex_coord_f;

layout(location = 0) uniform vec2 window_size;
layout(location = 2) uniform vec2 top_left;
layout(location = 4) uniform vec2 visual_size;

layout(location = 6) uniform vec2 texture_numbers;
layout(location = 8) uniform int textures[256];

void main() {
    int current_texture = textures[gl_InstanceID];
    gl_Position = vec4(((pos + vec2(gl_InstanceID % 16, gl_InstanceID / 16)) * visual_size + top_left) / window_size * 2, 0.95, 1.0);
    tex_coord_f = (tex_coord + vec2(current_texture % int(texture_numbers.x), current_texture / int(texture_numbers.x))) / texture_numbers;
}
)""";

const char* vertex_shader_chunk_depth = R"""(
#version 460 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex_coord;

out vec2 tex_coord_f;

layout(location = 0) uniform vec2 window_size;
layout(location = 2) uniform vec2 top_left;
layout(location = 4) uniform vec2 visual_size;

layout(location = 6) uniform vec2 texture_numbers;
layout(location = 8) uniform float y_coordinate_chunk;
layout(location = 9) uniform int textures[256];
layout(location = 265) uniform float offsets[256];

void main() {
    int current_texture = textures[gl_InstanceID];
    gl_Position = vec4(((pos + vec2(gl_InstanceID % 16, gl_InstanceID / 16)) * visual_size + top_left) / window_size * 2, (int(gl_InstanceID * 0.0625) + offsets[gl_InstanceID] * 0.0625) * 0.01125 + y_coordinate_chunk * 0.18, 1.0);
    tex_coord_f = (tex_coord + vec2(current_texture % int(texture_numbers.x), current_texture / int(texture_numbers.x))) / texture_numbers;
}
)""";

const char* fragment_shader_chunk = R"""(
#version 460 core

out vec4 frag_color;

in vec2 tex_coord_f;

uniform sampler2D input_texture;

void main() {
    frag_color = texture(input_texture, tex_coord_f);
    if(frag_color.w == 0.0) discard;
}
)""";

//operators

template<typename type>
std::array<type, 2> operator+(std::array<type, 2> a, std::array<type, 2> b) {
    return {a[0] + b[0], a[1] + b[1]};
}

template<typename type>
std::array<type, 2> operator-(std::array<type, 2> a, std::array<type, 2> b) {
    return {a[0] - b[0], a[1] - b[1]};
}

//

float vertices[] = {
    0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

unsigned int indices[] {
    0, 1, 2,
    0, 2, 3
};

std::array<double, 2> camera_pos;
float scale = 24;
double sqrt1_5 = std::sqrt(1.5);

std::map<int, bool> key_down_array = {
    {GLFW_KEY_W, false},
    {GLFW_KEY_S, false},
    {GLFW_KEY_A, false},
    {GLFW_KEY_D, false},
    {GLFW_KEY_SPACE, false}
};

int last_direction_key_pressed;

struct Tile {
    int floor_ID;
    int floor_tex;
    int object_ID;
    int object_tex;
};

struct Chunk_data {
    std::array<Tile, 256> tiles;
    std::array<int, 2> corner;
};

std::array<unsigned int, 25> active_chunks;
std::unordered_map<unsigned int, Chunk_data*> loaded_chunks;

int get_texture_variation(int target, std::array<int, 4> neighbors) {
    if(neighbors[0] == target) {
        if(neighbors[1] == target) {
            if(neighbors[2] == target) {
                if(neighbors[3] == target) return 14;
                return 12;
            } else if(neighbors[3] == target) return 13;
            return 8;
        } else if(neighbors[2] == target) {
            if(neighbors[3] == target) return 11;
            return 7;
        } else if(neighbors[3] == target) return 6;
        return 1;
    } else if(neighbors[1] == target) {
        if(neighbors[2] == target) {
            if(neighbors[3] == target) return 10;
            return 5;
        } else if(neighbors[3] == target) return 4;
        return 0;
    } else if(neighbors[2] == target) {
        if(neighbors[3] == target) return 9;
        return 3;
    } else if(neighbors[3] == target) return 2;
    return -1;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS) {
        if(key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
        else if(key == GLFW_KEY_EQUAL) scale *= 1.2;
        else if(key == GLFW_KEY_MINUS) scale /= 1.2;
        else if(key_down_array.contains(key)) {
            key_down_array[key] = true;
            if(key == GLFW_KEY_W || key == GLFW_KEY_D || key == GLFW_KEY_S || key == GLFW_KEY_A) last_direction_key_pressed = key;
        }
        else if(key == GLFW_KEY_R) loaded_chunks.clear();
        /*else if(key == GLFW_KEY_G) {
            for(int j = 0; j < active_chunks.size(); j++) {
                std::array<Tile, 256>* ptr = &active_chunks[j].tiles;
                for(int i = 0; i < 256; i++) {
                    if(ptr->at(i).floor_ID == 1) {
                        std::array<int, 4> neighbors;
                        if(i % 16 == 15) {
                            if(i < 16) {
                                neighbors = {ptr->at(i + 16).floor_ID, 0, ptr->at(i - 1).floor_ID, 0};
                            } else if(i > 239) {
                                neighbors = {0, ptr->at(i - 16).floor_ID, ptr->at(i - 1).floor_ID, 0};
                            } else {
                                neighbors = {ptr->at(i + 16).floor_ID, ptr->at(i - 16).floor_ID, ptr->at(i - 1).floor_ID, 0};
                            }
                        } else if(i % 16 == 0) {
                            if(i < 16) {
                                neighbors = {ptr->at(i + 16).floor_ID, 0, 0, ptr->at(i + 1).floor_ID};
                            } else if(i > 239) {
                                neighbors = {0, ptr->at(i - 16).floor_ID, 0, ptr->at(i + 1).floor_ID};
                            } else {
                                neighbors = {ptr->at(i + 16).floor_ID, ptr->at(i - 16).floor_ID, 0, ptr->at(i + 1).floor_ID};
                            }
                        } else if(i < 16) {
                            neighbors = {ptr->at(i + 16).floor_ID, 0, ptr->at(i - 1).floor_ID, ptr->at(i + 1).floor_ID};
                        } else if(i > 239) {
                            neighbors = {0, ptr->at(i - 16).floor_ID, ptr->at(i - 1).floor_ID, ptr->at(i + 1).floor_ID};
                        } else {
                            neighbors = {ptr->at(i + 16).floor_ID, ptr->at(i - 16).floor_ID, ptr->at(i - 1).floor_ID, ptr->at(i + 1).floor_ID};
                        }

                        int variation = get_texture_variation(2, neighbors);
                        if(variation != -1) {
                            printf("%i ", variation);
                            (*ptr)[i].floor_ID = variation + 80;
                        }
                    }
                }
            }
        }*/
    } else if(action == GLFW_RELEASE) {
       if(key_down_array.find(key) != key_down_array.end()) key_down_array[key] = false;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if(yoffset > 0) scale *= sqrt1_5;
    else scale /= sqrt1_5;
}

void draw_tile(unsigned int shader, unsigned int texture, std::array<float, 4> display_rect, std::array<int, 6> texture_rect, std::array<int, 2> window_size, float z_value) {
    glUseProgram(shader);

    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1f(0, z_value);
    glUniform2f(1, (double)window_size[0] / 2, (double)window_size[1] / 2);
    glUniform4f(3, display_rect[0], display_rect[1], display_rect[2], display_rect[3]);
    glUniform4f(7, (float)texture_rect[0] / texture_rect[4], (float)texture_rect[1] / texture_rect[5], (float)texture_rect[2] / texture_rect[4], (float)texture_rect[3] / texture_rect[5]);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

unsigned int generate_texture(char address[], std::array<int, 3> info) {
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char *data = stbi_load(address, &info[0], &info[1], &info[2], 0); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, info[0], info[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    return texture_id;
}

//world gen

std::array<int, 2> chunk_size = {16, 16};
std::array<int, 2> world_size_chunks = {2450, 1225};
std::array<int, 2> world_size = {world_size_chunks[0] * 16, world_size_chunks[1] * 16};
unsigned int current_chunk = NAN;

std::array<int, 25> offsets = {
    -2 + 2 * world_size_chunks[0], -1 + 2 * world_size_chunks[0], 2 * world_size_chunks[0], 1 + 2 * world_size_chunks[0], 2 + 2 * world_size_chunks[0],
    -2 + world_size_chunks[0], -1 + world_size_chunks[0], world_size_chunks[0], 1 + world_size_chunks[0], 2 + world_size_chunks[0],
    -2, -1, 0, 1, 2,
    -2 - world_size_chunks[0], -1 - world_size_chunks[0], -world_size_chunks[0], 1 - world_size_chunks[0], 2 - world_size_chunks[0],
    -2 - 2 * world_size_chunks[0], -1 - 2 * world_size_chunks[0], -2 * world_size_chunks[0], 1 - 2 * world_size_chunks[0], 2 - 2 * world_size_chunks[0]
};

std::array<Tile, 256> generate_chunk(Worldgen* worldgen, std::array<int, 2> top_left) {
    std::array<Tile, 256> tiles;
    for(int x = 0; x < chunk_size[0]; x++) {
        for(int y = 0; y < chunk_size[1]; y++) {
            //return_chunk[x + y * chunk_size.x] = pbg.get_biome(top_left.x + x, top_left.y + y, 39200, 19600);
            //if(is_overlay) chunk[x + y * chunk_size[0]] = get_floor_tile_overlay(worldgen->biomegen->get_biome(top_left[0] + x, top_left[1] + y, 39200, 19600), worldgen, top_left[0] + x, top_left[1] + y);
            //chunk[x + y * chunk_size[0]] = get_floor_tile(worldgen->biomegen->get_biome(top_left[0] + x, top_left[1] + y, 39200, 19600), worldgen, top_left[0] + x, top_left[1] + y);
            std::array<int, 2> position = {top_left[0] + x, (top_left[1] + y)};
            uint32_t biome = worldgen->retrieve(position, world_size);

            std::array<int, 2> floor = get_floor_tile(biome, worldgen);
            std::array<int, 2> object = get_object_tile(biome, worldgen);
            tiles[x + y * chunk_size[0]] = {floor[0], floor[1], object[0], object[1]};
        }
    }
    return tiles;
}

bool check_point_inclusion(std::array<double, 4> intrect, std::array<double, 2> point) {
    if(point[0] >= intrect[0] && point[0] < intrect[0] + intrect[2] && point[1] >= intrect[1] && point[1] < intrect[1] + intrect[3]) return true;
    return false;
}

bool check_point_inclusion(std::array<int, 4> intrect, std::array<double, 2> point) {
    if(point[0] >= intrect[0] && point[0] < intrect[0] + intrect[2] && point[1] >= intrect[1] && point[1] < intrect[1] + intrect[3]) return true;
    return false;
}

bool check_if_moved_chunk(std::array<double, 2> player_pos) {
    unsigned int pos = (unsigned int)(player_pos[0] / 16) + (unsigned int)(player_pos[1] / 16) * world_size_chunks[0];
    if(pos != current_chunk) {
        current_chunk = pos;
        return true;
    }
    return false;
}

void generate_chunk(unsigned int chunk_key, Worldgen* worldgen) {
    if(!loaded_chunks.contains(chunk_key)) {
        std::array<int, 2> corner = {int(chunk_key % 2450U * 16U), int(chunk_key / 2450U) * 16};
        printf("%i %i\n", corner[0], corner[1]);
        loaded_chunks.insert({chunk_key, new Chunk_data{generate_chunk(worldgen, corner), corner}});
    }
}

//entities

struct Double_counter {
    double value;
    double limit;

    Double_counter(double p_limit) : limit(p_limit) {};

    bool increment(double change) {
        value += change;
        if(value >= limit) {
            value = 0;
            return true;
        }
        return false;
    }

    void reset() {
        value = 0;
    }
};

struct Int_counter {
    int value;
    int limit;

    Int_counter(int p_limit) : limit(p_limit) {}

    bool increment() {
        value++;
        if(value >= limit) {
            value = 0;
            return true;
        }
        return false;
    }

    void reset() {
        value = 0;
    }
};

enum state{IDLE, WALKING, RUNNING};
enum direction{NORTH, EAST, SOUTH, WEST, NORTHEAST, SOUTHEAST, SOUTHWEST, NORTHWEST};

struct Entity {
    unsigned int spritesheet;

    std::array<float, 2> visual_size = {2, 2};
    std::array<float, 2> visual_offset = {-1, -0.125};
    std::array<double, 2> position = {13155 + 0.5, 8104 + 0.5};
    std::array<int, 2> sprite_size = {32, 32};
    std::array<int, 2> active_sprite = {0, 1};

    Double_counter cycle_timer = Double_counter(150);
    Int_counter sprite_counter = Int_counter(4);

    int state = IDLE;
    int direction_moving = SOUTH;
    int direction_facing = SOUTH;

    Entity(unsigned int p_image) {
        spritesheet = p_image;
    }
    
    void render(std::array<double, 2> camera_pos, float scale, unsigned int shader, std::array<int, 2> window_size) {
        draw_tile(shader, spritesheet, {float((position[0] - camera_pos[0] + visual_offset[0]) * scale), float((position[1] - camera_pos[1] + visual_offset[1]) * scale), visual_size[0] * scale, visual_size[1] * scale}, {active_sprite[0], active_sprite[1], 1, 1, 4, 4}, window_size, 0.36 + fmod(position[1] - 0.125, 16) * 0.01125);
    }

    void tick(double factor, double delta) {
        if(state != IDLE) {
            double change_x = 0.0;
            double change_y = 0.0;

            active_sprite[1] = direction_facing;
            
            switch(direction_moving) {
                case NORTH:
                    change_y = 0.005 * delta * factor;
                    break;
                case NORTHEAST:
                    change_x = 0.003536 * delta * factor;
                    change_y = 0.003536 * delta * factor;
                    break;
                case EAST:
                    change_x = 0.005 * delta * factor;
                    break;
                case SOUTHEAST:
                    change_x = 0.003536 * delta * factor;
                    change_y = -0.003536 * delta * factor;
                    break;
                case SOUTH:
                    change_y = -0.005 * delta * factor;
                    break;
                case SOUTHWEST:
                    change_x = -0.003536 * delta * factor;
                    change_y = -0.003536 * delta * factor;
                    break;
                case WEST:
                    change_x = -0.005 * delta * factor;
                    break;
                case NORTHWEST:
                    change_x = -0.003536 * delta * factor;
                    change_y = 0.003536 * delta * factor;
                    break;
            }

            if(state == WALKING) {
                if(cycle_timer.increment(delta)) {
                    sprite_counter.increment();
                }
                position[0] += change_x;
                position[1] += change_y;
            } else if(state == RUNNING) {
                if(cycle_timer.increment(delta * 1.5)) {
                    sprite_counter.increment();
                }
                position[0] += change_x * 25;
                position[1] += change_y * 25;
            }

            active_sprite[0] = sprite_counter.value;
        } else {
            sprite_counter.reset();
            cycle_timer.reset();
            active_sprite[0] = 3;
        }
    }
};

void set_player_state(Entity* player) {
    if(key_down_array[GLFW_KEY_SPACE]) {
        player->state = RUNNING;
    } else {
        player->state = WALKING;
    }
}

void set_player_enums(Entity* player) {
    bool check_last_key_pressed = true;
    if(key_down_array[GLFW_KEY_W]) {
        if(key_down_array[GLFW_KEY_D]) {
            player->direction_moving = NORTHEAST;
            player->direction_facing = EAST;
            set_player_state(player);
        } else if(key_down_array[GLFW_KEY_S]) {
            player->state = IDLE;
        } else if(key_down_array[GLFW_KEY_A]) {
            player->direction_moving = NORTHWEST;
        player->direction_facing = WEST;
            set_player_state(player);
        } else {
            player->direction_moving = NORTH;
            player->direction_facing = NORTH;
            set_player_state(player);
        }
    } else if(key_down_array[GLFW_KEY_D]) {
        if(key_down_array[GLFW_KEY_S]) {
            player->direction_moving = SOUTHEAST;
            player->direction_facing = EAST;
            set_player_state(player);
        } else if(key_down_array[GLFW_KEY_A]) {
            player->state = IDLE;
        } else {
            player->direction_moving = EAST;
            player->direction_facing = EAST;
            set_player_state(player);
        }
    } else if(key_down_array[GLFW_KEY_S]) {
        if(key_down_array[GLFW_KEY_A]) {
            player->direction_moving = SOUTHWEST;
        player->direction_facing = WEST;
            set_player_state(player);
        } else {
            player->direction_moving = SOUTH;
            player->direction_facing = SOUTH;
            set_player_state(player);
        }
    } else if(key_down_array[GLFW_KEY_A]) {
        player->direction_moving = WEST;
        player->direction_facing = WEST;
        set_player_state(player);
    } else {
        player->state = IDLE;
    }
}

//

unsigned int create_shader(const char* vertex_shader, const char* fragment_shader) {
    unsigned int v_shader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int f_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(v_shader, 1, &vertex_shader, NULL);
    glShaderSource(f_shader, 1, &fragment_shader, NULL);
    glCompileShader(v_shader);
    glCompileShader(f_shader);

    unsigned int shader = glCreateProgram();
    glAttachShader(shader, v_shader);
    glAttachShader(shader, f_shader);
    glLinkProgram(shader);

    glDeleteShader(v_shader);
    glDeleteShader(f_shader);

    return shader;
}

enum tile_ID{VOID, SAND, GRASS, WATER, TALL_GRASS};

std::unordered_map<int, float> offset_values = {
    {VOID, 0.0f},
    {TALL_GRASS, 0.0625f}
};

int main() {
    clock_t time_storage = clock();
    clock_t time_storage_frames = clock();
    if(!glfwInit()) {
        printf("Catastrophic Failure! (init)");
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(1000, 600, "test", NULL, NULL);
    if(!window) {
        printf("Catastrophic Failure! (window)");
        return 1;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);

    gladLoadGL();

    glfwSwapInterval(0);

    unsigned int shader_program = create_shader(vertex_shader, fragment_shader);
    unsigned int chunk_shader_program = create_shader(vertex_shader_chunk, fragment_shader_chunk);
    unsigned int chunk_shader_program_depth = create_shader(vertex_shader_chunk_depth, fragment_shader_chunk);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1); 

    glEnable(GL_DEPTH_TEST);
    //glDepthMask(GL_FALSE);
    glDepthFunc(GL_LESS);

    std::array<int, 3> floor_tileset_info, player_spritesheet_info;
    unsigned int floor_tileset, player_spritesheet; 
    floor_tileset = generate_texture("res\\tiles\\floor_tileset.png", floor_tileset_info);
    player_spritesheet = generate_texture("res\\entities\\Player\\player_spritesheet.png", player_spritesheet_info);

    int width, height, frames = 0;

    Worldgen worldgen(0.7707326, 6, 3, 7, 1.5, 4, 4, 3, 1.5, 4, 4, 4, 1.3);

    Entity player(player_spritesheet);

    while(!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glfwPollEvents();

        //keys

        set_player_enums(&player);

        //math
        
        double delta_time = clock() - time_storage;
        time_storage = clock();
            
        player.tick(1.5, delta_time);

        if(check_if_moved_chunk(player.position)) {
            for(int x = -2; x <= 2; x++) {
                for(int y = -2; y <= 2; y++) {
                    generate_chunk(current_chunk + x + world_size_chunks[0] * y, &worldgen);
                }
            }
            for(int i = 0; i < 25; i++) {
                active_chunks[i] = current_chunk + offsets[i];
            }
        }

        camera_pos = {player.position[0], player.position[1] + 0.875};

        //rendering

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        player.render(camera_pos, scale, shader_program, {width, height});

        glUseProgram(chunk_shader_program_depth);
        for(int i = 0; i < 25; i++) {
            Chunk_data* chunk = loaded_chunks[active_chunks[i]];
            glBindTexture(GL_TEXTURE_2D, floor_tileset);

            glUniform2f(0, (double)width / 2, (double)height / 2);
            glUniform2f(2, (chunk->corner[0] - camera_pos[0]) * scale, (chunk->corner[1] - camera_pos[1]) * scale);
            glUniform2f(4, scale, scale);
            glUniform2f(6, 8.0, 8.0);
            glUniform1f(8, 4 - int((i) * 0.2));

            for(int i = 0; i < 256; i++) {
                glUniform1i(9 + i, chunk->tiles[i].object_tex);
            }
            for(int i = 0; i < 256; i++) {
                glUniform1f(265 + i, offset_values[chunk->tiles[i].object_ID]);
            }

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 256);
        }

        glUseProgram(chunk_shader_program);
        for(unsigned int key : active_chunks) {
            Chunk_data* chunk = loaded_chunks[key];
            glBindTexture(GL_TEXTURE_2D, floor_tileset);

            glUniform2f(0, (double)width / 2, (double)height / 2);
            glUniform2f(2, (chunk->corner[0] - camera_pos[0]) * scale, (chunk->corner[1] - camera_pos[1]) * scale);
            glUniform2f(4, scale, scale);
            glUniform2f(6, 8.0, 8.0);
            for(int i = 0; i < 256; i++) {
                glUniform1i(8 + i, chunk->tiles[i].floor_tex);
            }

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 256);
        }

        //

        glfwSwapBuffers(window);
        frames++;
        
        time_t time_frames = clock();
        if(frames % 100 == 0) {
            printf("%f\n", (double)frames / (time_frames - time_storage_frames) * 1000);
            time_storage_frames = time_frames;
            frames = 0;
        }
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    glfwDestroyWindow(window);
    glfwTerminate();

    for(auto& [chunk_key, chunk_ptr] : loaded_chunks) {
        delete chunk_ptr;
        chunk_ptr = nullptr;
    }
    return 0;
}
