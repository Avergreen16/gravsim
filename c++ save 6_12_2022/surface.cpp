#define _USE_MATH_DEFINES
#include <SFML-2.5.1\include\SFML\Graphics.hpp>
#include <iostream>
#include <cmath>
#include <ctime>
#include <vector>
#include <sstream>
#include <iomanip>

#include "terraingen.cpp"

const sf::Vector2u chunk_size(16, 16);

const double log10_4 = log10(4);

bool check_point_inclusion(sf::IntRect intrect, sf::Vector2<double> point) {
    if(point.x >= intrect.left && point.x < intrect.left + intrect.width && point.y >= intrect.top && point.y < intrect.top + intrect.height) return true;
    return false;
}

bool check_within_generated_chunk(std::vector<std::pair<std::vector<sf::Color>, sf::IntRect>> chunk_vector, sf::Vector2<double> point) {
    for(std::pair<std::vector<sf::Color>, sf::IntRect> p : chunk_vector) {
        if(check_point_inclusion(p.second, point)) {
            return true;
        }
    }
    return false;
}

template<typename type>
bool rect_collision(sf::Rect<type> rect1, sf::Rect<type> rect2) {
    if(rect1.left < rect2.left + rect2.width && rect1.left + rect1.width > rect2.left && rect1.top < rect2.top + rect2.height && rect1.top + rect1.height > rect2.top) return true;
    return false;
}

struct Counter {
    int pos = 0;
    int lim;

    Counter() {};
    Counter(int p_lim) : lim(p_lim) {};

    bool increment() {
        pos++;
        if(pos >= lim) {
            pos = 0;
            return true;
        }
        return false;
    }

    void reset() {
        pos = 0;
    }
};

enum state{IDLE, RUNNING};
enum direction{NORTH, WEST, SOUTH, EAST};

struct Entity {
    sf::Texture spritesheet;
    sf::Sprite sprite;

    sf::Vector2<double> visual_size = sf::Vector2<double>(2, 2);
    sf::Vector2<double> position = sf::Vector2<double>(821 * 16 + 0.5, 506 * 16 + 0.5);
    sf::Vector2u sprite_size = sf::Vector2u(32, 32);
    sf::Vector2u active_sprite = sf::Vector2u(0, 1);
    sf::Vector2<double> sprite_offset = sf::Vector2<double>(-1, -1.875);

    Counter cycle_timer = Counter(15);
    Counter sprite_counter = Counter(4);

    int state = IDLE;
    int direction = NORTH;

    Entity(std::string p_spritesheet_filepath) {
        spritesheet.loadFromFile(p_spritesheet_filepath);
        sprite.setTexture(spritesheet);
    }
    
    void render(sf::Vector2<double> camera_pos, double scale, sf::RenderWindow* window) {
        sprite.setTextureRect(sf::IntRect(sprite_size.x * active_sprite.x, sprite_size.y * active_sprite.y, sprite_size.x, sprite_size.y));
        sprite.setScale(scale * visual_size.x / double(sprite_size.x), scale * visual_size.y / double(sprite_size.y));
        sprite.setPosition(sf::Vector2f((position + sprite_offset - camera_pos) * scale) + sf::Vector2f(window->getSize()) / 2.0f);
        window->draw(sprite);
    }

    void tick() {
        if(state == RUNNING) {
            if(cycle_timer.increment()) {
                sprite_counter.increment();
            }
            active_sprite.x = sprite_counter.pos;
            active_sprite.y = direction;

            if(direction == NORTH) position.y -= 0.05 * 6.4;
            else if(direction == WEST) position.x -= 0.05 * 6.4;
            else if(direction == SOUTH) position.y += 0.05 * 6.4;
            else if(direction == EAST) position.x += 0.05 * 6.4;
        } else if(state == IDLE) {
            sprite_counter.reset();
            cycle_timer.reset();
            active_sprite.x = 3;
        }
    }
};

std::vector<sf::Color> generate_chunk(PlanetBiomeGenerator pbg, sf::Vector2u top_left) {
    std::vector<sf::Color> return_chunk(chunk_size.x * chunk_size.y);
    for(int x = 0; x < chunk_size.x; x++) {
        for(int y = 0; y < chunk_size.y; y++) {
            return_chunk[x + y * chunk_size.x] = pbg.get_biome(top_left.x + x, top_left.y + y, 39200, 19600);
        }
    }
    return return_chunk;
}

int main() {
    sf::Texture map_texture;
    map_texture.loadFromFile("map203x.png");
    sf::Sprite map_sprite(map_texture);
    std::vector<sf::Color> active_chunk(chunk_size.x * chunk_size.y);
    sf::Vector2u active_chunk_top_left;
    sf::IntRect chunk_boundary(0, 0, chunk_size.x, chunk_size.y);

    std::vector<std::pair<std::vector<sf::Color>, sf::IntRect>> chunk_vector;

    //setup
    /*int frame = 0;
    int timeContainer = 0;
    sf::Clock clock;*/

    sf::Font font;
    font.loadFromFile("Q5text.ttf");
    sf::Text text;
    text.setFont(font);

    double scale = 48;
    sf::Vector2<double> camera_pos(0.0f, 0.0f);
    sf::Vector2f mouse_pos(0.0f, 0.0f);

    sf::Vector2u window_size(1600, 800);
    sf::RenderWindow window(sf::VideoMode(window_size.x, window_size.y), "Test Name", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(100);

    Entity player("res\\entities\\Player\\player_spritesheet.png");
    PlanetBiomeGenerator pbg;
    
    biome_map_terra.loadFromFile("biome_map_terra.png");
    pbg.set_parameters(0.7707326, 6, 3, 7, 1.5, 4, 4, 3, 1.5, 4, 4, 4, 1.3);

    active_chunk = generate_chunk(pbg, sf::Vector2u(chunk_boundary.left, chunk_boundary.top));

    sf::Sprite tile_sprite;
    sf::Texture white_tile;
    white_tile.loadFromFile("res\\tiles\\white_tile.png");
    tile_sprite.setTexture(white_tile);
    
    //loop
    while(window.isOpen()) {
        //input
        sf::Event event_;
        while(window.pollEvent(event_)) {
            switch(event_.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::MouseMoved:
                    /*if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                        camera_pos -= ((sf::Vector2<double>)sf::Mouse::getPosition(window) - (sf::Vector2<double>)mouse_pos)/scale;
                    }*/
                    mouse_pos = (sf::Vector2f)sf::Mouse::getPosition(window);
                    break;
                case sf::Event::MouseWheelScrolled:
                    int delta = event_.mouseWheelScroll.delta;
                    scale *= pow(1.2, delta);

                    break;
            }
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            player.direction = NORTH;
            player.state = RUNNING;
        } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            player.direction = WEST;
            player.state = RUNNING;
        } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            player.direction = SOUTH;
            player.state = RUNNING;
        } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            player.direction = EAST;
            player.state = RUNNING;
        } else {
            player.state = IDLE;
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            chunk_vector.clear();
            chunk_vector.push_back(std::pair(generate_chunk(pbg, sf::Vector2u(chunk_boundary.left, chunk_boundary.top)), chunk_boundary));
        }

        //rendering
        window.clear(sf::Color(50, 50, 50, 255));

        //if(!check_point_inclusion(chunk_boundary, player.position)) {
        if(!check_within_generated_chunk(chunk_vector, player.position)) {
            sf::Vector2u top_left(player.position.x - fmod(player.position.x, chunk_size.x), player.position.y - fmod(player.position.y, chunk_size.y));
            chunk_boundary.left = top_left.x;
            chunk_boundary.top = top_left.y;
            //active_chunk = generate_chunk(pbg, sf::Vector2u(chunk_boundary.left, chunk_boundary.top));
            chunk_vector.push_back(std::pair(generate_chunk(pbg, sf::Vector2u(chunk_boundary.left, chunk_boundary.top)), chunk_boundary));
        }
                        
        map_sprite.setScale(scale * 16, scale * 16);
        map_sprite.setPosition(-camera_pos.x * scale + window_size.x / 2, -camera_pos.y * scale + window_size.y / 2);

        window.draw(map_sprite);

        for(std::pair<std::vector<sf::Color>, sf::IntRect> p : chunk_vector) {
            if(rect_collision(sf::Rect<double>(p.second), sf::Rect<double>(camera_pos - sf::Vector2<double>(window_size) / (scale * 2), sf::Vector2<double>(window_size) / (scale)))) {
                for(int x = 0; x < chunk_size.x; x++) {
                    for(int y = 0; y < chunk_size.y; y++) {
                        sf::Vector2<double> position((x + p.second.left - camera_pos.x) * scale + double(window.getSize().x) / double(2), (y + p.second.top - camera_pos.y) * scale + double(window.getSize().y) / double(2));
                        tile_sprite.setPosition(position.x, position.y);
                        tile_sprite.setColor(p.first[x + y * chunk_size.x]);
                        tile_sprite.setScale(scale / 16, scale / 16);
                        window.draw(tile_sprite);
                    }
                }
            }
        }

        /*for(int x = 0; x < chunk_size.x; x++) {
            for(int y = 0; y < chunk_size.y; y++) {
                sf::Vector2<double> position((x + chunk_boundary.left - camera_pos.x) * scale + double(window.getSize().x) / double(2), (y + chunk_boundary.top - camera_pos.y) * scale + double(window.getSize().y) / double(2));
                tile_sprite.setPosition(position.x, position.y);
                tile_sprite.setColor(active_chunk[x + y * chunk_size.x]);
                tile_sprite.setScale(scale / 16, scale / 16);
                window.draw(tile_sprite);
            }
        }*/
       

        int scale2 = std::max(pow(4, floor(log10(128.0 / scale) / log10_4)), 1.0);   
        std::vector<sf::Vertex*> vertex_vec;
        for(int i = -ceil(((double(window_size.x) / 2) / scale - camera_pos.x) / scale2) * scale2; i <= floor((double(window_size.x) / 2) / scale + camera_pos.x); i += scale2) {
            sf::Vertex vv[] {
                sf::Vertex(sf::Vector2f((i - camera_pos.x) * scale + window_size.x / 2, 0), (i == 0 || i == 19600) ? sf::Color(0xFF0000FF) : sf::Color(0xBFBFBF20)),
                sf::Vertex(sf::Vector2f((i - camera_pos.x) * scale + window_size.x / 2, window_size.y - 1), (i == 0 || i == 19600) ? sf::Color(0xFF0000FF) : sf::Color(0xBFBFBF20))
            };
            window.draw(vv, 2, sf::Lines);
        }
        for(int i = -ceil(((double(window_size.y) / 2) / scale - camera_pos.y) / scale2) * scale2; i <= floor((double(window_size.y) / 2) / scale + camera_pos.y); i += scale2) {
            sf::Vertex vv[] {
                sf::Vertex(sf::Vector2f(0, (i - camera_pos.y) * scale + window_size.y / 2), (i == 0 || i == 39200) ? sf::Color(0xFF0000FF) : sf::Color(0xBFBFBF20)),
                sf::Vertex(sf::Vector2f(window_size.x - 1, (i - camera_pos.y) * scale + window_size.y / 2), (i == 0 || i == 39200) ? sf::Color(0xFF0000FF) : sf::Color(0xBFBFBF20))
            };
            window.draw(vv, 2, sf::Lines);
        }

        player.tick();

        camera_pos = player.position + sf::Vector2<double>(0, -0.875);

        player.render(camera_pos, scale, &window);

        /*sf::Vertex vv[] {
            sf::Vertex(sf::Vector2f(window_size) * 0.25f, sf::Color(0x00FF003F)),
            sf::Vertex(sf::Vector2f(float(window_size.x) * 0.75f, float(window_size.y) * 0.25f), sf::Color(0x00FF003F)),
            sf::Vertex(sf::Vector2f(window_size) * 0.75f, sf::Color(0x00FF003F)),
            sf::Vertex(sf::Vector2f(float(window_size.x) * 0.25f, float(window_size.y) * 0.75f), sf::Color(0x00FF003F))
        };
        window.draw(vv, 4, sf::PrimitiveType::Quads);*/

        text.setCharacterSize(15);
        text.setString("Coordinates: " + std::to_string(int(player.position.x)) + " " + std::to_string(int(player.position.y)));
        text.setPosition(12, 12);
        window.draw(text);

        window.display();
    }

    return 0;
}