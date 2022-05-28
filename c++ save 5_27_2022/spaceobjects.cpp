#pragma once
#define _USE_MATH_DEFINES
#include <SFML-2.5.1\include\SFML\Graphics.hpp>
#include <cmath>
#include <vector>

#include "spacefunctions.cpp"

double G = 1;

struct Planemo;
struct Barycenter;
struct Ring;

struct Grav {
    double mass;
    sf::Vector2<double> velocity, acceleration, position;
    Planemo* parent = NULL;
    Planemo* self = NULL;

    void set_properties(double p_mass, sf::Vector2<double> p_pos, sf::Vector2<double> p_vel, Planemo* p_parent, Planemo* p_self);
    void gravinteract();
    void apply_acceleration();
    sf::Vector2<double> get_velocity();
};

struct Planemo {
    std::string name;
    std::string type;
    int mk_class;

    bool is_star = false;

    Grav grav;
    Planemo* light_source;

    sf::RenderWindow* window;
    sf::Shader* shader;
    sf::Clock* clock;

    double density;
    double radius;
    sf::Vector3f color;
    sf::Color sfcolor;

    bool retrograde = false;
    bool small_crosshair = false;

    sf::Vector2<double> view_position;

    sf::RectangleShape draw_rect;

    Planemo(sf::RenderWindow* p_window, sf::Clock* p_clock, std::string p_name);

    void planet(sf::Shader* p_shader, Planemo* p_self, Planemo* p_parent, bool p_retrograde, Planemo* p_light_source, double p_mass, double p_density, sf::Vector2<double> p_position, sf::Vector2<double> p_velocity, sf::Vector3f p_color, std::string p_type, bool p_scrosshair);
    void star(sf::Shader* p_shader, Planemo* p_self, Planemo* p_parent, bool p_retrograde, double p_mass, double p_density, sf::Vector2<double> p_position, sf::Vector2<double> p_velocity, sf::Vector3f p_color, std::string p_type, int p_class, bool p_scrosshair);
    void render(sf::Vector2<double> camera_pos, double scale);
};

void Grav::set_properties(double p_mass, sf::Vector2<double> p_pos, sf::Vector2<double> p_vel, Planemo* p_parent, Planemo* p_self) {
    self = p_self;
    parent = p_parent;
    mass = p_mass;
    position = p_pos;
    velocity = p_vel;
    acceleration = sf::Vector2<double>(0, 0);
}

void Grav::gravinteract() {
    acceleration = sf::Vector2<double>(0, 0);
        
    sf::Vector2<double> difference = parent->grav.position - position;
    double angle = atan2(difference.y, difference.x);
    double grav_pull = G * parent->grav.mass / pow(hypot(difference.x, difference.y), 2);
    acceleration += sf::Vector2<double>(grav_pull * cos(angle), grav_pull * sin(angle));
}

void Grav::apply_acceleration() {
    velocity += acceleration;

    Planemo* recursive_parent = parent;
    while(recursive_parent != NULL) {
        position += recursive_parent->grav.velocity;
        recursive_parent = recursive_parent->grav.parent;
    }

    position += velocity;
}

sf::Vector2<double> Grav::get_velocity() {
    sf::Vector2<double> return_vector(0, 0);
    Planemo* recursive_parent = parent;
    while(recursive_parent != NULL) {
        return_vector += recursive_parent->grav.velocity;
        recursive_parent = recursive_parent->grav.parent;
    }

    return_vector += velocity;

    return return_vector;
}

Planemo::Planemo(sf::RenderWindow* p_window, sf::Clock* p_clock, std::string p_name) {
    window = p_window;
    clock = p_clock;

    name = p_name;
}

void Planemo::planet(sf::Shader* p_shader, Planemo* p_self, Planemo* p_parent, bool p_retrograde, Planemo* p_light_source, double p_mass, double p_density, sf::Vector2<double> p_position, sf::Vector2<double> p_velocity, sf::Vector3f p_color, std::string p_type, bool p_scrosshair) {
    shader = p_shader;
    light_source = p_light_source;
    color = p_color;
    density = p_density;
    radius = cbrt((3 * p_mass) / (4 * p_density * M_PI)) * 20;
    grav.set_properties(p_mass, p_position, p_velocity, p_parent, p_self);
    view_position = p_position;
    retrograde = p_retrograde;
    sfcolor = sf::Color(color.x * 255, color.y * 255, color.z * 255);
    type = p_type;
    small_crosshair = p_scrosshair;
}

void Planemo::star(sf::Shader* p_shader, Planemo* p_self, Planemo* p_parent, bool p_retrograde, double p_mass, double p_density, sf::Vector2<double> p_position, sf::Vector2<double> p_velocity, sf::Vector3f p_color, std::string p_type, int p_class, bool p_scrosshair) {
    shader = p_shader;
    color = p_color;
    density = p_density;
    radius = cbrt((3 * p_mass) / (4 * p_density * M_PI)) * 20;
    grav.set_properties(p_mass, p_position, p_velocity, p_parent, p_self);
    view_position = p_position;
    is_star = true;
    retrograde = p_retrograde;
    sfcolor = sf::Color(color.x * 255, color.y * 255, color.z * 255);
    type = p_type;
    mk_class = p_class;
    small_crosshair = p_scrosshair;
}

void Planemo::render(sf::Vector2<double> camera_pos, double scale) {
    draw_rect.setSize(sf::Vector2f(radius * 2 * scale, radius * 2 * scale));
    if(draw_rect.getSize().x < 1.5) {
        if(small_crosshair) {
            draw_rect.setSize(sf::Vector2f(7, 7));
            draw_rect.setPosition((sf::Vector2f)((-camera_pos + view_position) * scale + ((sf::Vector2<double>)window->getSize() / (double)2 - sf::Vector2<double>(3.5, 3.5))));
            if(collision(draw_rect, sf::RectangleShape((sf::Vector2f)window->getSize()))) {
                sf::Vector2f pos = draw_rect.getPosition();
                pos = sf::Vector2f(round(pos.x), round(pos.y));
                sf::Vertex line0[] {
                    sf::Vertex(sf::Vector2f(pos.x, pos.y), sfcolor),
                    sf::Vertex(sf::Vector2f(pos.x + 7, pos.y + 7), sfcolor)
                };
                sf::Vertex line1[] {
                    sf::Vertex(sf::Vector2f(pos.x + 7, pos.y), sfcolor),
                    sf::Vertex(sf::Vector2f(pos.x, pos.y + 7), sfcolor)
                };
                window->draw(line0, 2, sf::Lines);
                window->draw(line1, 2, sf::Lines);
            }
        } else {
            draw_rect.setSize(sf::Vector2f(15, 15));
            draw_rect.setPosition((sf::Vector2f)((-camera_pos + view_position) * scale + ((sf::Vector2<double>)window->getSize() / (double)2 - sf::Vector2<double>(7.5, 7.5))));
            if(collision(draw_rect, sf::RectangleShape((sf::Vector2f)window->getSize()))) {
                sf::Vector2f pos = draw_rect.getPosition();
                pos = sf::Vector2f(round(pos.x), round(pos.y));
                sf::Vertex line0[] {
                    sf::Vertex(sf::Vector2f(pos.x, pos.y), sfcolor),
                    sf::Vertex(sf::Vector2f(pos.x + 15, pos.y + 15), sfcolor)
                };
                sf::Vertex line1[] {
                    sf::Vertex(sf::Vector2f(pos.x + 15, pos.y), sfcolor),
                    sf::Vertex(sf::Vector2f(pos.x, pos.y + 15), sfcolor)
                };
                window->draw(line0, 2, sf::Lines);
                window->draw(line1, 2, sf::Lines);
            }
        }
    } else {
        draw_rect.setPosition((sf::Vector2f)((-camera_pos + view_position) * scale + ((sf::Vector2<double>)window->getSize() / (double)2 - sf::Vector2<double>(radius * scale, radius * scale))));
        if(collision(draw_rect, sf::RectangleShape((sf::Vector2f)window->getSize()))) {
            if(is_star) {
                shader->setUniform("u_resolution", (sf::Glsl::Vec2)window->getSize());
                shader->setUniform("u_shapeSize", (sf::Glsl::Vec2)draw_rect.getSize());
                shader->setUniform("u_shapePos", (sf::Glsl::Vec2)draw_rect.getPosition());
                shader->setUniform("u_color", (sf::Glsl::Vec3)color);

                window->draw(draw_rect, shader);
            } else {
                if(light_source != NULL) {
                    sf::Vector2<double> difference = light_source->grav.position - grav.position;
                    float lightAngle = atan2(-difference.y, difference.x) * 180 / M_PI;
                    if(lightAngle < 0) {
                        lightAngle = 360 + lightAngle;
                    }

                    shader->setUniform("u_resolution", (sf::Glsl::Vec2)window->getSize());
                    shader->setUniform("u_shapeSize", (sf::Glsl::Vec2)draw_rect.getSize());
                    shader->setUniform("u_shapePos", (sf::Glsl::Vec2)draw_rect.getPosition());
                    shader->setUniform("u_color", (sf::Glsl::Vec3)color);
                    shader->setUniform("u_lightAngle", lightAngle);

                    window->draw(draw_rect, shader);
                } else {
                    shader->setUniform("u_resolution", (sf::Glsl::Vec2)window->getSize());
                    shader->setUniform("u_shapeSize", (sf::Glsl::Vec2)draw_rect.getSize());
                    shader->setUniform("u_shapePos", (sf::Glsl::Vec2)draw_rect.getPosition());
                    shader->setUniform("u_color", (sf::Glsl::Vec3)color);

                    window->draw(draw_rect, shader);
                }
            } 
        }
    }
}

struct Ring;