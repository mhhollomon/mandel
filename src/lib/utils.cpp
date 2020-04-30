#include "utils.hpp"

#include "bmp_file.hpp"

#include <cmath>
#include <cstdint>
#include <complex>
#include <iostream>
#include <string>
#include <vector>


void create_image(CLIOptions const &clopts);

CLIOptions parse_commandline(int argc, char**argv) {

    CLIOptions clopts;

    try {
        cxxopts::Options options("mandel", "Mandelbrot Generator");

        options.add_options()
            ("h,help", "Print help message", cxxopts::value(clopts.help))
            ("o,output-file", "File in which to put the main output", cxxopts::value(clopts.output_file))
			("d,debug", "Print debug information", cxxopts::value(clopts.debug))
            ("s,samples", "Number of samples in each dimension", cxxopts::value(clopts.samples)->default_value("0"))
            ("width", "Width of image in pixels", cxxopts::value(clopts.width)->default_value("100"))
            ("height", "Height of image in pixels", cxxopts::value(clopts.height)->default_value("100"))
            ("ltr", "Left top real", cxxopts::value(clopts.left_top_real)->default_value("-2.0"))
            ("lti", "Left top imaginary", cxxopts::value(clopts.left_top_img)->default_value("2.0"))
            ("rbr", "Right bottom real", cxxopts::value(clopts.right_bottom_real)->default_value("2.0"))
            ("rbi", "Right bottom imaginary", cxxopts::value(clopts.right_bottom_img)->default_value("-2.0"))
            ("box", "Length of box - if given, replaces rbr and rbi", cxxopts::value(clopts.box)->default_value("0.0"))
            ("cr", "Center Real", cxxopts::value(clopts.center_real))
            ("ci", "Center Imaginary", cxxopts::value(clopts.center_img))
            ("limit", "Number of iterations to check divergence", cxxopts::value(clopts.limit)->default_value("1000"))
            ;


        auto results = options.parse(argc, argv);

        // Grrr. have to handle help here because we need the options object.
        if (clopts.help) {
            std::cout << options.help() << "\n";
            exit(1);
        }

        clopts.has_center_real = (results.count("cr") > 0);
        clopts.has_center_img  = (results.count("ci") > 0);

    } catch (const cxxopts::OptionException& e) {
        std::cerr << "Well, that didn't work: " << e.what() << "\n";
        exit(1);
    }

    if (clopts.samples > 0) {
        if (clopts.samples < 10) {
            throw std::runtime_error("Samples must be 0 or greater\n");
        }

        clopts.width = clopts.samples;
        clopts.height = clopts.samples;
    }

    if (clopts.width < 10) {
        throw std::runtime_error("Image width cannot be less than 10\n");
    }

    if (clopts.height < 10) {
        throw std::runtime_error("Image height cannot be less than 10\n");
    }

    if (clopts.limit < 10) {
        throw std::runtime_error("iteration limit cannot be less than 10\n");
    }

    if (clopts.has_center_real or clopts.has_center_img) {
        if ( !clopts.has_center_real or !clopts.has_center_img) {
            throw std::runtime_error("--cr and --ci must be used together\n");
        }

        if (clopts.box == 0.0) {
            throw std::runtime_error( 
                    "--box must be specified if --cr and --ci are specified\n");
        } else if (clopts.box < 0.0) {
            throw std::runtime_error( "Box size cannot be negative");
        }

        clopts.left_top_real = clopts.center_real - (clopts.box/2.0);
        clopts.right_bottom_real = clopts.center_real + (clopts.box/2.0);
        clopts.left_top_img = clopts.center_img + (clopts.box/2.0);
        clopts.right_bottom_img = clopts.center_img - (clopts.box/2.0);

    } else if (clopts.box < 0.0) {
        std::cerr << "Box size cannot be negative";
    } else if (clopts.box != 0.0) {
        clopts.right_bottom_real = clopts.left_top_real + clopts.box;
        clopts.right_bottom_img = clopts.left_top_img - clopts.box;
    }

    if (clopts.left_top_real < -2.0 or clopts.right_bottom_real > 2.0
            or clopts.left_top_img > 2.0 or clopts.right_bottom_img < -2.0) {

        std::cerr << "Mandelbrot set is bounded by the box (-2.0 + 2.0i), (2.0-2.0i)."
            " Given coordinates are outside this box\n";
    }

    if (clopts.left_top_real >= clopts.right_bottom_real or
            clopts.left_top_img <= clopts.right_bottom_img) {
        std::cerr << "Left top point must be to the left and above the right bottom point\n";
    }

    return clopts;

}


using point = std::complex<coord>;


struct check_results {
    point last_value = 0.0;
    coord last_modulus = 0.0;
    short int iterations = 0;
    bool diverged = false;
};

check_results test_point(point test_point, short int limit);


void create_image(CLIOptions const &clopts) {
    // because we are writing into a bmp we have to go left to right, bottopm to top
    coord real_increment = (clopts.right_bottom_real - clopts.left_top_real) / coord(clopts.width);
    coord img_increment = (clopts.left_top_img - clopts.right_bottom_img) / coord(clopts.height);

    auto output_file = BMPFile{clopts.output_file, clopts.height, clopts.width};
    auto pixels = std::vector<pixel>{};



    const bool x_debug = false;

    for (short int y = 0; y < clopts.height; ++y) {
        pixels.clear();
        if (y % 100 == 0)
            std::cout << "----------------- starting y = " << y << " ---\n";
        for (short int x = 0; x < clopts.width; ++x) {
            if (x_debug) std::cout << "x = " << x << ": ";
            point p{clopts.left_top_real + (real_increment * x),
                        clopts.right_bottom_img + (img_increment * y)};
            auto results = test_point( p, clopts.limit);
            if (x_debug) std::cout << "(" << p << ") " << results.diverged;
            pixel new_px;
            if (results.diverged) {
                coord smoothed_count = coord(results.iterations) + 1.0 + std::log(std::log2(results.last_modulus));
                double log_limited_count = std::log(smoothed_count)/std::log(double(clopts.limit+1));
                double hue = (4.0f/6.0f) *(1.0 - log_limited_count);
                if (hue < 0.0) hue += 1.0;
                double value = log_limited_count;
                new_px = pixels.emplace_back(hue, 1.0, value);

                if (x_debug) {
                    std::cerr << " m = " << results.last_modulus 
                        << " i = " << results.iterations
                        << " s = " << smoothed_count
                        << " llc = " << log_limited_count
                        << " hue = " << hue
                        << " value = " << value;
                }

            } else {

                new_px = pixels.emplace_back(0x00, 0x00, 0x00);
            }

            if (x_debug) std::cerr << " px = " << new_px << "\n";
        }
        output_file.write_row(pixels);

    }
}


check_results test_point(point test_point, short int limit) {

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


