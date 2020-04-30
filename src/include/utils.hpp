#if !defined(MANDEL_UTILS_HPP_)
#define MANDEL_UTILS_HPP_


#include "cxxopts.hpp"

using coord = double;


CLIOptions parse_commandline(int argc, char**argv);

void create_image(CLIOptions const &clopts);

#endif
