#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>

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

    cones.close();
    return 0;
}

/*
Unable to finish on time. Here are my current considered approaches

-   use potential field with track points as repulsors
    mark near-stationary trajectories as centreline
    https://upload.wikimedia.org/wikipedia/commons/3/3d/Cyberbotics%27_Robot_Curriculum.pdf

-   as per hints.txt: use delauney triangulation to generate voronoi diagram
    use midpoint of edges as markers for the centreline
    use cubic spline interpolation between marked points to generate smooth
    centreline
    https://www.cis.upenn.edu/~cis6100/convex8.pdf
    https://cgi.cse.unsw.edu.au/~blair/pub/iros03.pdf

*/