#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>

class Cone {
private:
    double x_;
    double y_;
public:
    Cone(double x, double y): x_{x}, y_{y} {}

    /**
     * @brief Length from origin
     */
    double len() const {return std::hypot(x_, y_);}

    double dist(Cone& ref) const {return std::hypot(x_ - ref.x(), y_ - ref.y());}

    double x() const {return x_;}
    double y() const {return y_;}



};


typedef struct Point {
    double x;
    double y;
} Point;

double dist(const Point& a, const Point& b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

double len(const Point& p) {return std::hypot(p.x, p.y);}


// Please also try document your thought process and your code as you go!
int main(void) {
    std::ifstream cones{"track.csv"};
    Point point{1,1};
    return 0;
}