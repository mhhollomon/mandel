#if not defined(_FRACTALATOR_HPP_)
#define _FRACTALATOR_HPP_

#include "compute.hpp"

struct fractalator_options {
    std::string output_file;
    std::string aspect;
    double box;
    double center_real;
    double center_img;
    int    samples;
    double escape;
    double left_top_real;
    double left_top_img;
    double right_bottom_real;
    double right_bottom_img;
    int    width;
    int    height;
    int    limit;
    bool   debug = false;
    int    jobs;

};

void compute_fractal(fractalator_options const &clopts);


void write_fractal_file(fractalator_options const &clopts, 
        std::shared_ptr<point_grid> data) ;

#endif
