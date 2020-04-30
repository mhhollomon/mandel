#if !defined(MANDEL_COMPUTE_HPP_)
#define MANDEL_COMPUTE_HPP_

#include <complex>
#include <memory>
#include <vector>

using coord = double;

using point = std::complex<coord>;

struct check_results {
    point last_value = 0.0;
    coord last_modulus = 0.0;
    int iterations = 0;
    bool diverged = false;
};

struct fractal_params {
    point bb_top_left;
    point bb_bottom_right;
    int limit;
    int samples_real;
    int samples_img;
};

check_results mandelbrot_test(point test_point, int limit);

using result_slice = std::shared_ptr<std::vector<check_results> >;

void compute_slice(result_slice rs, int limit, 
        int start_index, int end_index,
        coord base_img, coord base_real, coord real_increment);

std::shared_ptr<std::vector<result_slice>> compute_fractal(fractal_params p);


#endif
