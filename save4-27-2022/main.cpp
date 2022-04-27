#include <SFML-2.5.1\include\SFML\Graphics.hpp>
#include <iostream>
#include <cmath>
#include <ctime>

const std::string vertex_shader = R"""(
    
    )""";

const std::string fragment_shader = R"""(#version 330 core
#ifdef GL_ES
precision mediump float;
#endif
 
uniform vec2 u_resolution;
uniform vec2 u_shapeSize;
uniform vec2 u_shapePos;
uniform float u_time;
uniform sampler2D u_texture;
uniform int u_textureWidth;

float asin_approx(float x) {
    return x + 0.5 * (pow(x, 3) / 3) + 0.375 * (pow(x, 5) / 5) + 0.3125 * (pow(x, 7) / 7) + 0.2734375 * (pow(x, 9) / 9);
}

void main() {
    float pi = 3.14159265358979324;
    vec2 relativeCoord = vec2(gl_FragCoord.x - u_shapePos.x, gl_FragCoord.y * -1 + u_resolution.y - u_shapePos.y);
    vec2 st = relativeCoord/u_shapeSize;

    float py = st.y * 2 - 1;
    float width_at_height = sqrt(1 - py * py);
    float py2 = asin(py) * 2/pi;
    float px = st.x * 2 - 1;
    if(px * px + py * py > 1) {
        discard;
    }
    float px2 = asin(px / width_at_height) * 2/pi;

    float xCoord = (px2 + 1) / 4 + u_time / 10000;
    float light_xCoord = (u_time / 20000 - floor(u_time / 20000)) * 4 - 2;
    float shadow1_xCoord = light_xCoord - 1;
    float shadow2_xCoord = light_xCoord + 1;
    

    vec4 color = texture(u_texture, vec2(xCoord - floor(xCoord), (py2 + 1) / 2));
    if(px2 > shadow1_xCoord && px2 < shadow2_xCoord) {
        gl_FragColor = vec4(color.xyz / 4, 1.0);
    } else {
        gl_FragColor = vec4(color.xyz, 1.0);
    }
})""";

int main() 
{   
    int frame = 0;
    sf::Clock clock;
    unsigned int window_width = 1080, window_height = 720;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Test Name", sf::Style::Close | sf::Style::Titlebar | sf::Style::Resize);
    //window.setFramerateLimit(100);
    sf::Texture mapTexture;
    mapTexture.loadFromFile("file3v.png");
    sf::Sprite sprite;
    sf::Vector2u textureSize = mapTexture.getSize();

    sf::RectangleShape shape(sf::Vector2f(500.0f, 500.0f));
    //sf::RectangleShape shape2(sf::Vector2f(200.0f, 200.0f));
    sf::Shader shader;
    shader.loadFromMemory(fragment_shader, sf::Shader::Fragment);
    sf::Vector2f mousePos(0.0f, 0.0f);

    sprite.setTexture(mapTexture);
    sprite.setTextureRect(sf::IntRect(300, 0, textureSize.x / 2, textureSize.y));
    sprite.setPosition(100.0f, 100.0f);

    float scale = 1;
    sf::Vector2f cameraPos(0.0f, 0.0f);
    int timeContainer = 0;
    
    while(window.isOpen()) 
    {
        sf::Event event_;
        while(window.pollEvent(event_))
        {
            switch(event_.type) 
            {
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
                        cameraPos -= ((sf::Vector2f)sf::Mouse::getPosition(window) - mousePos)/scale;
                    }
                    mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
                    break;
                case sf::Event::MouseWheelScrolled:
                    int delta = event_.mouseWheelScroll.delta;
                    scale *= pow(1.2, delta);
                    shape.setSize(sf::Vector2f(500 * scale, 500 * scale));
                    //shape2.setSize(sf::Vector2f(200 * scale, 200 * scale));
                    break;
            }
        }

        shape.setPosition((-cameraPos) * scale + ((sf::Vector2f)window.getSize() - shape.getSize()) / 2.0f);
        //shape2.setPosition((-cameraPos + sf::Vector2f(1700.0f, 260.0f)) * scale + ((sf::Vector2f)window.getSize() - shape2.getSize()) / 2.0f);
        
        shader.setUniform("u_texture", mapTexture);
        shader.setUniform("u_resolution", (sf::Glsl::Vec2)window.getSize());
        shader.setUniform("u_shapeSize", (sf::Glsl::Vec2)shape.getSize());
        shader.setUniform("u_shapePos", (sf::Glsl::Vec2)shape.getPosition());
        //shader.setUniform("u_mouse", (sf::Glsl::Vec2)mousePos);
        shader.setUniform("u_time", (float)clock.getElapsedTime().asMilliseconds()); 
        shader.setUniform("u_textureWidth", (float)mapTexture.getSize().x);

        window.clear();
        //window.draw(sprite);
        window.draw(shape, &shader);
        window.display();
        frame++;
        if(frame % 100 == 0 && frame != 0) {
            std::cout << (double)frame / (clock.getElapsedTime().asMilliseconds() - timeContainer) * 1000 << std::endl;
            timeContainer = clock.getElapsedTime().asMilliseconds();
            frame = 0;
        }
    }

    

    return 0;
}