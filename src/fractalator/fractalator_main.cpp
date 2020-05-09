#include "fractalator.hpp"

#include "cxxopts.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>


fractalator_options parse_commandline(int argc, char**argv) {

    fractalator_options clopts;

    bool help_option;

    cxxopts::Options options("fractalator", "Mandelbrot Generator");

    options.add_options()
        ("h,help", "Print help message", cxxopts::value(help_option))
        ("o,output-file", "File in which to put the main output", cxxopts::value(clopts.output_file))
        ("d,debug", "Print debug information", cxxopts::value(clopts.debug))
        ("s,samples", "Number of samples in each dimension", cxxopts::value(clopts.samples)->default_value("0"))
        ("width", "Number of samples along the real axis", cxxopts::value(clopts.width)->default_value("0"))
        ("height", "Number of samples along the imaginary axis", cxxopts::value(clopts.height)->default_value("0"))
        ("aspect", "WxH samples along the axis - also computes new height", cxxopts::value(clopts.aspect))
        ("e,escape", "Escape radius", cxxopts::value(clopts.escape)->default_value("256.0"))
        ("ltr", "Left top real", cxxopts::value(clopts.left_top_real)->default_value("-2.0"))
        ("lti", "Left top imaginary", cxxopts::value(clopts.left_top_img)->default_value("2.0"))
        ("rbr", "Right bottom real", cxxopts::value(clopts.right_bottom_real)->default_value("2.0"))
        ("rbi", "Right bottom imaginary", cxxopts::value(clopts.right_bottom_img)->default_value("-2.0"))
        ("box", "Length of box - if given, replaces rbr and rbi", cxxopts::value(clopts.box)->default_value("0.0"))
        ("cr", "Center Real", cxxopts::value(clopts.center_real))
        ("ci", "Center Imaginary", cxxopts::value(clopts.center_img))
        ("l,limit", "Number of iterations to check divergence", cxxopts::value(clopts.limit)->default_value("1000"))
        ("j,jobs", "Number of parallel threads to use", cxxopts::value(clopts.jobs)->default_value("0"))
        ;


    auto results = options.parse(argc, argv);

    if (help_option) {
        std::cout << options.help() << "\n";
        exit(1);
    }

    bool has_center_real = (results.count("cr") > 0);
    bool has_center_img  = (results.count("ci") > 0);
    bool has_jobs        = (results.count("jobs") > 0);

    // -----------------------------------------------------------------------
    // Take care of the "easy" general options
    // -----------------------------------------------------------------------
    if (clopts.limit < 10) {
        std::cerr << "iteration limit cannot be less than 10\n";
        exit(1);
    }

    if (has_jobs and clopts.jobs < 0) {
        std::cerr << "--jobs must be nonnegative\n";
        exit(1);
    }

    if (clopts.samples > 0) {
        if (clopts.samples < 10) {
            std::cerr << "Samples must be 10 or greater\n";
            exit(1);
        }

        clopts.width = clopts.samples;
        clopts.height = clopts.samples;
    }

    if (clopts.escape < 2.0) {
        std::cerr << "--escape must be larger that 2.0\n";
        exit(1);
    }

    // -----------------------------------------------------------------------
    // Compute bounding box
    // -----------------------------------------------------------------------

    if (clopts.aspect != "") {
    // ---------------------------------------------------
    // Aspect driven config
    // ---------------------------------------------------
        int width,height;
        std::istringstream iss(clopts.aspect);
        char delimiter;

        iss >> width >> delimiter >> height;

        if (delimiter != 'x' and delimiter != 'X') {
            std::cerr << "Invalid separator in --aspect\n";
        }

        if (width < 10) {
            std::cerr << "Invalid width specifier (" << width << ") in --aspect\n";
            exit(1);
        }
        if (height < 10) {
            std::cerr << "Invalid height specifier (" << height << ") in --aspect\n";
            exit(1);
        }

        if (clopts.samples > 0) {
            std::cerr << "Cannot use --samples and --aspect together\n";
            exit(1);
        }
        if (clopts.height > 0) {
            std::cerr << "Cannot use --height and --aspect together\n";
            exit(1);
        }
        if (clopts.width > 0) {
            std::cerr << "Cannot use --width and --aspect together\n";
            exit(1);
        }

        clopts.height = height;
        clopts.width  = width;

        if ( not (results.count("box") > 0)) {
            std::cerr << "--aspect requires --box\n";
            exit(1);
        }

        if (clopts.box <= 0.0) {
            std::cerr << "--box must be positive\n";
            exit(1);
        }

        double aspect_ratio  = ((double)height/(double)width);
        double box_width  = clopts.box;
        double box_height = clopts.box * aspect_ratio;

        if (has_center_real and has_center_img) {
            clopts.left_top_real = clopts.center_real - (box_width/2.0);
            clopts.left_top_img  = clopts.center_img  + (box_height/2.0);

            clopts.right_bottom_real = clopts.center_real + (box_width/2.0);
            clopts.right_bottom_img  = clopts.center_img  - (box_height/2.0);
        } else if (results.count("ltr") > 0 and results.count("lti") > 0 ) {
            clopts.right_bottom_real = clopts.left_top_real + box_width;
            clopts.right_bottom_img  = clopts.left_top_img  - box_height;
        } else if (results.count("rbr") > 0 and results.count("rbi") > 0) {
            clopts.left_top_real = clopts.right_bottom_real - box_width;
            clopts.left_top_img  = clopts.right_bottom_img  + box_height;
        } else {
            std::cerr << "One of the following pairs must be specified with --aspect (--cr,--ci), (--ltr,lti), "
                << "(--rbr,--rbi)\n";
            exit(1);
        }

    } else if ( results.count("box") > 0) {
    // ---------------------------------------------------
    // Box driven config (without aspect)
    // ---------------------------------------------------
        if (has_center_real and has_center_img) {
            clopts.left_top_real = clopts.center_real - clopts.box/2.0;
            clopts.left_top_img  = clopts.center_img  + clopts.box/2.0;

            clopts.right_bottom_real = clopts.center_real + (clopts.box/2.0);
            clopts.right_bottom_img  = clopts.center_img  - (clopts.box/2.0);
        } else if (results.count("ltr") > 0 and results.count("lti") > 0 ) {
            clopts.right_bottom_real = clopts.left_top_real + clopts.box;
            clopts.right_bottom_img  = clopts.left_top_img  - clopts.box;
        } else if (results.count("rbr") > 0 and results.count("rbi") > 0) {
            clopts.left_top_real = clopts.right_bottom_real - clopts.box;
            clopts.left_top_img  = clopts.right_bottom_img  + clopts.box;
        } else {
            std::cerr << "One of the following pairs must be specified with --box (--cr,--ci), (--ltr,lti), "
                << "(--rbr,--rbi)\n";
            exit(1);
        }

    } else {
    // ---------------------------------------------------
    // Direct bounding box config
    // ---------------------------------------------------


        if (has_center_real or has_center_img) {
            std::cerr << "--cr and --ci may only be used with --box\n";
            exit(1);
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
    }


    // -----------------------------------------------------------------------
    // Final checks on (possibly) derived values
    // -----------------------------------------------------------------------
    if (clopts.width < 10) {
        std::cerr << "--width cannot be less than 10\n";
        exit(0);
    }

    if (clopts.height < 10) {
        std::cerr << "--height cannot be less than 10\n";
        exit(0);
    }

    return clopts;

}



int main (int argc, char*argv[]) {

    auto clopts = parse_commandline(argc, argv);

    std::cout << "bounding box = " 
        << std::fixed << std::setprecision( 16 ) 
        << "(" << clopts.left_top_real << ", " << clopts.left_top_img << "), "
        << "(" << clopts.right_bottom_real << ", " << clopts.right_bottom_img << ")\n";

    compute_fractal(clopts);

    return 0;
}
