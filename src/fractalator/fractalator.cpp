#include "compute.hpp"

#include "fractal_file.hpp"

#include "cxxopts.hpp"

struct CLIOptions {
    std::string output_file;
    coord box;
    coord center_real;
    coord center_img;
    int   samples;
    coord left_top_real;
    coord left_top_img;
    coord right_bottom_real;
    coord right_bottom_img;
    int   width;
    int   height;
    int   limit;
    bool  debug = false;
    bool  help = false;
    bool  has_center_real;
    bool  has_center_img;

};

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


void compute_fractal(CLIOptions const &clopts);

int main (int argc, char*argv[]) {

    auto clopts = parse_commandline(argc, argv);

    std::cout << "bounding box = (" << clopts.left_top_real << ", " << clopts.left_top_img << "), "
        << "(" << clopts.right_bottom_real << ", " << clopts.right_bottom_img << ")\n";

    compute_fractal(clopts);

    return 0;
}


void write_fractal_file(CLIOptions const &clopts, std::shared_ptr<std::vector<result_slice>> data) {
    std::cout << "Writing File\n";

    auto output_file = FractalFile{clopts.output_file};
    output_file.add_metadata(
                { clopts.left_top_real, clopts.left_top_img },
                { clopts.right_bottom_real, clopts.right_bottom_img },
                clopts.limit,
                clopts.width,
                clopts.height
            );

    for (auto const & data_row : *data) {
        output_file.write_row(*data_row);
    }

    output_file.finalize();
}

void compute_fractal(CLIOptions const &clopts) {
    std::cout << "Computing fractal\n";

    auto fractal_data = compute_fractal(
            fractal_params{
                point{ clopts.left_top_real, clopts.left_top_img },
                point{ clopts.right_bottom_real, clopts.right_bottom_img },
                clopts.limit,
                clopts.width,
                clopts.height
                }
        );

    write_fractal_file(clopts, fractal_data);
}
