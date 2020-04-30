#include "compute.hpp"

#include <cmath>
#include <iostream>

check_results mandelbrot_test(point test_point, int limit) {

    check_results retval;
    for (;retval.iterations < limit; ++retval.iterations) {

        retval.last_value =  retval.last_value*retval.last_value + test_point;

        retval.last_modulus = std::abs(retval.last_value);

        if (retval.last_modulus > 2) {
            retval.diverged = true;
            return retval;
        }


    }

    retval.diverged = false;
    return retval;
}

void compute_slice(result_slice rs, int limit, 
        int start_index, int end_index,
        coord base_img, coord base_real, coord real_increment) {

    for (int index = start_index; index < end_index; ++index) {
        coord real_coord = base_real + (real_increment * index);
        // TODO vectors suck.
        // This won't allow me to write in place to file parts of a row.
        //
        rs->push_back(mandelbrot_test(point{real_coord, base_img}, limit));
    }
}

std::shared_ptr<std::vector<result_slice>> compute_fractal(fractal_params p) {
    auto retval = std::make_shared<std::vector<result_slice>>();

    retval->reserve(p.samples_img);
    coord real_increment = (p.bb_bottom_right.real() - p.bb_top_left.real()) / p.samples_real;
    coord img_increment = (p.bb_top_left.imag() - p.bb_bottom_right.imag()) / p.samples_img;

    coord base_real = p.bb_top_left.real();

    for (int row = 0; row < p.samples_img; ++row) {
        if (row % 100 == 0)
            std::cout << "----------------- starting row = " << row << " ---\n";
        
        // TODO - this breaks encapsualtion of the type
        auto rs = std::make_shared<std::vector<check_results> >();
        rs->reserve(p.samples_real);

        coord base_img = p.bb_bottom_right.imag() + ( img_increment * row );

        compute_slice(rs, p.limit, 0, p.samples_real, 
                base_img, base_real, real_increment);

        retval->push_back(rs);
    }

    return retval;
}
