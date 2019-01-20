#include <NcursesUtility.h>
#include <ncurses.h>
#include <iostream>
#include <misc.h> // pad_left, pad_right, and friends

// WHY ARE YOU IN THIS FILE!? LOOK AT THE 
// EXAMPLE CODE IN THE test/ DIRECTORY! SHOO!

NcursesUtility::NcursesUtility(void) {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    getmaxyx(stdscr, this->rows, this->columns);
    curs_set(0);
    start_color();
}

// maybe a 'using namespace NcursesUtility;' would help unclutter this pile of bulls*it

NcursesUtility::~NcursesUtility(void) {            endwin(); }
void NcursesUtility::activate_color(int color) {   attron(COLOR_PAIR(color)); }
void NcursesUtility::deactivate_color(int color) { attroff(COLOR_PAIR(color)); }

auto NcursesUtility::getCursorPosition(void) -> std::pair<int, int> {
    int x, y;
    getyx(stdscr, y, x);
    return {x, y};
}

void NcursesUtility::flip(void) {
    auto coords = this->getCursorPosition();
    mvcur(coords.first, coords.second, this->rows-1, this->columns-1);
    refresh(); 
}

int  NcursesUtility::getRows(void) {               return this->rows; }
int  NcursesUtility::getColumns(void) {            return this->columns; }

int NcursesUtility::getCharacter(void) { return getch(); }
int NcursesUtility::getCharacter(int timeout) {
    timeout(250);
    return getch();
}

void NcursesUtility::drawBox(std::vector<int> box, int color) {
    int x = box[0]; 
    int y = box[1];
    int h = box[2] - 1; 
    int w = box[3] - 1;

    this->activate_color(color);

    for(int j : {y, y+h}) {
        for(int i = x; i < (x+w); i++)
            this->place_char_at(j, i, ' ');
    }

    for(int i : {x, x+w}) {
        for(int j = y; j < (y+h); j++)
            this->place_char_at(j, i, ' ');
    }

    this->place_char_at(y+h, x+w, ' ');

    this->deactivate_color(color);
}

int NcursesUtility::initializeColor(int foreground, int background) {
    int current_color = this->current_color_index++;
    init_pair(current_color, foreground, background);
    return current_color;
}

int NcursesUtility::initializeSolidColor(int color) {
    return this->initializeColor(color, color);
}

void NcursesUtility::setupColors(const std::vector<std::vector<int>> color_array) {

    for(auto& v : color_array) {
        if(v.size() == 2) {
            init_pair(v[0], v[1], v[1]);
        }
        else if(v.size() == 3) {
            init_pair(v[0], v[1], v[2]);
        }
        else {
            std::cout << "NcursesUtility::setupColors -> Invalid color request\n";
            exit(EXIT_FAILURE);
        }
    }
}

void NcursesUtility::place_char_at(int y, int x, int ch) {
    mvaddch(y, x, ch);
}

void NcursesUtility::clear(void) {
    for(int i = 0; i < this->rows; i++) {
        for(int j = 0; j < this->columns; j++)
            this->place_char_at(i, j, ' ');           
    }
}

void NcursesUtility::displayStringAt(std::string str, int row, int column, int color) {
    this->activate_color(color);
    for(int i = 0; i < (int)str.size(); i++)
        this->place_char_at(row, column + i, str[i]);
    this->deactivate_color(color);
}

void NcursesUtility::drawHorizontalLine(int y_offset, int color) {
    this->activate_color(color);
    for(int i = 0; i < this->columns; i++)
        this->place_char_at(y_offset, i, ' ');
    this->deactivate_color(color);
}

void NcursesUtility::drawVerticalLine(int x_offset, int color) {
    this->activate_color(color);
    for(int i = 0; i < this->rows; i++)
        this->place_char_at(i, x_offset, ' ');
    this->deactivate_color(color);
}
