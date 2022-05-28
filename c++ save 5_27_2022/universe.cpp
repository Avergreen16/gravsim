#define _USE_MATH_DEFINES
#include <SFML-2.5.1\include\SFML\Graphics.hpp>
#include <iostream>
#include <cmath>
#include <ctime>
#include <vector>
#include <sstream>
#include <iomanip>

#include "Spaceobjects.cpp"
#include "shaders.cpp"

sf::Vector2<double> get_orbital_speed(double parent_mass, double orbital_radius, double angle) {
    double orbital_speed = sqrt(G * parent_mass / orbital_radius);
    return sf::Vector2<double>(orbital_speed * cos(angle), orbital_speed * sin(angle));
}

void add_satellite(sf::Shader* p_shader, Planemo* satellite, Planemo* parent, bool retrograde, Planemo* light_source, double orbital_radius, double angle, double p_mass, double p_density, sf::Vector3f color, std::string p_type, bool p_scrosshair, bool is_star = false, int p_class = 0) {
    double radian_angle = angle * M_PI / 180;
    sf::Vector2<double> position(parent->grav.position.x + orbital_radius * cos(radian_angle), parent->grav.position.y + orbital_radius * sin(radian_angle));

    double velocity_angle = radian_angle + M_PI_2 * (-1 + 2 * retrograde);
    sf::Vector2<double> velocity = get_orbital_speed(parent->grav.mass, orbital_radius, velocity_angle);

    if(is_star) satellite->star(p_shader, satellite, parent, retrograde, p_mass, p_density, position, velocity, color, p_type, p_class, p_scrosshair);
    else satellite->planet(p_shader, satellite, parent, retrograde, light_source, p_mass, p_density, position, velocity, color, p_type, p_scrosshair);
}

bool compare_planet_radius(Planemo* p1, Planemo* p2) {
    return p1->radius > p2->radius;
}

bool compare_planet_mass(Planemo* p1, Planemo* p2) {
    return p1->grav.mass > p2->grav.mass ? true : p1->grav.mass == p2->grav.mass ? p1->radius > p2->radius : false;
}

bool compare_planet_distance(Planemo* p1, Planemo* p2) { 
    return hypot(p1->grav.position.x, p1->grav.position.y) < hypot(p2->grav.position.x, p2->grav.position.y);
}

struct Solar_system {
    std::vector<Planemo*> spaceobjects;

    std::vector<Planemo*> stars;
};

bool is_planet_clicked(Planemo* p, sf::Vector2<double> camera_pos, sf::Vector2f mouse_pos, double scale) {
    sf::RectangleShape rect;
    rect.setSize(sf::Vector2f(p->radius * 2 * scale, p->radius * 2 * scale));
    if(rect.getSize().x < 1.5) {
        if(p->small_crosshair) {
            rect.setSize(sf::Vector2f(7, 7));
            rect.setPosition((sf::Vector2f)((-camera_pos + p->view_position) * scale + ((sf::Vector2<double>)p->window->getSize() / (double)2 - sf::Vector2<double>(3.5, 3.5))));
            if(collision(rect, sf::RectangleShape((sf::Vector2f)p->window->getSize())) && point_collision(rect, mouse_pos)) {
                return true;
            }
        } else {
            rect.setSize(sf::Vector2f(15, 15));
            rect.setPosition((sf::Vector2f)((-camera_pos + p->view_position) * scale + ((sf::Vector2<double>)p->window->getSize() / (double)2 - sf::Vector2<double>(7.5, 7.5))));
            if(collision(rect, sf::RectangleShape((sf::Vector2f)p->window->getSize())) && point_collision(rect, mouse_pos)) {
                return true;
            }
        }
    } else {
        rect.setPosition((sf::Vector2f)((-camera_pos + p->view_position) * scale + ((sf::Vector2<double>)p->window->getSize() / (double)2 - sf::Vector2<double>(p->radius * scale, p->radius * scale))));
        if(collision(rect, sf::RectangleShape((sf::Vector2f)p->window->getSize())) && point_collision(rect, mouse_pos) && circle_point_collision(mouse_pos, rect.getPosition() + sf::Vector2f(p->radius * scale, p->radius * scale), p->radius * scale)) {
            return true;
        }
    }
    return false;
}

enum view_setting{ORBIT, RADIUS, MASS, DISTANCE};
enum view_scale{STAR, GALAXY};

std::vector<Planemo*> p_vector;

int active_view_setting = ORBIT;
int active_view_scale = STAR;
Planemo* focus_object = NULL;
bool sorted = true;
bool center_focus_object = true;

sf::Sprite sprite;

std::vector<std::string> selected_object_text = {
    "Selected object: ",
    "Parent: ",
    "Mass: ",
    "Radius: ",
    "Density: ",
    "Type: "
};

int main() {
    shadow_shader.loadFromMemory(shadow_fragment_shader, sf::Shader::Fragment);
    star_shader.loadFromMemory(star_fragment_shader, sf::Shader::Fragment);
    dark_shader.loadFromMemory(dark_fragment_shader, sf::Shader::Fragment);
    circle_shader.loadFromMemory(circle_fragment_shader, sf::Shader::Fragment);

    std::vector<sf::Texture> imgvector(5);
    
    imgvector[0].loadFromFile("startextures.png", sf::IntRect(92, 0, 15, 15));

    imgvector[1].loadFromFile("startextures.png", sf::IntRect(77, 0, 15, 15));

    imgvector[2].loadFromFile("startextures.png", sf::IntRect(54, 0, 23, 23));

    imgvector[3].loadFromFile("startextures.png", sf::IntRect(31, 0, 23, 23));

    imgvector[4].loadFromFile("startextures.png", sf::IntRect(0, 0, 31, 31));

    //setup
    int frame = 0;
    int timeContainer = 0;
    sf::Clock clock;

    sf::Font font;
    font.loadFromFile("Q5text.ttf");
    sf::Text text;
    text.setFont(font);

    unsigned int window_width = 1600, window_height = 800;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Test Name", sf::Style::Close | sf::Style::Titlebar | sf::Style::Resize);
    window.setFramerateLimit(100);

    double scale = 0.5 * pow(1.2, -14);
    sf::Vector2<double> camera_pos(0.0f, 0.0f);
    sf::Vector2f mouse_pos(0.0f, 0.0f);

    //planets, stars, and moons
    RNG rng;
    rng.set_seed(0.7987987, 49.97843);
    Solar_system ss1;

    for(int i = 0; i < 100; i++) {
        ss1.spaceobjects.push_back(new Planemo(&window, &clock, std::to_string(i)));
    }

    ss1.spaceobjects[0]->star(&star_shader, ss1.spaceobjects[0], NULL, false, 193000, pow((double)193000 / 200000 + 0.1 * rng.retrieve(), -1.3) * 1.4, sf::Vector2<double>(0, 0), sf::Vector2<double>(0, 0), sf::Vector3f(1.0f, 0.8f, 0.2f), "G-class star", 2, false);
    ss1.stars.push_back(ss1.spaceobjects[0]);
    
    add_satellite(&star_shader, ss1.spaceobjects[1], ss1.spaceobjects[0], false, NULL, 4000, 360 * rng.retrieve(), 61000, pow((double)61000 / 200000 + 0.1 * rng.retrieve(), -1.3) * 1.4, sf::Vector3f(1.0f, 0.2f, 0.0f), "M-class star", false, true, 0);

    for(int i = 1; i <= 15; i++) {
        add_satellite(&shadow_shader, ss1.spaceobjects[1 + i], ss1.spaceobjects[0], false, ss1.spaceobjects[0], 25000 + 20000 * rng.retrieve(), 360 * rng.retrieve(), 0.002 + rng.retrieve() * 0.005, 2.9 + rng.retrieve() * 1.2, sf::Vector3f(0.4 + rng.retrieve() * 0.2, 0.4 + rng.retrieve() * 0.05, 0.4), "terrestrial asteroid", true);
    }

    add_satellite(&shadow_shader, ss1.spaceobjects[17], ss1.spaceobjects[0], false, ss1.spaceobjects[0], 55000, 360 * rng.retrieve(), 17, 4.7, sf::Vector3f(0.55f, 0.4f, 0.5f), "Barren Metallic Terrestrial", false);

    add_satellite(&shadow_shader, ss1.spaceobjects[18], ss1.spaceobjects[17], false, ss1.spaceobjects[0], 500, 360 * rng.retrieve(), 11, 4.1, sf::Vector3f(0.61f, 0.6f, 0.63f), "Barren Terrestrial", false);

    add_satellite(&shadow_shader, ss1.spaceobjects[19], ss1.spaceobjects[0], false, ss1.spaceobjects[0], 90000, 360 * rng.retrieve(), 9600, 1.9, sf::Vector3f(0.7f, 0.65f, 0.4f), "Gas Giant", false);

    double moon20_angle = 360 * rng.retrieve();
    add_satellite(&shadow_shader, ss1.spaceobjects[20], ss1.spaceobjects[19], false, ss1.spaceobjects[0], 1950, moon20_angle,  0.32, 4.3, sf::Vector3f(0.5f, 0.5f, 0.3f), "Barren Terrestrial", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[21], ss1.spaceobjects[19], false, ss1.spaceobjects[0], 1950, moon20_angle + 60, 0.25, 4.1, sf::Vector3f(0.5f, 0.4f, 0.3f), "Barren Terrestrial", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[22], ss1.spaceobjects[19], false, ss1.spaceobjects[0], 3900, 360 * rng.retrieve(), 0.13, 3.2, sf::Vector3f(0.65f, 0.6f, 0.6f), "Barren Terrestrial", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[23], ss1.spaceobjects[19], false, ss1.spaceobjects[0], 7800, 360 * rng.retrieve(), 0.07, 2.6, sf::Vector3f(0.5f, 0.5f, 0.7f), "Barren Iceball", false);

    add_satellite(&shadow_shader, ss1.spaceobjects[24], ss1.spaceobjects[0], false, ss1.spaceobjects[0], 130000, 360 * rng.retrieve(), 1100, 1.5, sf::Vector3f(0.4f, 0.2f, 1.0f), "Ice Giant", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[25], ss1.spaceobjects[24], false, ss1.spaceobjects[0], 860, 360 * rng.retrieve(), 0.06, 2.0, sf::Vector3f(0.7f, 0.7f, 0.9f), "Barren Iceball", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[26], ss1.spaceobjects[24], false, ss1.spaceobjects[0], 1500, 360 * rng.retrieve(), 0.18, 2.4, sf::Vector3f(0.7f, 0.6f, 1.0f), "Barren Iceball", false);

    add_satellite(&shadow_shader, ss1.spaceobjects[27], ss1.spaceobjects[0], false, ss1.spaceobjects[0], 147000, 360 * rng.retrieve(), 1.34, 2.6, sf::Vector3f(0.6f, 0.7f, 0.9f), "Barren Iceball", false);

    for(int i = 1; i <= 18; i++) {
        add_satellite(&shadow_shader, ss1.spaceobjects[27 + i], ss1.spaceobjects[0], false, ss1.spaceobjects[0], 158000 + 30000 * rng.retrieve(), 360 * rng.retrieve(), 0.001 + rng.retrieve() * 0.009, 1.5 + rng.retrieve() * 1.3, sf::Vector3f(0.6 + 0.25 * rng.retrieve(), 0.6, 0.7 + 0.3 * rng.retrieve()), "iceball asteroid", true);
    }

    ss1.spaceobjects[46]->star(&star_shader, ss1.spaceobjects[46], NULL, false, 294000, pow((double)294000 / 200000 + 0.1 * rng.retrieve(), -1.3) * 1.4, sf::Vector2<double>(-3000000, 1000000), sf::Vector2<double>(0, 0), sf::Vector3f(1.0f, 1.0f, 0.8f), "F-class star", 3, false);
    ss1.stars.push_back(ss1.spaceobjects[46]);

    add_satellite(&shadow_shader, ss1.spaceobjects[47], ss1.spaceobjects[46], false, ss1.spaceobjects[46], 7500, 360 * rng.retrieve(), 45, 6.7, sf::Vector3f(0.4, 0.35, 0.2), "hot metallic barren terrestrial", false);

    add_satellite(&shadow_shader, ss1.spaceobjects[48], ss1.spaceobjects[46], false, ss1.spaceobjects[46], 18500, 360 * rng.retrieve(), 13, 4.9, sf::Vector3f(0.4, 0.4, 0.3), "barren terrestrial", false);

    for(int i = 1; i <= 13; i++) {
        add_satellite(&shadow_shader, ss1.spaceobjects[48 + i], ss1.spaceobjects[46], false, ss1.spaceobjects[46], 25000 + 21000 * rng.retrieve(), 360 * rng.retrieve(), 0.001 + rng.retrieve() * 0.006, 2.9 + rng.retrieve() * 1.2, sf::Vector3f(0.4 + rng.retrieve() * 0.2, 0.4 + rng.retrieve() * 0.05, 0.4), "terrestrial asteroid", true);
    }

    add_satellite(&shadow_shader, ss1.spaceobjects[62], ss1.spaceobjects[46], false, ss1.spaceobjects[46], 34000, 360 * rng.retrieve(), 0.13, 3.8, sf::Vector3f(0.4, 0.5, 0.4), "barren terrestrial", false);

    float planet19_angle = 360 * rng.retrieve();

    add_satellite(&shadow_shader, ss1.spaceobjects[63], ss1.spaceobjects[46], false, ss1.spaceobjects[46], 73000, planet19_angle, 923, 1.7, sf::Vector3f(0.3, 0.7, 0.9), "ice giant", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[64], ss1.spaceobjects[63], false, ss1.spaceobjects[46], 806, 360 * rng.retrieve(), 0.03, 3.9, sf::Vector3f(0.6, 0.5, 0.5), "barren terrestrial", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[65], ss1.spaceobjects[63], false, ss1.spaceobjects[46], 1250, 360 * rng.retrieve(), 0.13, 4.2, sf::Vector3f(0.4, 0.3, 0.6), "barren terrestrial", false);

    add_satellite(&shadow_shader, ss1.spaceobjects[66], ss1.spaceobjects[46], false, ss1.spaceobjects[46], 73000, planet19_angle + 60, 16, 4.8, sf::Vector3f(0.5, 0.9, 0.7), "wet terrestrial with life", false);

    add_satellite(&shadow_shader, ss1.spaceobjects[67], ss1.spaceobjects[46], false, ss1.spaceobjects[46], 157000, 360 * rng.retrieve(), 2900, 1.4, sf::Vector3f(0.6, 0.5, 0.4), "gas giant", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[68], ss1.spaceobjects[67], false, ss1.spaceobjects[46], 1780, 360 * rng.retrieve(), 0.04, 3.9, sf::Vector3f(0.6, 0.6, 0.4), "barren terrestrial", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[69], ss1.spaceobjects[67], false, ss1.spaceobjects[46], 2600, 360 * rng.retrieve(), 0.41, 3.6, sf::Vector3f(0.6, 0.3, 0.6), "cold storm world", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[70], ss1.spaceobjects[67], false, ss1.spaceobjects[46], 3600, 360 * rng.retrieve(), 0.03, 2.7, sf::Vector3f(0.7, 0.7, 0.8), "barren iceball", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[71], ss1.spaceobjects[67], false, ss1.spaceobjects[46], 4550, 360 * rng.retrieve(), 0.02, 2.8, sf::Vector3f(0.5, 0.6, 0.8), "barren iceball", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[72], ss1.spaceobjects[67], false, ss1.spaceobjects[46], 7900, 360 * rng.retrieve(), 0.025, 2.8, sf::Vector3f(0.5, 0.5, 0.7), "barren iceball", false);

    add_satellite(&shadow_shader, ss1.spaceobjects[73], ss1.spaceobjects[46], false, ss1.spaceobjects[46], 223000, 360 * rng.retrieve(), 120, 3.8, sf::Vector3f(0.6, 0.45, 0.25), "gas dwarf", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[74], ss1.spaceobjects[73], false, ss1.spaceobjects[46], 580, 360 * rng.retrieve(), 0.44, 2.8, sf::Vector3f(0.6, 0.4, 0.5), "barren iceball", false);
    add_satellite(&shadow_shader, ss1.spaceobjects[75], ss1.spaceobjects[73], false, ss1.spaceobjects[46], 5600, 360 * rng.retrieve(), 0.03, 1.8, sf::Vector3f(0.65, 0.7, 0.9), "barren iceball", false);

    add_satellite(&shadow_shader, ss1.spaceobjects[76], ss1.spaceobjects[46], false, ss1.spaceobjects[46], 265000, 360 * rng.retrieve(), 0.23, 1.7, sf::Vector3f(0.5, 0.7, 0.9), "barren iceball", false);

    for(int i = 1; i <= 23; i++) {
        add_satellite(&shadow_shader, ss1.spaceobjects[76 + i], ss1.spaceobjects[46], false, ss1.spaceobjects[46], 270000 + 70000 * rng.retrieve(), 360 * rng.retrieve(), 0.001 + rng.retrieve() * 0.004, 1.5 + rng.retrieve() * 1.8, sf::Vector3f(0.6 + rng.retrieve() * 0.1, 0.7, 0.6 + rng.retrieve() * 0.3), "icy asteroid", true);
    }

    /*for(int i = 0; i < 100; i++) {
        if(rng.retrieve() > 0.2 && i != 14) {
            int mass = 50000 + rng.retrieve() * 350000;
            int mk_class;
            std::string type;
            sf::Vector3f color;
            float rng_val = rng.retrieve();
            if(mass < 100000) {
                color = sf::Vector3f(1.0f, 0.3f * rng_val, 0.0f);
                mk_class = 0;
                type = "M-class star";
            } else if(mass < 150000) {
                color = sf::Vector3f(1.0f, 0.4 + 0.35f * rng_val, 0.0f);
                mk_class = 1;
                type = "K-class star";
            } else if(mass < 250000) {
                color = sf::Vector3f(1.0f, 0.8 + 0.2f * rng_val, 0.1 + 0.5f * rng_val);
                mk_class = 2;
                type = "G-class star";
            } else if(mass < 350000) {
                color = sf::Vector3f(1.0f, 1.0f, 0.7f + 0.3f * rng_val);
                mk_class = 3;
                type = "F-class star";
            } else {
                color = sf::Vector3f(1.0f - 0.5 * rng_val, 1.0 - 0.5 * rng_val, 1.0f);
                mk_class = 4;
                type = "A-class star";
            }
            ss1.spaceobjects[56 + i]->planemo->star(&star_shader, ss1.spaceobjects[57 + i], NULL, false, mass, pow((double)mass / 200000 + 0.1 * rng.retrieve(), -1.3) * 1.4, sf::Vector2<double>(i % 10 * 5000000 + rng.retrieve() * 4000000 + 500000 - 13000000, int(i / 10) * 5000000 + rng.retrieve() * 4000000 + 500000 - 13500000), sf::Vector2<double>(0, 0), color, type, mk_class, false);
            ss1.stars.push_back(ss1.spaceobjects[56 + i]->planemo);
        }
    }*/

    for(int i = 0; i < ss1.spaceobjects.size(); i++) {
        if(ss1.spaceobjects[i]->grav.self == NULL) {
            ss1.spaceobjects.erase(ss1.spaceobjects.begin() + i);
            i--;
        }
    }
    
    //loop
    while(window.isOpen()) {
        //input
        sf::Event event_;
        while(window.pollEvent(event_)) {
            switch(event_.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::Resized:
                    window_width = event_.size.width;
                    window_height = event_.size.height;

                    window.setSize(sf::Vector2u(window_width, window_height));
                    break;
                case sf::Event::MouseMoved:
                    if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                        camera_pos -= ((sf::Vector2<double>)sf::Mouse::getPosition(window) - (sf::Vector2<double>)mouse_pos)/scale;
                    }
                    mouse_pos = (sf::Vector2f)sf::Mouse::getPosition(window);
                    break;
                case sf::Event::MouseWheelScrolled:
                    int delta = event_.mouseWheelScroll.delta;
                    scale *= pow(1.2, delta);

                    if(scale < 0.00008) {
                        active_view_scale = GALAXY;
                    } else {
                        active_view_scale = STAR;
                    }

                    break;
            }
            if(event_.type == event_.MouseButtonPressed) {
                if(active_view_scale == GALAXY) {
                    focus_object = NULL;
                } else {
                    if(event_.mouseButton.button == sf::Mouse::Right) {
                        bool planet_clicked = false;
                        for(Planemo* p : ss1.spaceobjects) {
                            if(is_planet_clicked(p, camera_pos, mouse_pos, scale)) {
                                focus_object = p;
                                planet_clicked = true;
                                center_focus_object = false;
                                break;
                            }
                        }
                        if(!planet_clicked) {
                            focus_object = NULL;
                        }
                    }
                }
                
            }
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
            active_view_setting = ORBIT;
            center_focus_object = false;
        } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::N)) {
            active_view_setting = RADIUS;
            sorted = false;
            center_focus_object = false;
        } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::M)) {
            active_view_setting = MASS;
            sorted = false;
            center_focus_object = false;
        } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::V)) {
            active_view_setting = DISTANCE;
            sorted = false;
            center_focus_object = false;
        } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::G)) {
            std::cout << "New G-value: ";
            std::cin >> G;
        }

        //rendering
        window.clear();

        for(Planemo* s : ss1.stars) {
            sf::Vector2u tex_size = imgvector[s->mk_class].getSize();
            sprite.setTextureRect(sf::IntRect(0, 0, tex_size.x, tex_size.y));
            sprite.setTexture(imgvector[s->mk_class]);
            sprite.setScale(scale * 12500 * 5, scale * 12500 * 5);
            sprite.setColor(sf::Color(255, 255, 255, std::clamp((1 / (scale * 5000) - 0.05), (double)0, (double)1) * 255));
            sprite.setPosition((sf::Vector2f)((-camera_pos + s->grav.position) * scale + ((sf::Vector2<double>)window.getSize() - (sf::Vector2<double>)tex_size * (scale * 12500 * 5)) * 0.5));
            window.draw(sprite);
        }

        switch(active_view_scale) {
            case STAR:
                sf::Vector2<double> focus_add;
                sf::Vector2<double> coordinate;    
                switch(active_view_setting) {
                    case ORBIT:
                        for(Planemo* p : ss1.spaceobjects) if(p->grav.parent != NULL) p->grav.gravinteract();
                        for(Planemo* p : ss1.spaceobjects) if(p->grav.parent != NULL) p->grav.apply_acceleration();

                        for(Planemo* p : ss1.spaceobjects) p->view_position = p->grav.position;

                        if(!center_focus_object) {
                            if(focus_object != NULL) camera_pos = focus_object->view_position;
                            center_focus_object = true;
                        } else {
                            if(focus_object != NULL) camera_pos += focus_object->grav.get_velocity();
                        }
                        
                        for(Planemo* p : ss1.spaceobjects) p->render(camera_pos, scale);

                        break;

                    case RADIUS:
                        if(!sorted) {
                            std::sort(ss1.spaceobjects.begin(), ss1.spaceobjects.end(), compare_planet_radius);
                            sorted = true;
                        }

                        for(Planemo* p : ss1.spaceobjects) if(p->grav.parent != NULL) p->grav.gravinteract();
                        for(Planemo* p : ss1.spaceobjects) if(p->grav.parent != NULL) p->grav.apply_acceleration();

                        coordinate = sf::Vector2<double>(-ss1.spaceobjects[0]->radius, 0);
                        for(Planemo* p : ss1.spaceobjects) {
                            coordinate.x += p->radius;
                            p->view_position = coordinate;
                            coordinate.x += p->radius;
                        }

                        if(!center_focus_object) {
                            if(focus_object != NULL) camera_pos = focus_object->view_position;
                            center_focus_object = true;
                        }

                        for(Planemo* p : ss1.spaceobjects) p->render(camera_pos, scale);
                        break;
                    
                    case MASS:
                        if(!sorted) {
                            std::sort(ss1.spaceobjects.begin(), ss1.spaceobjects.end(), compare_planet_mass);
                            sorted = true;
                        }

                        for(Planemo* p : ss1.spaceobjects) if(p->grav.parent != NULL) p->grav.gravinteract();
                        for(Planemo* p : ss1.spaceobjects) if(p->grav.parent != NULL) p->grav.apply_acceleration();

                        coordinate = sf::Vector2<double>(-ss1.spaceobjects[0]->radius, 0);
                        for(Planemo* p : ss1.spaceobjects) {
                            coordinate.x += p->radius;
                            p->view_position = coordinate;
                            coordinate.x += p->radius;
                        }

                        if(!center_focus_object) {
                            if(focus_object != NULL) camera_pos = focus_object->view_position;
                            center_focus_object = true;
                        }

                        for(Planemo* p : ss1.spaceobjects) p->render(camera_pos, scale);
                        break;

                    case DISTANCE:
                        if(!sorted) {
                            std::sort(ss1.spaceobjects.begin(), ss1.spaceobjects.end(), compare_planet_distance);
                            sorted = true;
                        }

                        for(Planemo* p : ss1.spaceobjects) if(p->grav.parent != NULL) p->grav.gravinteract();
                        for(Planemo* p : ss1.spaceobjects) if(p->grav.parent != NULL) p->grav.apply_acceleration();

                        coordinate = sf::Vector2<double>(-ss1.spaceobjects[0]->radius, 0);
                        for(Planemo* p : ss1.spaceobjects) {
                            coordinate.x += p->radius;
                            p->view_position = coordinate;
                            coordinate.x += p->radius;
                        }

                        if(!center_focus_object) {
                            if(focus_object != NULL) camera_pos = focus_object->view_position;
                            center_focus_object = true;
                        }

                        for(Planemo* p : ss1.spaceobjects) p->render(camera_pos, scale);
                        break;
                }
        }

        if(focus_object != NULL) {
            std::vector<std::string> active_text = selected_object_text;
            text.setCharacterSize(15);
            text.setFillColor(sf::Color(0xFFFFFFFF));
            active_text[0] += focus_object->name;
            //if(focus_object->grav.parent != NULL) active_text[1] += focus_object->grav.parent->name;

            active_text[2] += num_to_string(focus_object->grav.mass) + " (";
            double mass_percentage = focus_object->grav.mass / 20 * 100;
            if(mass_percentage > 100) {
                active_text[2] += num_to_string(focus_object->grav.mass / 20) + "x";
            } else {
                active_text[2] += num_to_string(mass_percentage) + "%";
            }
            active_text[2] += " Terra mass)";

            active_text[3] += num_to_string(focus_object->radius) + " (";
            double radius_percentage = focus_object->radius / 19.57 * 100;
            if(radius_percentage > 100) {
                active_text[3] += num_to_string(focus_object->radius / 20) + "x";
            } else {
                active_text[3] += num_to_string(radius_percentage) + "%";
            }
            active_text[3] += " Terra radius)";

            active_text[4] += num_to_string(focus_object->density);
            active_text[5] += focus_object->type;

            for(int i = 0; i < active_text.size(); i++) {
                text.setString(active_text[i]);
                text.setPosition(12, 12 + 25 * i);
                window.draw(text);
            }
        } else {
            text.setCharacterSize(15);
            text.setString("scale: " + num_to_string(scale));
            text.setPosition(12, 12);
            window.draw(text);
        }

        window.display();
        frame++;
        if(frame % 1000 == 0 && frame != 0) {
            std::cout << (double)frame / (clock.getElapsedTime().asMilliseconds() - timeContainer) * 1000 << std::endl;
            timeContainer = clock.getElapsedTime().asMilliseconds();
            frame = 0;
        }
    }

    return 0;
}