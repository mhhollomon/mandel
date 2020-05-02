#if !defined(MANDEL_COMPUTE_HPP_)
#define MANDEL_COMPUTE_HPP_

#include "fixed_array.hpp"
#include "fractal_data.hpp"

#include <memory>
#include <complex>

struct fractal_params {
    std::complex<double> bb_top_left;
    std::complex<double> bb_bottom_right;
    int limit;
    int samples_real;
    int samples_img;
};

fractal_point_data mandelbrot_test(std::complex<double> test_point, int limit);

using result_slice = std::shared_ptr<fixed_array<fractal_point_data> >;

void compute_slice(result_slice rs, int limit, 
        int start_index, int end_index,
        double base_img, double base_real, double real_increment);

std::shared_ptr<fixed_array<result_slice>> compute_fractal(fractal_params p);


#endif
