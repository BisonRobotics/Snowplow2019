#pragma once

#include <string>
#include <vector>
#include <ncurses.h>
#include <utility>

class NcursesUtility {
private:
    int rows = 0, columns = 0;
    int current_color_index = 1;

    // used by other methods to place characters onscreen
    void place_char_at(int y, int x, int ch);

    // user supplies a color and this method makes it 
    // active. dev doesnt need to use this directly ever
    void activate_color(int color);
    void deactivate_color(int color);

public:
    // initialize the ncurses subsystem
    NcursesUtility(void);

    // stop the ncurses subsystem and return window to normal
    ~NcursesUtility(void);

    // window size parameters
    int getRows(void);
    int getColumns(void);

    // cursor position parameters
    auto getCursorPosition(void) -> std::pair<int, int>;

    // pretty self-explanatory. clears everything off of the screen
    void clear(void);

    // setup all of the colors needed for this application. see 
    // the test file for info on how to use this function
    void setupColors(const std::vector<std::vector<int>> color_array);

    // display a string with some formatting detail
    void displayStringAt(std::string str, int row, int column, int color);

    // initializing single colors, automatically creates color indexes
    int initializeColor(int foreground, int background);
    int initializeSolidColor(int color);

    // refresh the actual screen with whatever new data the dev has placed
    void flip(void);

    // waits until user presses a button
    int getCharacter(void);
    int getCharacter(int timeout); // wait only for a specific period of time

    // parameters are passed in like: {x, y, h, w}
    void drawBox(std::vector<int> params, int color);

    // straight line-drawing utilities
    void drawHorizontalLine(int y_offset, int color);
    void drawVerticalLine(int x_offset, int color);
}

// gotta have my pops
#define FULL_SEND ;
FULL_SEND
#undef FULL_SEND