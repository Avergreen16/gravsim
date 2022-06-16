#define _USE_MATH_DEFINES
#include <SFML-2.5.1\include\SFML\Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <ctime>

#include "worldgen.hpp"

bool point_sort_y(sf::Vector2<double> p1, sf::Vector2<double> p2) {
    if(p1.y < p2.y) return true;
    if(p1.y == p2.y) return (p1.x < p2.x);
    return false;
}

bool point_sort_x(sf::Vector2<double> p1, sf::Vector2<double> p2) {
    if(p1.x < p2.x) return true;
    if(p1.x == p2.x) return (p1.y < p2.y);
    return false;
}

double get_value_y(double slope, sf::Vector2<double> p, double y) {
    return (y - p.y) / slope + p.x;
}

double get_value_x(double slope, sf::Vector2<double> p, double x) {
    return slope * (x - p.x) + p.y;
}

double get_slope(sf::Vector2<double> p1, sf::Vector2<double> p2) {
    return (p2.y - p1.y) / (p2.x - p1.x);
}

template <typename T>
int sign(T input) {
    if(input > 0) return 1;
    if(input < 0) return -1;
    return 0;
}

void rasterize_line(sf::Vector2<double> p1, sf::Vector2<double> p2, sf::Image* img, sf::Color color) {
    double slope = get_slope(p1, p2);
    if(slope / sign(slope) <= 1) {
        for(int x = round(p1.x); x != round(p2.x); x += sign(p2.x - p1.x)) {
            img->setPixel(x, round(get_value_x(slope, p1, x)), color);
        }
    } else {
        for(int y = round(p1.y); y != round(p2.y); y += sign(p2.y - p1.y)) {
            img->setPixel(round(get_value_y(slope, p1, y)), y, color);
        }
    }
}

struct Triangle {
    std::vector<sf::Vector2<double>> points;
    bool is_on_edge = false;

    Triangle(sf::Vector2<double> p1, sf::Vector2<double> p2, sf::Vector2<double> p3, bool p_is_on_edge) {
        points.push_back(p1);
        points.push_back(p2);
        points.push_back(p3);

        is_on_edge = p_is_on_edge;
    }

    void rasterize(sf::Image* img, sf::Color color, sf::Vector2u img_size);
};

void Triangle::rasterize(sf::Image* img, sf::Color color, sf::Vector2u img_size) {
    if(is_on_edge) {
        sort(points.begin(), points.end(), point_sort_y);
        double slope1 = get_slope(points[0], points[1]);
        double slope2 = get_slope(points[0], points[2]);
        double slope3 = get_slope(points[2], points[1]);

        int x1, y1;
        int y = round(points[0].y);
        if(1.0 / slope1 < 1.0 / slope2) {
            for(; y < points[1].y; y++) {
                for(int x = round(get_value_y(slope1, points[1], y)); x < round(get_value_y(slope2, points[2], y)); x++) {
                    if(x < 0) {
                        x1 = x + img_size.x;
                    } else if(x >= img_size.x) {
                        x1 = x - img_size.x;
                    } else {
                        x1 = x;
                    }

                    if(y < 0) {
                        y1 = y + img_size.y;
                    } else if(y >= img_size.y) {
                        y1 = y - img_size.y;
                    } else {
                        y1 = y;
                    }

                    img->setPixel(x1, y1, color);
                }
            }
            for(; y < points[2].y; y++) {
                for(int x = round(get_value_y(slope3, points[1], y)); x < round(get_value_y(slope2, points[2], y)); x++) {
                    if(x < 0) {
                        x1 = x + img_size.x;
                    } else if(x >= img_size.x) {
                        x1 = x - img_size.x;
                    } else {
                        x1 = x;
                    }

                    if(y < 0) {
                        y1 = y + img_size.y;
                    } else if(y >= img_size.y) {
                        y1 = y - img_size.y;
                    } else {
                        y1 = y;
                    }

                    img->setPixel(x1, y1, color);
                }
            }
        } else {
            for(; y < points[1].y; y++) {
                for(int x = round(get_value_y(slope2, points[2], y)); x < round(get_value_y(slope1, points[1], y)); x++) {
                    if(x < 0) {
                        x1 = x + img_size.x;
                    } else if(x >= img_size.x) {
                        x1 = x - img_size.x;
                    } else {
                        x1 = x;
                    }

                    if(y < 0) {
                        y1 = y + img_size.y;
                    } else if(y >= img_size.y) {
                        y1 = y - img_size.y;
                    } else {
                        y1 = y;
                    }

                    img->setPixel(x1, y1, color);
                }
            }
            for(; y < points[2].y; y++) {
                for(int x = round(get_value_y(slope2, points[2], y)); x < round(get_value_y(slope3, points[1], y)); x++) {
                    if(x < 0) {
                        x1 = x + img_size.x;
                    } else if(x >= img_size.x) {
                        x1 = x - img_size.x;
                    } else {
                        x1 = x;
                    }

                    if(y < 0) {
                        y1 = y + img_size.y;
                    } else if(y >= img_size.y) {
                        y1 = y - img_size.y;
                    } else {
                        y1 = y;
                    }

                    img->setPixel(x1, y1, color);
                }
            }
        }
    } else {
        sort(points.begin(), points.end(), point_sort_y);
        double slope1 = get_slope(points[0], points[1]);
        double slope2 = get_slope(points[0], points[2]);
        double slope3 = get_slope(points[2], points[1]);

        int y = round(points[0].y);
        if(1 / slope1 < 1 / slope2) {
            for(; y < points[1].y; y++) {
                for(int x = round(get_value_y(slope1, points[1], y)); x < round(get_value_y(slope2, points[2], y)); x++) {
                    if(x >= 0 && x < img_size.x && y >= 0 && y < img_size.y) img->setPixel(x, y, color);
                }
            }
            for(; y < points[2].y; y++) {
                for(int x = round(get_value_y(slope3, points[1], y)); x < round(get_value_y(slope2, points[2], y)); x++) {
                    if(x >= 0 && x < img_size.x && y >= 0 && y < img_size.y) img->setPixel(x, y, color);
                }
            }
        } else {
            for(; y < points[1].y; y++) {
                for(int x = round(get_value_y(slope2, points[2], y)); x < round(get_value_y(slope1, points[1], y)); x++) {
                    if(x >= 0 && x < img_size.x && y >= 0 && y < img_size.y) img->setPixel(x, y, color);
                }
            }
            for(; y < points[2].y; y++) {
                for(int x = round(get_value_y(slope2, points[2], y)); x < round(get_value_y(slope3, points[1], y)); x++) {
                    if(x >= 0 && x < img_size.x && y >= 0 && y < img_size.y) img->setPixel(x, y, color);
                }
            }
        }
    }   
}

int main() {
    sf::Clock clock;

    PlanetBiomeGenerator pgen;
    
    biome_map_terra.loadFromFile("biome_map_terra.png");

    RNG rng;
    rng.set_seed(0.87468, 45.02578);

    for(int t = 0; t < 1; t++) {
        float seed = rng.retrieve();
        //0.7707326
        pgen.set_parameters(0.3202376, 6, 3, 7, 1.5, 4, 4, 3, 1.5, 4, 4, 4, 1.3);
        //pgen.set_parameters(0.6053257, 6, 3, 7, 1.5, 4, 4, 3, 1.5, 4, 4, 4, 1.3);

        //sf::Vector2u hexagons(round(100 * 2 / sqrt(3)), 50);//100 * 0.375);
        //sf::Vector2u triangles(hexagons.x / 2 * 3, hexagons.y * 2);

        sf::Vector2u img_size(15408 / 8, 15408 / 16);
        //sf::Vector2u img_size(800, 400);

        sf::Image img;

        img.create(img_size.x, img_size.y);

        //
        //
        
        for(int x = 0; x < img_size.x; x++) {
            for(int y = 0; y < img_size.y; y++) {
                /*if(x < 200 && y < 200) {
                    float color = (noise.GetNoise(float(x), float(y)) + 1) / 2;
                    img.setPixel(x, y, sf::Color(color * 255, color * 255, color * 255));
                }*/

                img.setPixel(x, y, sf::Color(pgen.get_biome(x, y, (img_size.x), (img_size.y))));
            }
        }

        /*std::vector<sf::Vector2<double>> points;
        for(int j = 0; j < triangles.y; j++) {
            for(int i = 0; i < triangles.x; i++) {
                if((j % 2 == 1 && i % 3 == 1) || (j % 2 == 0 && i % 3 == 2)) {
                    points.push_back(sf::Vector2<double>((i - 0.25 + 0.5 * (j % 2 == 0)) * (double(img_size.x) / triangles.x), j * (double(img_size.y) / triangles.y)));
                } else {
                    points.push_back(sf::Vector2<double>((i + (0.1 + 0.8 * rng.retrieve()) - 0.25 + 0.5 * (j % 2 == 0)) * (double(img_size.x) / triangles.x), (j + (0.1 + 0.8 * rng.retrieve())) * (double(img_size.y) / triangles.y)));
                }
            }
        }

        std::vector<std::pair<Triangle, sf::Color>> tri_vec;
        
        int x = triangles.x;
        int y = triangles.y;

        sf::Vector2<double> flip_x(img_size.x, 0);
        sf::Vector2<double> flip_y(0, img_size.y);

        for(int i = 0; i < triangles.x * triangles.y; i++) {
            double num = rng.retrieve();
            double col = pgen.pnf1.retrieve(points[i].x, points[i].y, img_size.x, img_size.y);
            sf::Color color = pgen.get_biome(points[i].x, points[i].y, img_size.x, img_size.y);
            
            if(i % (triangles.x * 2) >= triangles.x && i % 3 == 1) {
                if(i >= triangles.x * (triangles.y - 1)) {
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x - 1], points[i - x], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x], points[i + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1], points[i % x] + flip_y, true), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i % x] + flip_y, points[i % x - 1] + flip_y, true), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i % x - 1] + flip_y, points[i - 1], true), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[i - x - 1], false), color));
                } else {
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x - 1], points[i - x], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x], points[i + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1], points[i + x], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i + x - 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x - 1], points[i - 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[i - x - 1], false), color));
                }
                
            } else if(i % (triangles.x * 2) < triangles.x && i % 3 == 2) {
                if(i >= triangles.x * (triangles.y - 1)) {

                } else if(i % triangles.x == triangles.x - 1) {
                    if(!(i == triangles.x - 1)) {*/
                        /*if(points[i].x >= img_size.x) {
                            color = pgen.get_biome(points[i].x - img_size.x, points[i].y, img_size.x, img_size.y);

                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i - x], points[i - (2 * x) + 1] - flip_x, true), color));
                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i - (2 * x) + 1] - flip_x, points[i - x + 1] - flip_x, true), color));
                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i - x + 1] - flip_x, points[i + 1] - flip_x, true), color));
                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i + 1] - flip_x, points[i + x], true), color));
                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i + x], points[i - 1], true), color));
                            tri_vec.push_back(std::pair(Triangle(points[i] - flip_x, points[i - 1], points[i - x], true), color));
                        } else {
                            color = pgen.get_biome(points[i].x, points[i].y, img_size.x, img_size.y);*/

                        /*tri_vec.push_back(std::pair(Triangle(points[i], points[i - x], points[i - (2 * x) + 1] + flip_x, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i - (2 * x) + 1] + flip_x, points[i - x + 1] + flip_x, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i - x + 1] + flip_x, points[i + 1] + flip_x, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1] + flip_x, points[i + x], true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i - 1], false), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[i - x], false), color));
                        //}
                    } else {
                        tri_vec.push_back(std::pair(Triangle(points[i], points[x * y - 1] - flip_y, points[x * (y - 1)] + flip_x - flip_y, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[x * (y - 1)] + flip_x - flip_y, points[0] + flip_x, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[0] + flip_x, points[x] + flip_x, true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[x] + flip_x, points[i + x], true), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i - 1], false), color));
                        tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[x * y - 1] - flip_y, true), color));
                    }
                } else if(i < triangles.x) {
                    tri_vec.push_back(std::pair(Triangle(points[i], points[x * y - (x - i)] - flip_y, points[x * y - (x - i) + 1] - flip_y, true), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[x * y - (x - i) + 1] - flip_y, points[i + 1], true), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1], points[i + x + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x + 1], points[i + x], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i - 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[x * y - (x - i)] - flip_y, true), color));
                } else {
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x], points[i - x + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - x + 1], points[i + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + 1], points[i + x + 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x + 1], points[i + x], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i + x], points[i - 1], false), color));
                    tri_vec.push_back(std::pair(Triangle(points[i], points[i - 1], points[i - x], false), color));
                }
                
            }
        }

        for(std::pair<Triangle, sf::Color> pair : tri_vec) {*/
            /*if(pair.first.points[0].x < img_size.x && pair.first.points[1].x < img_size.x, pair.first.points[2].x < img_size.x)*/ 
            /*pair.first.rasterize(&img, pair.second, img_size);
        }

        std::vector<int> adjacent_indices;*/

        /*for(int i = 0; i < 20; i++) {
            int index = floor(rng.retrieve() * triangles.x * triangles.y);
            while(!((index % (triangles.x * 2) < triangles.x && (index % 3 == 0 || index % 3 == 1)) || (index % (triangles.x * 2) >= triangles.x && (index % 3 == 0 || index % 3 == 2))) || !(pgen.pnf1.retrieve(points[index].x, points[index].y, img_size.x, img_size.y) > 4.0/7)) {
                index = floor(rng.retrieve() * triangles.x * triangles.y);
            }

            int former_index = -1;
            adjacent_indices.clear();

            for(int j = 0; j < 100; j++) {
                if(index % (triangles.x * 2) < triangles.x) {
                    if(index % 3 == 0) {
                        adjacent_indices.push_back(index + triangles.x);
                        adjacent_indices.push_back(index + 1);
                        if(index < triangles.x) {
                            //adjacent_indices.push_back((triangles.x - 1) * triangles.y + index);
                        } else {
                            adjacent_indices.push_back(index - triangles.x);
                        }
                    } else if(index % 3 == 1) {
                        adjacent_indices.push_back(index + triangles.x + 1);
                        adjacent_indices.push_back(index - 1);
                        if(index < triangles.x) {
                            //adjacent_indices.push_back((triangles.x - 1) * triangles.y + index + 1);
                        } else {
                            adjacent_indices.push_back(index - triangles.x + 1);
                        }
                    }
                } else {
                    if(index % 3 == 0) {
                        adjacent_indices.push_back(index - triangles.x);

                        if(index >= triangles.x * (triangles.y - 1)) {
                            adjacent_indices.push_back(index % triangles.x);
                        } else {
                            adjacent_indices.push_back(index + triangles.x);
                        }

                        if(index % triangles.x == 0) {
                            adjacent_indices.push_back(index + triangles.x - 1);
                        } else {
                            adjacent_indices.push_back(index - 1);
                        }
                    } else if(index % 3 == 2) {
                        adjacent_indices.push_back(index - triangles.x - 1);

                        if(index >= triangles.x * (triangles.y - 1)) {
                            adjacent_indices.push_back(index % triangles.x - 1);
                        } else {
                            adjacent_indices.push_back(index + triangles.x - 1);
                        }

                        if(index % triangles.x == triangles.x - 1) {
                            adjacent_indices.push_back(index - triangles.x + 1);
                        } else {
                            adjacent_indices.push_back(index + 1);
                        }
                    }
                }

                if(former_index != -1) std::remove(adjacent_indices.begin(), adjacent_indices.end(), former_index);

                double lowest_height = 1;
                int new_index = -1;
                double current_height;
                for(int adjacent_index : adjacent_indices) {
                    current_height = pgen.pnf1.retrieve(points[adjacent_index].x, points[adjacent_index].y, img_size.x, img_size.y);
                    
                    if(current_height < lowest_height) {
                        lowest_height = current_height;
                        new_index = adjacent_index;
                    }
                }

                double active_height = pgen.pnf1.retrieve(points[index].x, points[index].y, img_size.x, img_size.y);

                if(active_height < lowest_height) break;
                if(lowest_height < 5.0/9) break;
                rasterize_line(points[new_index], points[index], &img, pgen.biome_lookup_table.at(19));

                //for(int adj_index : adjacent_indices) {
                //    rasterize_line(points[adj_index], points[index], &img, sf::Color(255, 0, 0));
                //}
                //rasterize_line(points[adjacent_indices[1]], points[index], &img, sf::Color(255, 0, 0));
                //rasterize_line(points[adjacent_indices[2]], points[index], &img, sf::Color(255, 0, 0));
                former_index = index;
                index = new_index;
            }
        }*/

        //rasterize_line(sf::Vector2<double>(100, 100), sf::Vector2<double>(200, 400), &img, pgen.biome_lookup_table.at(19));

        /*for(int i = 0; i < 1000; i++) {
            int x = rng.retrieve() * img_size.x;
            int y = rng.retrieve() * img_size.y;
            float height = pgen.pnf1.retrieve(x, y, img_size.x, img_size.y);
            bool continue_ = true;

            for(int j = 0; j < 1000; j++) {
                if(height > (4/7) && continue_) {
                    img.setPixel(x, y, pgen.biome_lookup_table.at(19));
                    if(x <= 1 || x >= img_size.x - 1 || y <= 1 || y >= img_size.y - 1) {
                        continue_ = false;
                    } else {
                        float height_n = pgen.pnf1.retrieve(x, y - 1, img_size.x, img_size.y);
                        float height_s = pgen.pnf1.retrieve(x, y + 1, img_size.x, img_size.y);
                        float height_e = pgen.pnf1.retrieve(x + 1, y, img_size.x, img_size.y);
                        float height_w = pgen.pnf1.retrieve(x - 1, y, img_size.x, img_size.y);

                        if(height_n >= height_s && height_n >= height_e && height_n >= height_w) {
                            y -= 1;
                            height = height_n;
                        } else if(height_s >= height_n && height_s >= height_e && height_s >= height_w) {
                            y += 1;
                            height = height_s;
                        } else if(height_e >= height_n && height_e >= height_s && height_e >= height_w) {
                            x += 1;
                            height = height_e;
                        } else if(height_w >= height_n && height_w >= height_s && height_w >= height_e) {
                            x -= 1;
                            height = height_w;
                        } else {
                            continue_ = false;
                        }
                    }
                }
            }
        }*/
        
        img.saveToFile("map" + std::to_string(t + 206) + "x.png");
        std::cout << std::to_string(t) + " " + std::to_string(seed) << "\n";
    }
    
    printf("%f", clock.getElapsedTime().asSeconds());
}