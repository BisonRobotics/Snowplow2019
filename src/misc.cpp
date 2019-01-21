#include <misc.h>


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

uint64_t UsecTimestamp(void) {
    timeval tv;
    gettimeofday(&tv, NULL);

    return (tv.tv_sec * 1000000) + tv.tv_usec;
}
