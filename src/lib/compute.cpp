#include "compute.hpp"

#include <cmath>
#include <iostream>

fractal_point_data mandelbrot_test(std::complex<double> test_point, int limit) {

    fractal_point_data retval;
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
        double base_img, double base_real, double real_increment) {

    for (int index = start_index; index < end_index; ++index) {
        double real_double = base_real + (real_increment * index);
        (*rs)[index] = mandelbrot_test({real_double, base_img}, limit);
    }
}

std::shared_ptr<fixed_array<result_slice>> compute_fractal(fractal_params p) {
    auto retval = std::make_shared<fixed_array<result_slice>>(p.samples_img);

    double real_increment = (p.bb_bottom_right.real() - p.bb_top_left.real()) / p.samples_real;
    double img_increment = (p.bb_top_left.imag() - p.bb_bottom_right.imag()) / p.samples_img;

    double base_real = p.bb_top_left.real();

    for (int row = 0; row < p.samples_img; ++row) {
        if (row % 100 == 0)
            std::cout << "----------------- starting row = " << row << " ---\n";
        
        // TODO - this breaks encapsualtion of the type
        auto rs = std::make_shared<fixed_array<fractal_point_data> >(p.samples_real);

        double base_img = p.bb_bottom_right.imag() + ( img_increment * row );

        compute_slice(rs, p.limit, 0, p.samples_real, 
                base_img, base_real, real_increment);

        (*retval)[row] = rs;
    }

    return retval;
}
