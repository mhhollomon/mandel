#if !defined(MANDEL_COMPUTE_HPP_)
#define MANDEL_COMPUTE_HPP_

#include "fixed_array.hpp"
#include "work_queue.hpp"
#include "fractal_data.hpp"

#include <memory>
#include <complex>

struct fractal_params {
    std::complex<double> bb_top_left;
    std::complex<double> bb_bottom_right;
    double escape_radius;
    int limit;
    int samples_real;
    int samples_img;
};

struct work_item {
    std::shared_ptr<point_row> output;
    int row_number;
    int limit;
    int start_index;
    int end_index;
    double base_img;
    double base_real;
    double real_increment;
    double escape_radius;
};

using fractal_work_queue = work_queue<work_item>;

fractal_point_data mandelbrot_test(std::complex<double> test_point, int limit);

void compute_slice(work_item wi);

std::shared_ptr<point_grid> compute_fractal(fractal_params p);
std::shared_ptr<point_grid> compute_fractal(fractal_params p,
        fractal_work_queue &wq, int jobs);


#endif
