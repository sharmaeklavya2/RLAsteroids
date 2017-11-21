#include <cstdlib>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <string>
#include <ctime>
#include <unistd.h>

typedef std::pair<double, double> dpair;

class Flyer {
public:
    double x, y;
    double vx, vy;
    double radius;

    Flyer(double _x, double _y, double _vx, double _vy, double _radius):
        x(_x), y(_y), vx(_vx), vy(_vy), radius(_radius) {}
    void tick(double width, double height, double t) {
        x += vx * t; y += vy * t;
        if(x >= width) {
            x -= width;
        }
        else if(x < 0) {
            x += width;
        }
        if(y >= height) {
            y -= height;
        }
        else if(y < 0) {
            y += height;
        }
    }

    void fprint(FILE* fp) const {
        std::fprintf(fp, "Flyer(x=%lf, y=%lf, vx=%lf, vy=%lf, radius=%lf)", x, y, vx, vy, radius);
    }
};

static inline double random(double xmin, double xmax) {
    return xmin + (xmax - xmin) * (double(unsigned(std::rand())) / double(RAND_MAX));
}

static inline double square(double x) {
    return x * x;
}

bool is_collision(const Flyer& f1, const Flyer& f2) {
    return square(f1.x - f2.x) + square(f1.y - f2.y) <= square(f1.radius + f2.radius);
}

class Arena {
public:
    int score;
    double width, height, accel;
    Flyer ship;
    std::vector<Flyer> asts;

    Arena(int n, double _width, double _height, double _accel=0.0):
            score(0), width(_width), height(_height), ship(_width/2, _height/2, 0, 0, 0)
        {
        int mindim = std::min(width, height);
        if(_accel == 0) {
            accel = mindim;
        }
        else {
            accel = _accel;
        }
        ship.radius = mindim / 20;
        double vmax = mindim / 4;
        for(int i=0; i<n; ++i) {
            Flyer flyer(random(0, width), random(0, height), random(-vmax, vmax),
                random(-vmax, vmax), random(1, mindim / 3));
            if(is_collision(flyer, ship)) {
                i--; continue;
            }
            else {
                asts.push_back(flyer);
            }
        }
    }

    bool tick(double ax, double ay, double t) {
        // returns whether game is over
        score++;
        ship.tick(width, height, t);
        ship.vx += accel * ax * t;
        ship.vy += accel * ay * t;
        bool collision = false;
        for(Flyer& f: asts) {
            f.tick(width, height, t);
            if(is_collision(f, ship)) {
                collision = true;
            }
        }
        return collision;
    }

    void fprint(FILE* fp) const {
        std::fprintf(fp, "Arena(score=%d, width=%lf, height=%lf, accel=%lf,\nship=", score, width, height, accel);
        ship.fprint(fp); std::fprintf(fp, ",\n[");
        for(const Flyer& f: asts) {
            f.fprint(fp);
            std::fprintf(fp, ",\n");
        }
        std::fprintf(fp, "])\n");
    }

    void render(int w2, int h2) const {
        // fprint(stderr);
        double tw = width / w2;
        double th = height / h2;
        std::vector<std::string> rending(h2, std::string(w2, ' '));
        rending[int(ship.y / th)][int(ship.x / tw)] = '+';
        const Flyer& f = ship;
        int imin = int(f.y - f.radius)/th - 1;
        int imax = int(f.y + f.radius)/th + 1;
        int jmin = int(f.x - f.radius)/tw - 1;
        int jmax = int(f.x + f.radius)/tw + 1;
        for(int i=imin; i<=imax; ++i) {
            for(int j=jmin; j<=jmax; ++j) {
                double y = (i+0.5)*th, x = (j+0.5)*tw;
                if(square(x-f.x) + square(y-f.y) <= square(f.radius)) {
                    rending[(i+h2)%h2][(j+w2)%w2] = '+';
                }
            }
        }
        for(unsigned k=0; k<asts.size(); ++k) {
            const Flyer& f = asts[k];
            int i0 = int(f.y / th), j0 = int(f.x / tw);
            // fprintf(stderr, "i0=%d, j0=%d\n", i0, j0);
            rending[i0][j0] = '0' + k;
            int imin = int(f.y - f.radius)/th - 1;
            int imax = int(f.y + f.radius)/th + 1;
            int jmin = int(f.x - f.radius)/tw - 1;
            int jmax = int(f.x + f.radius)/tw + 1;
            for(int i=imin; i<=imax; ++i) {
                for(int j=jmin; j<=jmax; ++j) {
                    double y = (i+0.5)*th, x = (j+0.5)*tw;
                    if(square(x-f.x) + square(y-f.y) <= square(f.radius)) {
                        rending[(i+h2)%h2][(j+w2)%w2] = '0' + k;
                    }
                }
            }
        }
        std::string hr(w2+1, '=');
        std::printf("%s\n", hr.c_str());
        for(int i=0; i<h2; ++i) {
            std::printf("%s|\n", rending[i].c_str());
        }
        std::printf("%s\n", hr.c_str());
        std::printf("\n");
    }
};

class Player {
public:
    dpair decide(const Arena& arena) const {
        return dpair(0.0, 0.0);
    }
};

using std::printf;

// TODO: Use command-line arguments to specify arena size, number of obstacles and frame rate.

int main() {
    std::srand(std::time(NULL));
    Player player;
    double t = 0.1;
    for(int i=0; i<100; ++i) {
        usleep(1000000);
        printf("%dth episode\n", i);
        Arena arena(5, 800, 300);
        while(true) {
            arena.fprint(stdout);
            arena.render(80, 30);
            usleep(int(t * 1000000));
            dpair action = player.decide(arena);
            bool game_over = arena.tick(action.first, action.second, t);
            if(game_over) break;
            printf("\n");
        }
        arena.render(80, 30);
    }
    return 0;
}
