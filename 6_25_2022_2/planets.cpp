#define GLFW_INCLUDE_NONE
#define _USE_MATH_DEFINES
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glfw\glfw3.h>
#include <glad\glad.h>
#include <array>
#include <map>
#include <cmath>
#include <ctime>
#include <vector>

//shaders

const char* vertex_shader_ts = R"""(
#version 460 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex_coord;

out vec2 tex_coord_f;
out vec2 internal_pos;

layout(location = 0) uniform float z_value;
layout(location = 1) uniform vec2 window_size;
layout(location = 3) uniform vec4 display_rect;
layout(location = 7) uniform vec4 texture_rect;

void main() {
    gl_Position = vec4((pos * display_rect.zw + display_rect.xy) / window_size * 2, z_value, 1.0);
    tex_coord_f = tex_coord * texture_rect.zw + texture_rect.xy;
    internal_pos = pos * 2 - 1;
}
)""";

const char* fragment_shader_ts = R"""(
#version 460 core

in vec2 tex_coord_f;
in vec2 internal_pos;

layout(location = 11) uniform float axial_tilt;
layout(location = 12) uniform float rotation_angle;
layout(location = 13) uniform float light_angle;

layout(location = 14) uniform sampler2D input_texture;

void main() {
    float pi = 3.14159265358979;
    float angle_in_radians = axial_tilt * pi / 180;
    float py_rot = internal_pos.y * cos(angle_in_radians) + internal_pos.x * sin(angle_in_radians);
    float px_rot = internal_pos.x * cos(angle_in_radians) - internal_pos.y * sin(angle_in_radians);

    if(px_rot * px_rot + py_rot * py_rot >= 1) {
        discard;
    }
    
    float width_at_height = sqrt(1 - py_rot * py_rot);
    float width_at_height_horizontal = sqrt(1 - internal_pos.y * internal_pos.y);
    float py2 = asin(py_rot) * 2/pi;
    float px2 = asin(px_rot / width_at_height) * 2/pi;
    float px3 = asin(internal_pos.x / width_at_height_horizontal) * 2/pi;

    float xCoord = (px2 + 1) / 4 + rotation_angle;
    float tex_x = xCoord - floor(xCoord);
    float tex_y = (py2 + 1) / 2;

    vec4 color = texture(input_texture, vec2(tex_x, tex_y));

    float light_angle2 = light_angle + 0.25;
    if(light_angle2 < 0) {
        light_angle2 += 1;
    }

    float light_xcoord = light_angle2 * 4 - 2;
    float shadow_xcoord0 = light_xcoord - 1;
    float shadow_xcoord1 = light_xcoord + 1;

    if(px3 > shadow_xcoord0 && px3 < shadow_xcoord1) {
        gl_FragColor = vec4(color.xyz / 4, 1.0);
    } else {
        gl_FragColor = vec4(color.xyz, 1.0);
    }
}
)""";

const char* vertex_shader_c_cs = R"""(
#version 460 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex_coord;

out vec2 internal_pos;

layout(location = 0) uniform float z_value;
layout(location = 1) uniform vec2 window_size;
layout(location = 3) uniform vec4 display_rect;

void main() {
    gl_Position = vec4((pos * display_rect.zw + display_rect.xy) / window_size * 2, z_value, 1.0);
    internal_pos = pos * 2 - 1;
}
)""";

const char* fragment_shader_cs = R"""(
#version 460 core

in vec2 internal_pos;

layout(location = 7) uniform vec3 color;
layout(location = 10) uniform float light_angle;

void main() {
    float pi = 3.14159265358979;

    if(internal_pos.x * internal_pos.x + internal_pos.y * internal_pos.y >= 1) {
        discard;
    }
    
    float width_at_height = sqrt(1 - internal_pos.y * internal_pos.y);
    float px2 = asin(internal_pos.x / width_at_height) * 2/pi;

    float light_angle2 = light_angle + 0.25;
    if(light_angle2 < 0) {
        light_angle2 += 1;
    }

    float light_xcoord = (light_angle2) * 4 - 2;
    float shadow_xcoord0 = light_xcoord - 1;
    float shadow_xcoord1 = light_xcoord + 1;

    if(px2 > shadow_xcoord0 && px2 < shadow_xcoord1) {
        gl_FragColor = vec4(color / 4, 1.0);
    } else {
        gl_FragColor = vec4(color, 1.0);
    }
}
)""";

const char* fragment_shader_c = R"""(
#version 460 core

in vec2 internal_pos;

layout(location = 7) uniform vec3 color;

void main() {
    if(internal_pos.x * internal_pos.x + internal_pos.y * internal_pos.y >= 1) {
        discard;
    }
    gl_FragColor = vec4(color, 1.0);
}
)""";

const char* vertex_shader_x = R"""(
#version 460 core
layout(location = 2) in vec2 pos;

layout(location = 0) uniform float z_value;
layout(location = 1) uniform vec2 window_size;
layout(location = 3) uniform vec4 display_rect;

void main() {
    gl_Position = vec4((pos * display_rect.zw + display_rect.xy) / window_size * 2, z_value, 1.0);
}
)""";

const char* fragment_shader_x = R"""(
#version 460 core

layout(location = 7) uniform vec3 color;

void main() {
    gl_FragColor = vec4(color, 1.0);
}
)""";

double gravitational_constant = 10000;

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

//operators

bool collision(std::array<double, 4> a, std::array<double, 4> b) {
    return (a[0] <= b[0] + b[2] && a[0] + a[2] >= b[0] && a[1] <= b[1] + b[3] && a[1] + a[3] >= b[1]);
}

bool point_collision(std::array<double, 4> a, std::array<double, 2> b) {
    return (b[0] >= a[0] && b[0] <= a[0] + a[2] && b[1] >= a[1] && b[1] <= a[1] + a[3]);
}

bool circle_point_collision(std::array<double, 2> a, std::array<double, 2> b, double radius) {
    return hypot(a[0] - b[0], a[1] - b[1]) <= radius;
}

bool collision(std::array<float, 4> a, std::array<float, 4> b) {
    return (a[0] <= b[0] + b[2] && a[0] + a[2] >= b[0] && a[1] <= b[1] + b[3] && a[1] + a[3] >= b[1]);
}

bool point_collision(std::array<float, 4> a, std::array<float, 2> b) {
    return (b[0] >= a[0] && b[0] <= a[0] + a[2] && b[1] >= a[1] && b[1] <= a[1] + a[3]);
}

bool circle_point_collision(std::array<float, 2> a, std::array<float, 2> b, float radius) {
    return hypot(a[0] - b[0], a[1] - b[1]) <= radius;
}

//

float vertices[] = {
    0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
};

float vertices_x[] = {
    0.0f, 0.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f
};

std::array<double, 2> camera_pos;
float scale = 24;
double sqrt1_5 = std::sqrt(1.5);

std::map<int, bool> user_input_array = {
    {GLFW_KEY_W, false},
    {GLFW_KEY_S, false},
    {GLFW_KEY_A, false},
    {GLFW_KEY_D, false},
    {GLFW_KEY_SPACE, false},
    {GLFW_KEY_V, false},
    {GLFW_KEY_B, false},
    {GLFW_KEY_N, false},
    {GLFW_KEY_M, false},
    {GLFW_MOUSE_BUTTON_LEFT, false}
};

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS) {
        if(key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
        else if(key == GLFW_KEY_EQUAL) scale *= 1.2;
        else if(key == GLFW_KEY_MINUS) scale /= 1.2;
        else if(user_input_array.contains(key)) {
            user_input_array[key] = true;
        }
    } else if(action == GLFW_RELEASE) {
       if(user_input_array.find(key) != user_input_array.end()) user_input_array[key] = false;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if(yoffset > 0) scale *= sqrt1_5;
    else scale /= sqrt1_5;
}

std::array<double, 2> mouse_pos;
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if(user_input_array[GLFW_MOUSE_BUTTON_LEFT]) {
        double mouse_pos_change_x = xpos - mouse_pos[0];
        double mouse_pos_change_y = ypos - mouse_pos[1];

        camera_pos[0] -= mouse_pos_change_x / scale;
        camera_pos[1] += mouse_pos_change_y / scale;
    }
    mouse_pos = {xpos, ypos};
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT) {
        if(action == GLFW_PRESS) {
            user_input_array[GLFW_MOUSE_BUTTON_LEFT] = true;
        } else if(action == GLFW_RELEASE) {
            user_input_array[GLFW_MOUSE_BUTTON_LEFT] = false;
        }
    }
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

bool check_point_inclusion(std::array<double, 4> intrect, std::array<double, 2> point) {
    if(point[0] >= intrect[0] && point[0] < intrect[0] + intrect[2] && point[1] >= intrect[1] && point[1] < intrect[1] + intrect[3]) return true;
    return false;
}

bool check_point_inclusion(std::array<int, 4> intrect, std::array<double, 2> point) {
    if(point[0] >= intrect[0] && point[0] < intrect[0] + intrect[2] && point[1] >= intrect[1] && point[1] < intrect[1] + intrect[3]) return true;
    return false;
}

//structs

enum direction{NORTH, EAST, SOUTH, WEST, NORTHEAST, SOUTHEAST, SOUTHWEST, NORTHWEST, NONE};
enum speed{NORMAL, FAST};

struct Camera {
    std::array<double, 2>* position;
    float* scale;
    int direction = NONE;
    int speed = NORMAL;

    void construct(std::array<double, 2>* pposition, float* pscale) {
        position = pposition;
        scale = pscale;
    }

    void tick(double delta) {
        if(direction != NONE) {
            double change_x = 0.0;
            double change_y = 0.0;
            
            switch(direction) {
                case NORTH:
                    change_y = 1.0 * delta;
                    break;
                case NORTHEAST:
                    change_x = 0.7071 * delta;
                    change_y = 0.7071 * delta;
                    break;
                case EAST:
                    change_x = 1.0 * delta;
                    break;
                case SOUTHEAST:
                    change_x = 0.7071 * delta;
                    change_y = -0.7071 * delta;
                    break;
                case SOUTH:
                    change_y = -1.0 * delta;
                    break;
                case SOUTHWEST:
                    change_x = -0.7071 * delta;
                    change_y = -0.7071 * delta;
                    break;
                case WEST:
                    change_x = -1.0 * delta;
                    break;
                case NORTHWEST:
                    change_x = -0.7071 * delta;
                    change_y = 0.7071 * delta;
                    break;
            }

            if(speed == NORMAL) {
                (*position)[0] += change_x / *scale;
                (*position)[1] += change_y / *scale;
            } else if(speed == FAST) {
                (*position)[0] += change_x * 2.5 / *scale;
                (*position)[1] += change_y * 2.5 / *scale;
            }
        }
    }

    void process_key_input() {
        if(user_input_array[GLFW_KEY_W]) {
            if(user_input_array[GLFW_KEY_D]) {
                direction = NORTHEAST;
            } else if(user_input_array[GLFW_KEY_S]) {
                direction = NONE;
            } else if(user_input_array[GLFW_KEY_A]) {
                direction = NORTHWEST;
            } else {
                direction = NORTH;
            }
        } else if(user_input_array[GLFW_KEY_D]) {
            if(user_input_array[GLFW_KEY_S]) {
                direction = SOUTHEAST;
            } else if(user_input_array[GLFW_KEY_A]) {
                direction = NONE;
            } else {
                direction = EAST;
            }
        } else if(user_input_array[GLFW_KEY_S]) {
            if(user_input_array[GLFW_KEY_A]) {
                direction = SOUTHWEST;
            } else {
                direction = SOUTH;
            }
        } else if(user_input_array[GLFW_KEY_A]) {
            direction = WEST;
        } else {
            direction = NONE;
        }

        if(user_input_array[GLFW_KEY_SPACE]) {
            speed = FAST;
        } else {
            speed = NORMAL;
        }

        /*if(user_input_array[GLFW_KEY_V]) {
            axial_tilt -= 0.1;
        } else if(user_input_array[GLFW_KEY_B]) {
            axial_tilt += 0.1;
        }

        if(user_input_array[GLFW_KEY_N]) {
            rotation_angle -= 0.0005;
        } else if(user_input_array[GLFW_KEY_M]) {
            rotation_angle += 0.0005;
        }*/
    }
};

enum shader_type{TEXTURE_SHADOW, COLOR_SHADOW, COLOR};

unsigned int x_shader;

struct Planemo {
    unsigned int shader;
    unsigned int shader_type;
    unsigned int texture;
    std::array<float, 3> color;

    std::array<double, 2> position;
    double mass;
    double radius;
    double density;

    Planemo* parent = NULL;
    Planemo* light_source = NULL;
    double orbital_period;
    double orbital_radius;
    double start_angle;
    bool retrograde;
    double rotational_period;
    bool retrograde_rotation;
    double axial_tilt;

    void construct(double pmass, double pdensity, double prot_period, bool pretrograde_rot, double paxial_tilt) {
        mass = pmass;
        density = pdensity;
        radius = cbrt((3 * pmass) / (4 * pdensity * M_PI)) * 10;

        rotational_period = prot_period;
        retrograde_rotation = pretrograde_rot;
        axial_tilt = paxial_tilt;
    }

    void set_orbit(Planemo* pparent, Planemo* plight_source, double porbital_radius, double pstart_angle, bool pretrograde) {
        parent = pparent;
        light_source = plight_source;
        orbital_radius = porbital_radius;
        start_angle = pstart_angle;
        retrograde = pretrograde;
        orbital_period = sqrt(orbital_radius * orbital_radius * orbital_radius / (gravitational_constant * parent->mass)) * 2 * M_PI;
    }

    void set_shader(unsigned int pshader_type, unsigned int pshader, unsigned int ptexture, std::array<float, 3> pcolor) {
        shader = pshader;
        shader_type = pshader_type;
        texture = ptexture;
        color = pcolor;
    }

    void set_position(clock_t time) {
        double angle = ((double(time) / (1000 * orbital_period)) * 2 + start_angle / 180) * M_PI;
        if(parent != NULL) position = {parent->position[0] + cos(angle) * orbital_radius, parent->position[1] + sin(angle) * orbital_radius};
    }

    void render(int window_width, int window_height, clock_t time) {
        glUseProgram(shader);

        double scaled_diameter = radius * 2 * scale;

        if(scaled_diameter < 5) {
            std::array<double, 4> bounding_box = {int((position[0] - camera_pos[0]) * scale) - 8, int((position[1] - camera_pos[1]) * scale) - 8, 15, 15};
            if(collision(bounding_box, std::array<double, 4>{-window_width * 0.5, -window_height * 0.5, (double)window_width, (double)window_height})) {
                glUseProgram(x_shader);

                glUniform1f(0, 0.5f);
                glUniform2f(1, window_width, window_height);
                glUniform4f(3, bounding_box[0], bounding_box[1], bounding_box[2], bounding_box[3]);
                glUniform3f(7, color[0], color[1], color[2]);

                glDrawArrays(GL_LINES, 0, 4);
            }

        } else {
            std::array<double, 4> bounding_box = {(position[0] - radius - camera_pos[0]) * scale, (position[1] - radius - camera_pos[1]) * scale, radius * 2 * scale, radius * 2 * scale};
            if(collision(bounding_box, std::array<double, 4>{-window_width * 0.5, -window_height * 0.5, (double)window_width, (double)window_height})) {
                double difference_x, difference_y;
                switch(shader_type) {
                    case TEXTURE_SHADOW:
                        glBindTexture(GL_TEXTURE_2D, texture);
                        glUniform1f(0, 0.5f);
                        glUniform2f(1, window_width, window_height);
                        glUniform4f(3, bounding_box[0], bounding_box[1], bounding_box[2], bounding_box[3]);
                        glUniform4f(7, 0.0f, 0.0f, 1.0f, 1.0f);
                        glUniform1f(11, axial_tilt);
                        glUniform1f(12, (double(time) / 1000) / rotational_period);
                        difference_x = light_source->position[0] - position[0];
                        difference_y = light_source->position[1] - position[1];
                        glUniform1f(13, atan2(difference_y, difference_x) / (2 * M_PI));

                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        break;

                    case COLOR_SHADOW:
                        glUniform1f(0, 0.5f);
                        glUniform2f(1, window_width, window_height);
                        glUniform4f(3, bounding_box[0], bounding_box[1], bounding_box[2], bounding_box[3]);
                        glUniform3f(7, color[0], color[1], color[2]);
                        difference_x = light_source->position[0] - position[0];
                        difference_y = light_source->position[1] - position[1];
                        glUniform1f(10, atan2(difference_y, difference_x) / (2 * M_PI));

                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        break;


                    case COLOR:
                        glUniform1f(0, 0.5f);
                        glUniform2f(1, window_width, window_height);
                        glUniform4f(3, bounding_box[0], bounding_box[1], bounding_box[2], bounding_box[3]);
                        glUniform3f(7, color[0], color[1], color[2]);

                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        break;
                }
            }
        }
    }
};

int main() {
    clock_t start_time = clock();
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
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwMakeContextCurrent(window);

    gladLoadGL();

    glfwSwapInterval(0);

    unsigned int VBO, VAO, VBOx;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VBOx);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, VBOx);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_x), vertices_x, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    int width, height, frames = 0;

    stbi_set_flip_vertically_on_load(true);
    unsigned int planet_texture = generate_texture("world_map3.png", {});
    unsigned int planet_texture2 = generate_texture("world_map2.png", {});

    unsigned int texture_shadow_shader = create_shader(vertex_shader_ts, fragment_shader_ts);
    unsigned int color_shadow_shader = create_shader(vertex_shader_c_cs, fragment_shader_cs);
    unsigned int color_shader = create_shader(vertex_shader_c_cs, fragment_shader_c);
    x_shader = create_shader(vertex_shader_x, fragment_shader_x);

    Camera cam; 
    cam.construct(&camera_pos, &scale);

    std::vector<Planemo> planemo_v(11);
    planemo_v[0].construct(86000, 3.9, 0, false, 0);
    planemo_v[0].set_shader(COLOR, color_shader, 0, {1.0f, 0.4f, 0.0f});
    planemo_v[0].position = {0, 0};

    planemo_v[1].construct(20, 5.1, 60, false, 15);
    planemo_v[1].set_shader(TEXTURE_SHADOW, texture_shadow_shader, planet_texture, {0.2f, 0.5f, 1.0f});
    planemo_v[1].set_orbit(&planemo_v[0], &planemo_v[0], 13000, 15, false);

    planemo_v[2].construct(9, 4.7, 97, false, 4);
    planemo_v[2].set_shader(TEXTURE_SHADOW, texture_shadow_shader, planet_texture2, {1.0f, 0.8f, 0.3f});
    planemo_v[2].set_orbit(&planemo_v[0], &planemo_v[0], 8000, -20, false);
    
    planemo_v[3].construct(29, 6.3, 0, false, 0);
    planemo_v[3].set_shader(COLOR_SHADOW, color_shadow_shader, 0, {0.6f, 0.55f, 0.6f});
    planemo_v[3].set_orbit(&planemo_v[0], &planemo_v[0], 2500, 170, false);

    while(!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glfwPollEvents();

        //keys

        cam.process_key_input();

        //math
        
        double delta_time = clock() - time_storage;
        time_storage = clock();

        cam.tick(delta_time);

        for(int i = 0; i < planemo_v.size(); i++) {
            if(planemo_v[i].parent != NULL) planemo_v[i].set_position(clock() - start_time);
        }

        //rendering

        //glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for(Planemo p : planemo_v) {
            p.render(width, height, clock() - start_time);
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

    return 0;
}
