#define _USE_MATH_DEFINES
#include <SFML-2.5.1\include\SFML\Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <ctime>
#include <string>

struct RNG {
    double factor;
    double product;

    void set_seed(double p_seed, double p_factor) {
        factor = p_factor;
        product = p_seed;
    }

    double retrieve() {
        product = product * factor;
        product -= floor(product);
        return product;
    }
};

float get_length(sf::Vector2f input) {
    return std::sqrt(input.x * input.x + input.y * input.y);
}

float get_slope(sf::Vector2f p1, sf::Vector2f p2) {
    float delta_x = (p1.x - p2.x);
    if(delta_x == 0) return NAN;
    return (p1.y - p2.y) / delta_x;
}

sf::Vector2f get_midpoint(sf::Vector2f p1, sf::Vector2f p2) {
    return sf::Vector2f((p1.x + p2.x) / 2, (p1.y + p2.y) / 2);
}

float get_value(sf::Vector2f point, float slope, float x) {
    return slope * (x - point.x) + point.y;
}

sf::Vector2f get_intersection(sf::Vector2f p1, float s1, sf::Vector2f p2, float s2) {
    if(s1 == s2) return sf::Vector2f(NAN, NAN);
    if(isnanf(s1)) {
        if(isnanf(s2)) return sf::Vector2f(NAN, NAN);
        return sf::Vector2f(p1.x, get_value(p2, s2, p1.x));
    }
    if(isnanf(s2)) return sf::Vector2f(p2.x, get_value(p1, s1, p2.x));
    float x = (s1 * p1.x - p1.y - s2 * p2.x + p2.y) / (s1 - s2);
    return sf::Vector2f(x, get_value(p1, s1, x));
}

sf::Vector2f p_invert(sf::Vector2f point) {
    return sf::Vector2f(point.y, point.x);
}

sf::Vector2f create_angle_unit_vector(float angle) {
    double angle_in_radians = (double)angle * M_PI / 180;
    return sf::Vector2f(std::cos(angle_in_radians) , std::sin(angle_in_radians));
}

float get_perpendicular_slope(float slope) {
    if(isnanf(slope)) return 0;
    if(slope == 0) return NAN;
    return -1 / slope;
}

std::pair<sf::Vector2f*, sf::Vector2f*> create_edge(sf::Vector2f* p1, sf::Vector2f* p2) {
    if(p1 > p2) {
        return std::pair(p1, p2);
    }
    return std::pair(p2, p1);
}

float get_flipped_slope(float slope) {
    if(isnanf(slope)) return 0;
    if(slope == 0) return NAN;
    return 1 / slope;
}

void draw_line(sf::Vector2f* p1, sf::Vector2f* p2, sf::Image* img, sf::Color color) {
    float slope = get_slope(*p1, *p2);
    if(isnanf(slope)) {
        for(int y = round(p1->y); y != round(p2->y); y += (p2->y - p1->y) / fabs(p2->y - p1->y)) {
            img->setPixel(round(p1->x), y, color);
        }
    } else if(fabs(slope) <= 1) {
        for(int x = round(p1->x); x != round(p2->x); x += (p2->x - p1->x) / fabs(p2->x - p1->x)) {
            int y = round(get_value(*p1, slope, (float)x));
            img->setPixel(x, y, color);
        }
    } else {
        float flipped_slope = get_flipped_slope(slope);
        for(int y = round(p1->y); y != round(p2->y); y += (p2->y - p1->y) / fabs(p2->y - p1->y)) {
            sf::Vector2f inverted_p1(p1->y, p1->x);
            int x = round(get_value(inverted_p1, flipped_slope, (float)y));
            img->setPixel(x, y, color);
        }
    }
}

class Dtriangle {
    public:

    sf::Vector2f c_center;
    float c_radius;
    
    sf::Vector2f perp_vector;
    std::pair<int, int> inclusion_data;
    int infpoints;
    
    int sign(float x) {
        return (x > 0) - (x < 0);
    }

    float dot_product(sf::Vector2f v1, sf::Vector2f v2) {
        return v1.x * v2.x + v1.y * v2.y;
    }
    
    std::vector<sf::Vector2f*> points;

    Dtriangle() {};

    Dtriangle(sf::Vector2f* p1, sf::Vector2f* p2, sf::Vector2f* p3, int p_infpoints) {
        points = {p1, p2, p3};
        infpoints = p_infpoints;
        if(infpoints == 0) {
            float slope1 = get_perpendicular_slope(get_slope(*p1, *p2));
            sf::Vector2f midpt1 = get_midpoint(*p1, *p2);
            float slope2 = get_perpendicular_slope(get_slope(*p2, *p3));
            sf::Vector2f midpt2 = get_midpoint(*p2, *p3);
            c_center = get_intersection(midpt1, slope1, midpt2, slope2);
            c_radius = get_length(c_center - *p2);
        } else if(infpoints == 1) {
            float perp_slope = get_perpendicular_slope(get_slope(*p2, *p3));
            if(isnanf(perp_slope)) {
                perp_vector = sf::Vector2f(0, 1);
            } else perp_vector = sf::Vector2f(1, perp_slope);
        } else if(infpoints == 2) {
            sf::Vector2f inf_sign1(sign(p1->x), sign(p1->y));
            sf::Vector2f inf_sign2(sign(p2->x), sign(p2->y));
            float perp_slope = get_perpendicular_slope(get_slope(inf_sign1, inf_sign2));
            if(isnanf(perp_slope)) {
                perp_vector = sf::Vector2f(0, 1);
            } else perp_vector = sf::Vector2f(1, perp_slope);
        }
    }

    bool circumcircle_inclusion(sf::Vector2f point) {
        if(infpoints == 0) {
            sf::Vector2f dist = c_center - point;
            return dist.x * dist.x + dist.y * dist.y < c_radius * c_radius;
        } else if(infpoints == 1 || infpoints == 2) {
            sf::Vector2f inf_sign(sign(points[0]->x), sign(points[0]->y));
            return sign(dot_product(point - *points[2], perp_vector)) == sign(dot_product(inf_sign, perp_vector));
        } else if(infpoints == 3) {
            return true;
        }
    }
};

std::vector<Dtriangle> delaunay_triangulation(std::vector<sf::Vector2f*> center_points, sf::Vector2i bounding_box_size) {
    std::vector<Dtriangle> good_triangles;
    std::vector<Dtriangle> bad_triangles;
    std::vector<std::pair<sf::Vector2f*, sf::Vector2f*>> edge_collector;
    std::vector<std::pair<sf::Vector2f*, sf::Vector2f*>> polygon;

    std::vector<sf::Vector2f> exterior_points = {
        sf::Vector2f(-INFINITY, -INFINITY),
        sf::Vector2f(INFINITY, 0.0f),
        sf::Vector2f(0.0f, INFINITY)
    };

    good_triangles.push_back(Dtriangle(&exterior_points[0], &exterior_points[1], &exterior_points[2], 3));

    for(sf::Vector2f* point : center_points) {
        for(int j = 0; j < good_triangles.size(); j++) {
            Dtriangle tri = good_triangles[j];
            if(tri.circumcircle_inclusion(*point)) {
                bad_triangles.push_back(tri);
                good_triangles.erase(good_triangles.begin() + j);
                j--;
            }
        }

        for(Dtriangle tri : bad_triangles) {
            edge_collector.push_back(create_edge(tri.points[0], tri.points[1]));
            edge_collector.push_back(create_edge(tri.points[1], tri.points[2]));
            edge_collector.push_back(create_edge(tri.points[2], tri.points[0]));
        }

        for(int i = 0; i < edge_collector.size(); i++) {
            bool unique = true;
            std::pair<sf::Vector2f*, sf::Vector2f*> edge1 = edge_collector[i];
            for(int j = i + 1; j < edge_collector.size(); j++) {
                    std::pair<sf::Vector2f*, sf::Vector2f*> edge2 = edge_collector[j];
                    if(edge1.first == edge2.first && edge1.second == edge2.second) {
                    unique = false;
                    edge_collector.erase(edge_collector.begin() + j);
                    j--;
                }
            }
            if(unique) polygon.push_back(edge1);
            else {
                edge_collector.erase(edge_collector.begin() + i);
                i--;
            }
        }

        for(std::pair<sf::Vector2f*, sf::Vector2f*> edge : polygon) {
            int infpoints = 0;
            bool flip = false;
            if(edge.first == &exterior_points[0] || edge.first == &exterior_points[1] || edge.first == &exterior_points[2]) infpoints++;
            if(edge.second == &exterior_points[0] || edge.second == &exterior_points[1] || edge.second == &exterior_points[2]) {
                if(infpoints == 0) flip = true;
                infpoints++;
            }

            Dtriangle tri;
            flip ? tri = Dtriangle(edge.second, edge.first, point, infpoints) : tri = Dtriangle(edge.first, edge.second, point, infpoints);
            
            good_triangles.push_back(tri);
        }

        bad_triangles.clear();
        edge_collector.clear();
        polygon.clear();
    }

    for(int i = 0; i < good_triangles.size(); i++) {
        Dtriangle tri = good_triangles[i];
        for(sf::Vector2f* point : tri.points) {
            if(point == &exterior_points[0] || point == &exterior_points[1] || point == &exterior_points[2]) {
                good_triangles.erase(good_triangles.begin() + i);
                i--;
                break;
            }
        }
    }

    return good_triangles;
}

bool compare_point(sf::Vector2f* d1, sf::Vector2f* d2) {
    if(d1->x + d1->y < d2->x + d2->y) return true; 
    else if(d1->x + d1->y == d2->x + d2->y && d1->y < d2->y) return true;
    return false;
}

int main() {
    time_t starting_time = std::time(NULL);

    sf::Vector2i img_size(500, 500);
    sf::Vector2i point_wh(20, 20);

    std::vector<sf::Vector2f> input_points;

    RNG rng;
    rng.set_seed(0.6541764, 71.923749);

    for(int i = 0; i < point_wh.x; i++) {
        for(int j = 0; j < point_wh.y; j++) {
            float x = (i + rng.retrieve()) * ((float)img_size.x / point_wh.x);
            float y = (j + rng.retrieve()) * ((float)img_size.y / point_wh.y);
            if(x >= 0 && x < img_size.x - 1 && y >= 0 && y < img_size.y - 1) input_points.push_back(sf::Vector2f(floor(x), floor(y)));
        }
    }

    std::vector<sf::Vector2f*> pointers;
    for(int i = 0; i < input_points.size(); i++) {
        pointers.push_back(&input_points[i]);
    }

    std::sort(pointers.begin(), pointers.end(), compare_point);

    std::vector<Dtriangle> good_triangles = delaunay_triangulation(pointers, img_size);

    sf::Image img;
    img.create(img_size.x, img_size.y);
    sf::Color color(0x0000FFFF);

    for(Dtriangle tri : good_triangles) {
        draw_line(tri.points[0], tri.points[1], &img, color);
        draw_line(tri.points[1], tri.points[2], &img, color);
        draw_line(tri.points[2], tri.points[0], &img, color);
    }

    for(sf::Vector2f point : input_points) {
        img.setPixel(point.x, point.y, sf::Color(0xFF0000FF));
    }
    
    img.saveToFile("file_triangulation.png");
    
    printf("triangle count %i\n", good_triangles.size());

    time_t ending_time = std::time(NULL);
    std::cout << std::to_string(ending_time - starting_time) << std::endl;
}