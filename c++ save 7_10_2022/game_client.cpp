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
#include <algorithm>
#include <thread>
#include <winsock2.h>

#include "worldgen.cpp"
#include "pathfinding.cpp"

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

in vec2 tex_coord_f;

layout(location = 11) uniform sampler2D input_texture;

void main() {
    gl_FragColor = texture(input_texture, tex_coord_f);
    if(gl_FragColor.w == 0.0) discard;
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

in vec2 tex_coord_f;

uniform sampler2D input_texture;

void main() {
    gl_FragColor = texture(input_texture, tex_coord_f);
    if(gl_FragColor.w == 0.0) discard;
}
)""";

struct player_packet {
    std::array<double, 2> position;
    std::array<int, 2> active_texture;
    bool quit_game;
};

SOCKET set_up_socket() {
    WSADATA wsadata;
    int wsaerr = WSAStartup(0x0202, &wsadata);
    if(wsaerr != 0) {
        std::cout << "WSAStartup error!\n";
        std::cout << wsadata.szSystemStatus << "\n";
        return 1;
    }

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(client_socket == INVALID_SOCKET) {
        std::cout << "socket error!\n";
        std::cout << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    unsigned short int server_port = 16003;
    std::string server_ip = "127.0.0.1";

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(server_ip.c_str());
    address.sin_port = htons(server_port);

    if(connect(client_socket, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cout << "connect error!\n";
        std::cout << WSAGetLastError() << "\n";
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    std::cout << "connection established\n";

    return client_socket;
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

std::array<double, 2> camera_pos;
float scale = 48;
double sqrt1_5 = std::sqrt(1.5);

std::map<int, bool> key_down_array = {
    {GLFW_KEY_W, false},
    {GLFW_KEY_S, false},
    {GLFW_KEY_A, false},
    {GLFW_KEY_D, false},
    {GLFW_KEY_SPACE, false}
};

int last_direction_key_pressed;

struct Chunk_data {
    std::array<tile, 256> floor_tiles;
    std::array<tile, 256> object_tiles;
    std::array<int, 2> corner;
};

std::array<unsigned int, 25> active_chunks;
std::unordered_map<unsigned int, Chunk_data*> loaded_chunks;

int get_texture_variation(tile_ID target, std::array<tile_ID, 4> neighbors) {
    if(neighbors[0] == target) {
        if(neighbors[1] == target) {
            if(neighbors[2] == target) {
                if(neighbors[3] == target) return 15;
                return 13;
            } else if(neighbors[3] == target) return 14;
            return 9;
        } else if(neighbors[2] == target) {
            if(neighbors[3] == target) return 12;
            return 8;
        } else if(neighbors[3] == target) return 7;
        return 2;
    } else if(neighbors[1] == target) {
        if(neighbors[2] == target) {
            if(neighbors[3] == target) return 11;
            return 6;
        } else if(neighbors[3] == target) return 5;
        return 1;
    } else if(neighbors[2] == target) {
        if(neighbors[3] == target) return 10;
        return 4;
    } else if(neighbors[3] == target) return 3;
    return 0;
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
    glUniform2f(1, (double)window_size[0], (double)window_size[1]);
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
unsigned int current_chunk = 0xffffffffu;

tile_ID get_tile(unsigned int chunk_key, int x, int y) {
    if(loaded_chunks.contains(chunk_key)) {
        return loaded_chunks[chunk_key]->floor_tiles[x + y * 16].id;
    }
    return VOIDTILE;
}

void update_tile_water(unsigned int chunk_key, int x, int y) {
    int i = x + y * 16;
    if(loaded_chunks[chunk_key]->floor_tiles[i].id == WATER) {
        std::array<tile_ID, 4> neighbors;
        if(i % 16 == 15) {
            if(i < 16) {
                neighbors = {loaded_chunks[chunk_key]->floor_tiles[i + 16].id, get_tile(chunk_key - world_size_chunks[0], i, 15), loaded_chunks[chunk_key]->floor_tiles[i - 1].id, get_tile(chunk_key + 1, 0, int(i / 16))};
            } else if(i > 239) {
                neighbors = {get_tile(chunk_key + world_size_chunks[0], i % 16, 0), loaded_chunks[chunk_key]->floor_tiles[i - 16].id, loaded_chunks[chunk_key]->floor_tiles[i - 1].id, get_tile(chunk_key + 1, 0, int(i / 16))};
            } else {
                neighbors = {loaded_chunks[chunk_key]->floor_tiles[i + 16].id, loaded_chunks[chunk_key]->floor_tiles[i - 16].id, loaded_chunks[chunk_key]->floor_tiles[i - 1].id, get_tile(chunk_key + 1, 0, int(i / 16))};
            }
        } else if(i % 16 == 0) {
            if(i < 16) {
                neighbors = {loaded_chunks[chunk_key]->floor_tiles[i + 16].id, get_tile(chunk_key - world_size_chunks[0], i, 15), get_tile(chunk_key - 1, 15, int(i / 16)), loaded_chunks[chunk_key]->floor_tiles[i + 1].id};
            } else if(i > 239) {
                neighbors = {get_tile(chunk_key + world_size_chunks[0], i % 16, 0), loaded_chunks[chunk_key]->floor_tiles[i - 16].id, get_tile(chunk_key - 1, 15, int(i / 16)), loaded_chunks[chunk_key]->floor_tiles[i + 1].id};
            } else {
                neighbors = {loaded_chunks[chunk_key]->floor_tiles[i + 16].id, loaded_chunks[chunk_key]->floor_tiles[i - 16].id, get_tile(chunk_key - 1, 15, int(i / 16)), loaded_chunks[chunk_key]->floor_tiles[i + 1].id};
            }
        } else if(i < 16) {
            neighbors = {loaded_chunks[chunk_key]->floor_tiles[i + 16].id, get_tile(chunk_key - world_size_chunks[0], i, 15), loaded_chunks[chunk_key]->floor_tiles[i - 1].id, loaded_chunks[chunk_key]->floor_tiles[i + 1].id};
        } else if(i > 239) {
            neighbors = {get_tile(chunk_key + world_size_chunks[0], i % 16, 0), loaded_chunks[chunk_key]->floor_tiles[i - 16].id, loaded_chunks[chunk_key]->floor_tiles[i - 1].id, loaded_chunks[chunk_key]->floor_tiles[i + 1].id};
        } else {
            neighbors = {loaded_chunks[chunk_key]->floor_tiles[i + 16].id, loaded_chunks[chunk_key]->floor_tiles[i - 16].id, loaded_chunks[chunk_key]->floor_tiles[i - 1].id, loaded_chunks[chunk_key]->floor_tiles[i + 1].id};
        }

        int variation = get_texture_variation(WATER, neighbors);
        switch(variation) {
            case 0:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 56;
                break;
            case 1:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 104;
                break;
            case 2:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 116;
                break;
            case 3:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 112;
                break;
            case 4:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 108;
                break;
            case 5:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 88;
                break;
            case 6:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 92;
                break;
            case 7:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 96;
                break;
            case 8:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 100;
                break;
            case 9:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 84;
                break;
            case 10:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 80;
                break;
            case 11:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 68;
                break;
            case 12:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 64;
                break;
            case 13:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 72;
                break;
            case 14:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 76;
                break;
            case 15:
                loaded_chunks[chunk_key]->floor_tiles[i].texture = 60;
                break;
        }
    }
}

void update_chunk_water(unsigned int chunk_key) {
    for(int x = 0; x < 16; x++) {
        for(int y = 0; y < 16; y++) {
            update_tile_water(chunk_key, x, y);
        }
    }
    unsigned int chunk_key_north = chunk_key + world_size_chunks[0];
    unsigned int chunk_key_east = chunk_key + 1;
    unsigned int chunk_key_south = chunk_key - world_size_chunks[0];
    unsigned int chunk_key_west = chunk_key - 1;
    if(loaded_chunks.contains(chunk_key_north)) {
        for(int x = 0; x < 16; x++) {
            update_tile_water(chunk_key_north, x, 0);
        }
    }
    if(loaded_chunks.contains(chunk_key_east)) {
        for(int y = 0; y < 16; y++) {
            update_tile_water(chunk_key_east, 0, y);
        }
    }
    if(loaded_chunks.contains(chunk_key_south)) {
        for(int x = 0; x < 16; x++) {
            update_tile_water(chunk_key_south, x, 15);
        }
    }
    if(loaded_chunks.contains(chunk_key_west)) {
        for(int y = 0; y < 16; y++) {
            update_tile_water(chunk_key_west, 15, y);
        }
    }
}

std::array<int, 25> offsets = {
    -2 + 2 * world_size_chunks[0], -1 + 2 * world_size_chunks[0], 2 * world_size_chunks[0], 1 + 2 * world_size_chunks[0], 2 + 2 * world_size_chunks[0],
    -2 + world_size_chunks[0], -1 + world_size_chunks[0], world_size_chunks[0], 1 + world_size_chunks[0], 2 + world_size_chunks[0],
    -2, -1, 0, 1, 2,
    -2 - world_size_chunks[0], -1 - world_size_chunks[0], -world_size_chunks[0], 1 - world_size_chunks[0], 2 - world_size_chunks[0],
    -2 - 2 * world_size_chunks[0], -1 - 2 * world_size_chunks[0], -2 * world_size_chunks[0], 1 - 2 * world_size_chunks[0], 2 - 2 * world_size_chunks[0]
};

Chunk_data* generate_chunk(Worldgen* worldgen, std::array<int, 2> southwest) {
    std::array<tile, 256> floor_tiles;
    std::array<tile, 256> object_tiles;
    for(int x = 0; x < chunk_size[0]; x++) {
        for(int y = 0; y < chunk_size[1]; y++) {
            std::array<int, 2> position = {southwest[0] + x, (southwest[1] + y)};
            unsigned int biome = worldgen->retrieve(position, world_size);

            tile floor = get_floor_tile(biome, worldgen);
            tile object = get_object_tile(biome, worldgen);
            floor_tiles[x + y * chunk_size[0]] = floor;
            object_tiles[x + y * chunk_size[0]] = object;
        }
    }

    return new Chunk_data{floor_tiles, object_tiles, southwest};
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

bool generate_chunk(unsigned int chunk_key, Worldgen* worldgen) {
    if(!loaded_chunks.contains(chunk_key)) {
        std::array<int, 2> corner = {int(chunk_key % world_size_chunks[0] * 16), int(chunk_key / world_size_chunks[0]) * 16};
        printf("%i %i\n", corner[0], corner[1]);
        loaded_chunks.insert({chunk_key, generate_chunk(worldgen, corner)});
        return true;
    }
    return false;
}

//entities

struct Double_counter {
    double value = 0;
    double limit;

    void construct(double plimit) {
        limit = plimit;
    };

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
    int value = 0;
    int limit;

    void construct(int plimit) {
        limit = plimit;
    };

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

struct Approach {
    double value = 0.0;
    double target;

    void modify(double rising_change, double falling_change) {
        if(value != target) {
            if(value < target) {
                value += rising_change;
                if(value > target) value = target;
            } else {
                value -= falling_change;
                if(value < target) value = target;
            }
        }
    }

    void modify(double change) {
        if(value != target) {
            if(value < target) {
                value += change;
                if(value > target) value = target;
            } else {
                value -= change;
                if(value < target) value = target;
            }
        }
    }
};

enum states{IDLE, WALKING, RUNNING, SWIMMING};

struct Entity {
    unsigned int spritesheet;

    std::array<float, 2> visual_size = {2, 2};
    std::array<float, 2> visual_offset = {-1, -0.125};
    std::array<double, 2> position;
    std::array<int, 2> sprite_size = {32, 32};
    std::array<int, 2> active_sprite = {0, 3};
    std::array<double, 4> walk_box = {-0.375, -0.125, 0.75, 0.25};

    Double_counter cycle_timer;
    Int_counter sprite_counter;
    Approach speed;

    states state = IDLE;
    directions direction_moving = SOUTH;
    directions direction_facing = SOUTH;
    bool keys_pressed = false;
    states texture_version = WALKING;

    void construct(std::array<double, 2> p_position, unsigned int p_image) {
        position = p_position;
        spritesheet = p_image;
        cycle_timer.construct(150);
        sprite_counter.construct(4);
        speed.target = 0.0;
    }
    
    void render(std::array<double, 2> camera_pos, float scale, unsigned int shader, std::array<int, 2> window_size) {
        draw_tile(shader, spritesheet, {float((position[0] - camera_pos[0] + visual_offset[0]) * scale), float((position[1] - camera_pos[1] + visual_offset[1]) * scale), visual_size[0] * scale, visual_size[1] * scale}, {active_sprite[0], active_sprite[1], 1, 1, 4, 8}, window_size, (position[1] - 0.125 - loaded_chunks[active_chunks[20]]->corner[1]) * 0.01125);
    }

    void tick(double delta) {
        if(state != IDLE) {
            double change_x = 0.0;
            double change_y = 0.0;

            active_sprite[1] = direction_facing;
            
            switch(direction_moving) {
                case NORTH:
                    change_y = 0.006 * delta;
                    break;
                case NORTHEAST:
                    change_x = 0.004243 * delta;
                    change_y = 0.004243 * delta;
                    break;
                case EAST:
                    change_x = 0.006 * delta;
                    break;
                case SOUTHEAST:
                    change_x = 0.004243 * delta;
                    change_y = -0.004243 * delta;
                    break;
                case SOUTH:
                    change_y = -0.006 * delta;
                    break;
                case SOUTHWEST:
                    change_x = -0.004243 * delta;
                    change_y = -0.004243 * delta;
                    break;
                case WEST:
                    change_x = -0.006 * delta;
                    break;
                case NORTHWEST:
                    change_x = -0.004243 * delta;
                    change_y = 0.004243 * delta;
                    break;
            }
            
            switch(state) {
                case WALKING:
                    texture_version = WALKING;
                    sprite_counter.limit = 4;
                    if(keys_pressed) {
                        speed.target = 1.0;
                        speed.modify(0.01 * delta);
                        if(cycle_timer.increment(delta)) {
                            sprite_counter.increment();
                        }
                    } else {
                        speed.target = 0.0;
                        speed.modify(0.02 * delta);
                        if(cycle_timer.increment(delta * 0.7)) {
                            sprite_counter.increment();
                        }
                    }
                    
                    position[0] += change_x * speed.value;
                    position[1] += change_y * speed.value;
                    break;

                case RUNNING:
                    texture_version = WALKING;
                    sprite_counter.limit = 4;
                    if(keys_pressed) {
                        speed.target = 1.6;
                        speed.modify(0.01 * delta);
                        if(cycle_timer.increment(delta * 1.3)) {
                            sprite_counter.increment();
                        }
                    } else {
                        speed.target = 0.0;
                        speed.modify(0.02 * delta);
                        if(cycle_timer.increment(delta * 0.7)) {
                            sprite_counter.increment();
                        }
                    }

                    position[0] += change_x * speed.value;
                    position[1] += change_y * speed.value;
                    break;

                case SWIMMING:
                    texture_version = SWIMMING;
                    sprite_counter.limit = 2;
                    if(sprite_counter.value > 1) sprite_counter.reset();
                    active_sprite[1] = direction_facing + 4;
                    if(keys_pressed) {
                        speed.target = 0.5;
                        speed.modify(0.005 * delta, 0.04 * delta);
                        if(cycle_timer.increment(delta * 0.6)) {
                            sprite_counter.increment();
                        }
                    } else {
                        speed.target = 0.0;
                        speed.modify(0.04 * delta);
                        if(cycle_timer.increment(delta * 0.4)) {
                            sprite_counter.increment();
                        }
                    }

                    position[0] += change_x * speed.value;
                    position[1] += change_y * speed.value;
                    break;
            }

            active_sprite[0] = sprite_counter.value;
        } else {
            sprite_counter.reset();
            cycle_timer.reset();
            if(texture_version == WALKING) active_sprite[0] = 3;
            else if(texture_version == SWIMMING) active_sprite[0] = 1;
        }
    }
};

tile_ID get_tile(int x, int y) {
    int chunk_x = x / 16;
    int chunk_y = y / 16;
    unsigned int chunk_key = chunk_x + chunk_y * world_size_chunks[0];

    if(loaded_chunks.contains(chunk_key)) {
        int pos_x = x % 16;
        int pos_y = y % 16;

        return loaded_chunks[chunk_key]->floor_tiles[pos_x + pos_y * 16].id;
    } else {
        return VOIDTILE;
    }
}

std::vector<tile_ID> get_blocks_around(std::array<double, 4> bounding_box) {
    std::vector<tile_ID> return_vector;

    int lower_x = bounding_box[0];
    int lower_y = bounding_box[1];
    int upper_x = bounding_box[0] + bounding_box[2];
    int upper_y = bounding_box[1] + bounding_box[3];

    for(int y = lower_y; y <= upper_y; y++) {
        for(int x = lower_x; x <= upper_x; x++) {
            return_vector.push_back(get_tile(x, y));
        }
    }

    return return_vector;
}

std::array<std::array<int, 15>, 15> empty_9x9_array = {
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    std::array<int, 15>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

struct Enemy {
    unsigned int spritesheet;

    std::array<float, 2> visual_size = {2, 2};
    std::array<float, 2> visual_offset = {-1, -0.125};
    std::array<double, 2> position = {13150 + 0.5, 8110 + 0.5};
    std::array<int, 2> sprite_size = {32, 32};
    std::array<int, 2> active_sprite = {0, 1};

    Double_counter cycle_timer;
    Int_counter sprite_counter;

    states state = IDLE;
    directions direction_moving = SOUTH;
    directions direction_facing = SOUTH;

    void construct(std::array<double, 2> p_position, unsigned int p_image) {
        position = p_position;
        spritesheet = p_image;
        cycle_timer.construct(150);
        sprite_counter.construct(4);
    }
    
    void render(std::array<double, 2> camera_pos, float scale, unsigned int shader, std::array<int, 2> window_size) {
        draw_tile(shader, spritesheet, {float((position[0] - camera_pos[0] + visual_offset[0]) * scale), float((position[1] - camera_pos[1] + visual_offset[1]) * scale), visual_size[0] * scale, visual_size[1] * scale}, {active_sprite[0], active_sprite[1], 1, 1, 4, 4}, window_size, (position[1] - 0.125 - loaded_chunks[active_chunks[20]]->corner[1]) * 0.01125);
    }

    void tick(double delta) {
        if(state != IDLE) {
            double change_x = 0.0;
            double change_y = 0.0;

            active_sprite[1] = direction_facing;
            
            switch(direction_moving) {
                case NORTH:
                    change_y = 0.006 * delta;
                    break;
                case NORTHEAST:
                    change_x = 0.004243 * delta;
                    change_y = 0.004243 * delta;
                    break;
                case EAST:
                    change_x = 0.006 * delta;
                    break;
                case SOUTHEAST:
                    change_x = 0.004243 * delta;
                    change_y = -0.004243 * delta;
                    break;
                case SOUTH:
                    change_y = -0.006 * delta;
                    break;
                case SOUTHWEST:
                    change_x = -0.004243 * delta;
                    change_y = -0.004243 * delta;
                    break;
                case WEST:
                    change_x = -0.006 * delta;
                    break;
                case NORTHWEST:
                    change_x = -0.004243 * delta;
                    change_y = 0.004243 * delta;
                    break;
            }

            if(state == WALKING) {
                if(cycle_timer.increment(delta)) {
                    sprite_counter.increment();
                }
                position[0] += change_x * 0.8;
                position[1] += change_y * 0.8;
            } else if(state == RUNNING) {
                if(cycle_timer.increment(delta * 1.3)) {
                    sprite_counter.increment();
                }
                position[0] += change_x * 1.28;
                position[1] += change_y * 1.28;
            }

            active_sprite[0] = sprite_counter.value;
        } else {
            sprite_counter.reset();
            cycle_timer.reset();
            active_sprite[0] = 3;
        }
    }

    void pathfind(Entity target) {
        directions dir = Astar_pathfinding({7, 7}, {int(target.position[0] - position[0]) + 7, int(target.position[1] - position[1]) + 7}, empty_9x9_array);
        if(dir != -1) {
            state = WALKING;
            direction_moving = dir;
            switch(dir) {
                case NORTH:
                    direction_facing = NORTH;
                    break;

                case NORTHEAST:
                case SOUTHEAST:
                case EAST:
                    direction_facing = EAST;
                    break;
                
                case SOUTH:
                    direction_facing = SOUTH;
                    break;

                case NORTHWEST:
                case SOUTHWEST:
                case WEST:
                    direction_facing = WEST;
                    break;
            }
        } else {
            state = IDLE;
        }
    }
};

void set_player_state(Entity* player) {
    std::vector<tile_ID> tiles_stepped_on = get_blocks_around({player->walk_box[0] + player->position[0], player->walk_box[1] + player->position[1], player->walk_box[2], player->walk_box[3]});
    if(std::count(tiles_stepped_on.begin(), tiles_stepped_on.end(), WATER) == tiles_stepped_on.size()) {
        player->state = SWIMMING;
    } else if(key_down_array[GLFW_KEY_SPACE]) {
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
            player->keys_pressed = true;
            set_player_state(player);
        } else if(key_down_array[GLFW_KEY_S]) {
            player->keys_pressed = false;
            if(player->speed.value == 0.0) player->state = IDLE;
            else {
                set_player_state(player);
            }
        } else if(key_down_array[GLFW_KEY_A]) {
            player->direction_moving = NORTHWEST;
            player->direction_facing = WEST;
            player->keys_pressed = true;
            set_player_state(player);
        } else {
            player->direction_moving = NORTH;
            player->direction_facing = NORTH;
            player->keys_pressed = true;
            set_player_state(player);
        }
    } else if(key_down_array[GLFW_KEY_D]) {
        if(key_down_array[GLFW_KEY_S]) {
            player->direction_moving = SOUTHEAST;
            player->direction_facing = EAST;
            player->keys_pressed = true;
            set_player_state(player);
        } else if(key_down_array[GLFW_KEY_A]) {
            player->keys_pressed = false;
            if(player->speed.value == 0.0) player->state = IDLE;
            else {
                set_player_state(player);
            }
        } else {
            player->direction_moving = EAST;
            player->direction_facing = EAST;
            player->keys_pressed = true;
            set_player_state(player);
        }
    } else if(key_down_array[GLFW_KEY_S]) {
        if(key_down_array[GLFW_KEY_A]) {
            player->direction_moving = SOUTHWEST;
            player->direction_facing = WEST;
            player->keys_pressed = true;
            set_player_state(player);
        } else {
            player->direction_moving = SOUTH;
            player->direction_facing = SOUTH;
            player->keys_pressed = true;
            set_player_state(player);
        }
    } else if(key_down_array[GLFW_KEY_A]) {
        player->direction_moving = WEST;
        player->direction_facing = WEST;
        player->keys_pressed = true;
        set_player_state(player);
    } else {
        player->keys_pressed = false;
        if(player->speed.value == 0.0) player->state = IDLE;
        else {
            set_player_state(player);
        }
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

std::unordered_map<int, float> offset_values = {
    {VOIDTILE, 0.0f},
    {TALL_GRASS, 0.0625f}
};

struct position_time {
    std::array<double, 2> target_position;
    int time;
};

player_packet send_packet;

player_packet recv_packet;

int sign(double val) {
    if(val > 0) return 1;
    if(val < 0) return -1;
    return 0;
}

struct position_manager {
    std::array<double, 2> old_position = {0, 0};
    std::array<double, 2> interpolated_position = {0, 0};
    std::queue<position_time> position_time_queue;
    int x_displacement = 0;
    int y_displacement = 0;
    int time_container = 0;
    bool initialized = false;
    int passes = 0;

    /*void initialize() {
        if(!initialized) {
            initialized = true;
            old_position = position_time_queue.front().target_position;
            interpolated_position = position_time_queue.front().target_position;
            sign_x = 
            sign_y = 
        }
    }*/

    std::array<double, 2> interpolate(int delta) {
        time_container += delta;
        if(position_time_queue.size() == 0) {
            interpolated_position = recv_packet.position;
        } else {
            passes++;
            if(position_time_queue.front().time != 0) {
                if(!initialized) {
                    interpolated_position = position_time_queue.front().target_position;
                    initialized = true;
                }
                interpolated_position = {old_position[0] - x_displacement * (double(time_container) / position_time_queue.front().time), old_position[1] - y_displacement * (double(time_container) / position_time_queue.front().time)};
            }
            if((sign(interpolated_position[0] - position_time_queue.front().target_position[0]) != sign(x_displacement) && sign(interpolated_position[1] - position_time_queue.front().target_position[1]) != sign(y_displacement)) || (x_displacement == 0 && y_displacement == 0) || position_time_queue.front().time != 0) {
                std::cout << passes << " ";
                passes = 0;
                time_container = 0;
                old_position = position_time_queue.front().target_position;
                position_time_queue.pop();
                x_displacement = old_position[0] - position_time_queue.front().target_position[0];
                y_displacement = old_position[1] - position_time_queue.front().target_position[1];
            }
        }
        return interpolated_position;
    }
};

position_manager pos_manager;

void socket_loop(SOCKET s) {
    time_t time_container = clock();
    bool quit_game = false;
    while(!quit_game) {
        send(s, (char*)&send_packet, sizeof(player_packet), 0);
        recv(s, (char*)&recv_packet, sizeof(player_packet), 0);
        if(pos_manager.position_time_queue.size() < 3) {
            pos_manager.position_time_queue.push({recv_packet.position, int(clock() - time_container)});
            time_container = clock();
        }
        if(send_packet.quit_game) {
            quit_game = true;
        }
    }
    closesocket(s);
}

int main() {
    clock_t time_storage = clock();
    clock_t time_storage_frames = clock();

    SOCKET client_socket = set_up_socket();

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

    std::array<int, 3> floor_tileset_info, player_spritesheet_info, bandit0_spritesheet_info, bandit1_spritesheet_info;
    unsigned int floor_tileset, player_spritesheet, bandit0_spritesheet, bandit1_spritesheet; 
    floor_tileset = generate_texture("res\\tiles\\floor_tileset.png", floor_tileset_info);
    player_spritesheet = generate_texture("res\\entities\\Player\\player_spritesheet.png", player_spritesheet_info);
    bandit0_spritesheet = generate_texture("res\\entities\\Bandit\\bandit0_spritesheet.png", bandit0_spritesheet_info);
    bandit1_spritesheet = generate_texture("res\\entities\\Bandit\\bandit1_spritesheet.png", bandit1_spritesheet_info);

    int width, height, frames = 0;

    Worldgen worldgen;
    worldgen.construct_3(0.7707326, 6, 3, 7, 1.5, 4, 4, 3, 1.5, 4, 4, 4, 1.3);

    Entity player;
    Entity player1;
    Enemy enemy0;
    Enemy enemy1;

    player1.construct({13155 + 0.5, 8110 + 0.5}, player_spritesheet);
    player.construct({13155 + 0.5, 8110 + 0.5}, player_spritesheet);
    enemy0.construct({13160 + 0.5, 8115 + 0.5}, bandit0_spritesheet);
    enemy1.construct({13150 + 0.5, 8119 + 0.5}, bandit1_spritesheet);

    bool c = true;

    Double_counter water_cycle_timer;
    Int_counter water_sprite_counter;
    water_cycle_timer.construct(800);
    water_sprite_counter.construct(4);

    std::thread t0(socket_loop, client_socket);

    while(!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glfwPollEvents();

        //keys

        set_player_enums(&player);

        //math
        
        double delta_time = clock() - time_storage;
        time_storage = clock();
            
        player.tick(delta_time);

        player1.position = pos_manager.interpolate(delta_time);
        player1.active_sprite = recv_packet.active_texture;
        send_packet.position = player.position;
        send_packet.active_texture = player.active_sprite;

        double a = enemy0.position[0] - int(enemy0.position[0]);
        double b = enemy0.position[1] - int(enemy0.position[1]);
        if(a < 0.55 && a > 0.45 && b < 0.55 && b > 0.45) {
            if(c) {
                enemy0.pathfind(player);
                if(enemy0.state != IDLE) c = false;
            }
        } else {
            c = true;
        }
        enemy0.tick(delta_time);
        a = enemy1.position[0] - int(enemy1.position[0]);
        b = enemy1.position[1] - int(enemy1.position[1]);
        if(a < 0.55 && a > 0.45 && b < 0.55 && b > 0.45) {
            if(c) {
                enemy1.pathfind(player);
                if(enemy1.state != IDLE) c = false;
            }
        } else {
            c = true;
        }
        enemy1.tick(delta_time);
        if(water_cycle_timer.increment(delta_time)) water_sprite_counter.increment();

        std::vector<unsigned int> update_chunk_queue;
        if(check_if_moved_chunk(player.position)) {
            for(int x = -2; x <= 2; x++) {
                for(int y = -2; y <= 2; y++) {
                    unsigned int chunk_key = current_chunk + x + world_size_chunks[0] * y;
                    if(generate_chunk(chunk_key, &worldgen)) {
                        update_chunk_queue.push_back(chunk_key);
                    }
                }
            }

            for(unsigned int chunk_key : update_chunk_queue) {
                update_chunk_water(chunk_key);
            }
            update_chunk_queue.clear();

            for(int i = 0; i < 25; i++) {
                active_chunks[i] = current_chunk + offsets[i];
            }
        }

        camera_pos = {player.position[0], player.position[1] + 0.875};

        //rendering

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        player.render(camera_pos, scale, shader_program, {width, height});
        player1.render(camera_pos, scale, shader_program, {width, height});
        enemy0.render(camera_pos, scale, shader_program, {width, height});
        enemy1.render(camera_pos, scale, shader_program, {width, height});

        glUseProgram(chunk_shader_program_depth);
        for(int i = 0; i < 25; i++) {
            Chunk_data* chunk = loaded_chunks[active_chunks[i]];
            glBindTexture(GL_TEXTURE_2D, floor_tileset);

            glUniform2f(0, (double)width, (double)height);
            glUniform2f(2, (chunk->corner[0] - camera_pos[0]) * scale, (chunk->corner[1] - camera_pos[1]) * scale);
            glUniform2f(4, scale, scale);
            glUniform2f(6, 8.0, 16.0);
            glUniform1f(8, 4 - int((i) * 0.2));

            for(int i = 0; i < 256; i++) {
                glUniform1i(9 + i, chunk->object_tiles[i].texture);
            }
            for(int i = 0; i < 256; i++) {
                glUniform1f(265 + i, offset_values[chunk->object_tiles[i].id]);
            }

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 256);
        }

        glUseProgram(chunk_shader_program);
        for(unsigned int key : active_chunks) {
            Chunk_data* chunk = loaded_chunks[key];
            glBindTexture(GL_TEXTURE_2D, floor_tileset);

            glUniform2f(0, (double)width, (double)height);
            glUniform2f(2, (chunk->corner[0] - camera_pos[0]) * scale, (chunk->corner[1] - camera_pos[1]) * scale);
            glUniform2f(4, scale, scale);
            glUniform2f(6, 8.0, 16.0);
            for(int i = 0; i < 256; i++) {
                int floor_tex = chunk->floor_tiles[i].texture;
                if(floor_tex >= 56)floor_tex += water_sprite_counter.value;
                glUniform1i(8 + i, floor_tex);
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
            std::cout << "\n";
        }
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    glfwDestroyWindow(window);
    glfwTerminate();

    send_packet.quit_game = true;
    t0.join();

    WSACleanup();

    for(auto& [chunk_key, chunk_ptr] : loaded_chunks) {
        delete chunk_ptr;
        chunk_ptr = nullptr;
    }
    return 0;
}