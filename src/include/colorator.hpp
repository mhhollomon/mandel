#if not defined(MANDEL_COLORATOR_HPP_)
#define MANDEL_COLORATOR_HPP_

#include "fractal_file.hpp"

struct colorator_options {
    std::string input_file;
    std::string output_file;
    std::string script_file = "";
    std::string script_args = "";
};

void color_image(colorator_options const &clopts, FractalFile const &data);

#endif
