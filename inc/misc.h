#ifndef __JJC__MISC__H__
#define __JJC__MISC__H__

#include <string>
#include <sys/time.h>

// Visual Studio Code has a problem with size_t, the compiler 
// does NOT, and we all know who is more important

std::string pad_left(std::string input, size_t sz) {
    while(input.size() < sz)
        input = " " + input;
    return input;
}

std::string pad_right(std::string input, size_t sz) {
    while(input.size() < sz)
        input += " ";
    return input;
}

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
        float output_start, float output_end) {

    return output_start 
            + ((output_end - output_start) 
            / (input_end - input_start)) 
            * (input - input_start);
}

double mapDouble(
        double input,
        double input_start,  double input_end,
        double output_start, double output_end) {

    return output_start 
            + ((output_end - output_start) 
            / (input_end - input_start)) 
            * (input - input_start);
}

// it think this alternate form looks nicer. sue me
#ifndef map_float
#define map_float mapDouble
#endif // map_float

uint64_t UsecTimestamp(void) {
    timeval tv;
    gettimeofday(&tv, NULL);

    return (tv.tv_sec * 1000000) + tv.tv_usec;
}

#endif // __JJC__MISC__H__