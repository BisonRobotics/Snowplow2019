#pragma once


struct float_rect_t {
    float x, y, h, w;

    float_rect_t(float x, float y, float h, float w) {
        this->x = x;
        this->y = y;
        this->h = h;
        this->w = w;
    }
};

#include <initializer_list>
#include <vector>

std::initializer_list<float_rect_t> snowplow_list = {
    // main body
    {-325.0f, -265.0f, 500.0f, 650.0f},
    
    // left wheel well
    {-675.0f, 35.0f, 500.0f, 350.0f},

    // right wheel well
    {325.0f, 35.0f, 500.0f, 350.0f},

    // plow
    {-625.0f, 385.0f, 200.0f, 1250.0f},

    // SICK
    {-45.0f, -45.0f, 90.0f, 90.0f},
};

// the data above is tf'd at runtime to calculate actual pixel coords for the display
std::vector<float_rect_t> tf_snowplow_list;

float_rect_t warning_zone = {-1500.0f, 3000.0f, 4500.0f, 3000.0f};
