#include "utils.hpp"
#include "compute.hpp"

#include "bmp_file.hpp"

void create_image_v2(CLIOptions const &clopts);

int main (int argc, char*argv[]) {

    auto clopts = parse_commandline(argc, argv);

    std::cout << "bounding box = (" << clopts.left_top_real << ", " << clopts.left_top_img << "), "
        << "(" << clopts.right_bottom_real << ", " << clopts.right_bottom_img << ")\n";

    create_image_v2(clopts);

    return 0;
}


pixel color_algo_1(CLIOptions const &clopts, check_results const &results) {
    //
    // Starting at blue, work around the coor wheel so that we are at red
    // Just before diverging.
    // Value is proportional to the count.
    //
    if (results.diverged) {
        coord smoothed_count = coord(results.iterations) + 1.0 + std::log(std::log2(results.last_modulus));
        double log_limited_count = std::log(smoothed_count)/std::log(double(clopts.limit+1));
        double hue = (4.0f/6.0f) *(1.0 - log_limited_count);
        if (hue < 0.0) hue += 1.0;
        double value = log_limited_count;

        return pixel{hue, 1.0, value};

    } else {

        return pixel(0x00, 0x00, 0x00);
    }
}

pixel color_algo_2(CLIOptions const &clopts, check_results const &results) {
    //
    // Starting at blue, work around the color wheel N times
    // Value is proportional to the count.
    //
    const double N = 3.0;
    if (results.diverged) {
        coord smoothed_count = coord(results.iterations) + 1.0 + std::log(std::log2(results.last_modulus));
        double log_limited_count = std::log(smoothed_count)/std::log(double(clopts.limit+1));
        double hue = (4.0f/6.0f) *(1.0 - N*log_limited_count);
        while (hue < 0.0) hue += 1.0;
        double value = log_limited_count;

        return pixel{hue, 1.0, value};

    } else {

        return pixel(0x00, 0x00, 0x00);
    }
}

pixel color_algo_3(CLIOptions const &clopts, check_results const &results) {
    //
    // Starting at blue, work around the color wheel N times
    // Avoid the "purple" sector of the color wheel. So modulo things
    // by .6666. Then subtract.
    // Value is proportional to the count.
    //
    const double N = 6.0;
    const double color_wheel_size = 4.0f/6.0f;
    if (results.diverged) {
        coord smoothed_count = coord(results.iterations) + 1.0 + std::log(std::log2(results.last_modulus));
        double log_limited_count = std::log(smoothed_count)/std::log(double(clopts.limit+1));

        double hue = color_wheel_size - fmod(N*log_limited_count, color_wheel_size);
        while (hue < 0.0) hue += 1.0;

        double value = log_limited_count;

        return pixel{hue, 1.0, value};

    } else {

        return pixel(0x00, 0x00, 0x00);
    }
}
void write_bmp_file(CLIOptions const &clopts, std::shared_ptr<std::vector<result_slice>> data) {
    std::cout << "Writing File\n";

    auto output_file = BMPFile{clopts.output_file, clopts.height, clopts.width};
    auto pixels = std::vector<pixel>{};

    for (auto const & data_row : *data) {
        pixels.clear();

        for( auto const &results : *data_row) {

            pixels.push_back(color_algo_1(clopts, results));
        }

        output_file.write_row(pixels);
    }

}

void create_image_v2(CLIOptions const &clopts) {
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

    write_bmp_file(clopts, fractal_data);
}
