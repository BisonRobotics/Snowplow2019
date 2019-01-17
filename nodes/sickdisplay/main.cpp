#include <iostream>
#include <vector>
#include <utility>
#include <SDL/SDL.h> // 2D graphics library for displaying data

#include <CPJL.hpp>              // CPJL... duh
#include <cpp/SickMeasurement.h> // CPJL message type for SICK data
#include <SICK_Sensor.h>         // for accessing the SICK scanner
#include <misc.h>                // mapFloat and friends

using namespace std;

#define WINDOW_SZ 650

SickMeasurement* sick_msg = NULL;

// needs to be accessed by callback
SDL_Surface* window = NULL;
vector<pair<float, float>> cartesian_transform;
const int visible_radius = 3000.0f; // millimeters (raw from SICK)
int plot_color;
bool display_window_open = false;

// draw some reference lines onscreen
void draw_reference_lines(SDL_Surface* win, int color);
int reference_color;
int coord_color;

long int previous_timestamp = 0;

int main(int argc, char* argv[]) {

    if(argc != 2) {
        cout << "Usage: " << argv[0] << " <CPJL host location>\n";
        exit(EXIT_FAILURE);
    }

    sick_msg = new SickMeasurement(new CPJL(argv[1], 14000), "sick_data", 
        []() {
            double dt = sick_msg->timestamp - previous_timestamp;
            dt /= 1000000.0;
            cout << "dt = " << dt << " seconds. throughput = " << 1.0/dt << " msgs / second\n";
            previous_timestamp = sick_msg->timestamp;

            // updates the UI
            if(!display_window_open)
                return; // no point in continuing if nothing to do

            cartesian_transform.clear();

            for(int i = 0; i < 541; i++) {
                // heres that mysterious magic number again...
                float angle = 0.00872665*float(i) - 0.7853985;

                cartesian_transform.push_back({
                        sick_msg->data[i] * cosf(angle), // x-coordinate
                        sick_msg->data[i] * sinf(angle)  // y-coordinate
                });
            }

            // plot a single point onscreen at a given set of coordinates
            auto plot_point_on_screen = [](int x, int y, int color, SDL_Surface* win) {
                SDL_Rect r;
                //r.x = x-1;
                //r.y = y-1;
                //r.h = 3;
                //r.w = 3;

                r.x = x;
                r.y = y;
                r.h = 1;
                r.w = 1;

                SDL_FillRect(win, &r, color);
            };

            // clear and draw lines
            SDL_FillRect(window, NULL, 0x00000000);
            draw_reference_lines(window, reference_color);

            // now those coordinates need to be mapped onto the screen surface
            for(int i = 0; i < 541; i++) {
                auto& pt = cartesian_transform[i];
                int x = mapFloat(pt.first,  -visible_radius, visible_radius, 0, WINDOW_SZ);
                int y = mapFloat(pt.second, -visible_radius, visible_radius, WINDOW_SZ, 0);
                plot_point_on_screen(x, y, plot_color, window);
            }

            SDL_Flip(window);

            // track events so everything can be closed 
            // when the window is closed
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                switch(event.type) {
                    case SDL_QUIT:
                        display_window_open = false;
                    default:
                        break;
                }
            }

        });

    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_SetVideoMode(WINDOW_SZ, WINDOW_SZ, 32, SDL_HWSURFACE);
    display_window_open = true;

    // specify the color
    plot_color = SDL_MapRGB(window->format, 255, 0, 0);          // red
    reference_color = SDL_MapRGB(window->format, 255, 255, 255); // white
    coord_color = SDL_MapRGB(window->format, 64, 64, 64);        // grey

    CPJL_Message::loop();

    while(display_window_open)
        usleep(1000000);

    SDL_Quit();

    return 0;
}

void draw_reference_lines(SDL_Surface* win, int color) {
    SDL_Rect r;

    const int half_x = WINDOW_SZ / 2;

    for(int i = -visible_radius; i <= visible_radius; i += 1000) {
        // plot a coordinate line every meter
        int mapped_i = mapFloat(i, -visible_radius, visible_radius, 0, WINDOW_SZ);

        r.x = mapped_i;
        r.y = 0;
        r.h = WINDOW_SZ;
        r.w = 1;
        SDL_FillRect(win, &r, coord_color);

        r.x = 0;
        r.y = mapped_i;
        r.h = 1;
        r.w = WINDOW_SZ;
        SDL_FillRect(win, &r, coord_color);
    }

    r.x = half_x;
    r.y = 0;
    r.w = 1;
    r.h = half_x;
    SDL_FillRect(win, &r, color);

    r.y = half_x;
    r.x = 0;
    r.w = WINDOW_SZ;
    r.h = 1;
    SDL_FillRect(win, &r, color);

    for(int i = 0; i < half_x; i++) {
        r.x = half_x + i;
        r.y = half_x + i;
        r.h = 1;
        r.w = 1;
        SDL_FillRect(win, &r, color);

        r.x = half_x - i;
        SDL_FillRect(win, &r, color);
    }

}
