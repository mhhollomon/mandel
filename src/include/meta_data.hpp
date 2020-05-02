#if not defined(MANDEL_META_DATA_HPP_)
#define MANDEL_META_DATA_HPP_

#include <complex>

struct fractal_meta_data {
    std::complex<double> bb_top_left;
    std::complex<double> bb_bottom_right;
    int limit;
    int samples_real;
    int samples_img;
    int max_iterations;
    int min_iterations;
};

#endif
