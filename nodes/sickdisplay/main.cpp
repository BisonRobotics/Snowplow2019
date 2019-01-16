#include <iostream>
#include <vector>
#include <utility>
#include <SDL/SDL.h> // 2D graphics library for displaying data

#include <CPJL.hpp>              // CPJL... duh
#include <cpp/SickMeasurement.h> // CPJL message type for SICK data
#include <SICK_Sensor.h>         // for accessing the SICK scanner

using namespace std;

#define HEIGHT 600
#define WIDTH 800

SickMeasurement* sick_msg = NULL;

// needs to be accessed by callback
SDL_Surface* window = NULL;
vector<pair<float, float>> cartesian_transform;
const float visible_radius = 3000.0f; // millimeters (raw from SICK)
int plot_color;

int main(int argc, char* argv[]) {

    if(argc != 2) {
        cout << "Usage: " << argv[0] << " <CPJL host location>\n";
        exit(EXIT_FAILURE);
    }

    sick_msg = new SickMeasurement(new CPJL(argv[1], 14000), "sick_data", 
        []() {
            // updates the UI

            cartesian_transform.clear();

            for(int i = 0; i < 451; i++) {
                float angle = float(i-90) * 0.00872665;
                cartesian_transform.push_back({
                        sick_msg->data[i] * cosf(angle), 
                        sick_msg->data[i] * sinf(angle)
                });
            }

            // plot a single point onscreen at a given set of coordinates
            auto plot_point_on_screen = [](int x, int y) {
                SDL_Rect r;
                r.x = x-1;
                r.y = y-1;
                r.h = 3;
                r.w = 3;
                SDL_FillRect(window, &r, plot_color);
            };

            SDL_FillRect(window, NULL, 0x00000000);

            // now those coordinates need to be mapped onto the screen surface
            for(int i = 0; i < 451; i++) {
                auto& pt = cartesian_transform[i];
                int x = mapFloat(pt.first, -visible_radius, visible_radius, 0, 800);
                int y = mapFloat(pt.second, -visible_radius, visible_radius, 800, 0);
                plot_point_on_screen(x, y);
            }

            SDL_Flip(window);
        });

    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_SetVideoMode(800, 800, 32, SDL_HWSURFACE);
    float aspect_ratio = ((float)WIDTH / (float)HEIGHT);

    // specify the color
    plot_color = SDL_MapRGB(window->format, 255, 0, 0);

    CPJL_Message::loop();

    bool loop = true;
    while(loop) {
        usleep(1000000);
    }

    SDL_Quit();

    return 0;
}
