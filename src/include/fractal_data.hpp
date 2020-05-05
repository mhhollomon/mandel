#if not defined(MANDEL_FRACTAL_DATA_HPP_)
#define MANDEL_FRACTAL_DATA_HPP_

#include "fixed_array.hpp"
#include <complex>
#include <memory>

struct fractal_meta_data {
    std::complex<double> bb_top_left;
    std::complex<double> bb_bottom_right;
    int limit;
    int samples_real;
    int samples_img;
    int max_iterations;
    int min_iterations;
};


struct fractal_point_data {
    std::complex<double> last_value = 0.0;
    double last_modulus = 0.0;
    int iterations = 0;
    bool diverged = false;

    fractal_point_data() = default;
    fractal_point_data(fractal_point_data const &o) = default;
};

using point_row = fixed_array<fractal_point_data>;

using point_grid = fixed_array<std::shared_ptr<point_row>>;

#endif
