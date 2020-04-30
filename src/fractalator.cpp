#include "compute.hpp"

#include "fractal_file.hpp"

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
