#ifndef __JJC__MISC__H__
#define __JJC__MISC__H__

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

#endif // __JJC__MISC__H__