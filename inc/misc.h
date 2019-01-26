#ifndef __JJC__MISC__H__
#define __JJC__MISC__H__

#include <string>
#include <sys/time.h>

// Visual Studio Code has a problem with size_t, the compiler 
// does NOT, and we all know who is more important
std::string pad_left(std::string input, size_t sz);
std::string pad_right(std::string input, size_t sz);

// should prolly just change the function name and all 
// the occurances of it, but that is a task for another day
#ifndef pad_string_right
#define pad_string_right pad_right
#endif // pad_string_right

#ifndef pad_string_left
#define pad_string_left pad_left
#endif // pad_string_left

float mapFloat(
        float input, 
        float input_start,  float input_end, 
        float output_start, float output_end);

double mapDouble(
        double input,
        double input_start,  double input_end,
        double output_start, double output_end);

// it think this alternate form looks nicer. sue me
#ifndef map_float
#define map_float mapDouble
#endif // map_float

uint64_t UsecTimestamp(void);

float rotation_crossing(float angle);

int clamp(int value, const int min, const int max);
int clamp(int value, const int max_magnitude);

#endif // __JJC__MISC__H__