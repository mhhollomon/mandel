#include "cxxopts.hpp"

#include "fractal_file.hpp"

#include "color_script_engine.hpp"

#include "bmp_file.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>


struct CLIOptions {
    std::string input_file;
    std::string output_file;
    std::string script_file = "";
    bool  help = false;
};

CLIOptions parse_commandline(int argc, char**argv) {

    CLIOptions clopts;

    try {
        cxxopts::Options options("colorator", "fractal file colorizer");

        options.add_options()
            ("h,help", "Print help message", cxxopts::value(clopts.help))
            ("o,output-file", "File in which to put the main output", cxxopts::value(clopts.output_file))
            ("i,input-file", ".fract file to read for fractal data", cxxopts::value(clopts.input_file))
            ("s,script-file", "angelscript file to read for coloring algorithm", cxxopts::value(clopts.script_file))
            ;


        auto results = options.parse(argc, argv);

        // Grrr. have to handle help here because we need the options object.
        if (clopts.help) {
            std::cout << options.help() << "\n";
            exit(1);
        }


    } catch (const cxxopts::OptionException& e) {
        std::cerr << "Well, that didn't work: " << e.what() << "\n";
        exit(1);
    }

    if (clopts.input_file == "") throw std::runtime_error("No input file specified");
    if (clopts.output_file == "") throw std::runtime_error("No output file specified");
    
    return clopts;

}

fractal_point_data & get_result_from_file(fractal::Results const &data) {
    fractal_point_data *retval = new fractal_point_data();

    retval->last_value = std::complex<double>(data.last_value().real(), data.last_value().img());
    retval->last_modulus = data.last_modulus();
    retval->iterations = data.iterations();
    retval->diverged = data.diverged();

    return *retval;
}


pixel color_algo_1(fractal_meta_data fp, fractal_point_data const &results) {
    //
    // Starting at blue, work around the coor wheel so that we are at red
    // Just before diverging.
    // Value is proportional to the count.
    //
    if (results.diverged) {
        double smoothed_count = double(results.iterations) + 1.0 + std::log(std::log2(results.last_modulus));
        double log_limited_count = std::log(smoothed_count)/std::log(double(fp.limit+1));
        double hue = (4.0f/6.0f) *(1.0 - log_limited_count);
        if (hue < 0.0) hue += 1.0;
        double value = log_limited_count;

        return pixel{hue, 1.0, value};

    } else {

        return pixel(0x00, 0x00, 0x00);
    }
}

void color_image(CLIOptions const &clopts, FractalFile const &data) {
    ColorScriptEngine se;
    bool using_script = false;

    
    auto params = data.get_meta_data();

    std::cerr << "params.limit = " << params.limit << "\n";
    
    auto output_file = BMPFile{clopts.output_file, params.samples_img, 
		params.samples_real};

    if (clopts.script_file != "") {
        using_script = true;
        se.initialize(clopts.script_file);

        std::cerr << "calling setup\n";
        if (not se.call_setup(&params)) {
            std::cerr << "Why didn't that work?\n";
            return;
        }
        std::cerr << "setup call complete\n";
    }
		

	auto rows = data.get_rows();

    if (using_script and se.has_prepass()) {
        std::cout << "calling prepass\n";
        for (int i = 0; i < rows->size(); ++i) {
            auto data_row = (*rows)[i];
            
            for (int j = 0; j < data_row->size(); ++j) {

                se.call_prepass((*data_row)[j]);
            }
        }
    }

    if (using_script) {
        std::cout << "calling precolor\n";
        se.call_precolor();
    }

    auto pixels = std::vector<pixel>{};
    std::cout << "colorizing\n";
	for (int i = 0; i < rows->size(); ++i) {
        auto data_row = (*rows)[i];
		
        pixels.clear();

        for (int j = 0; j < data_row->size(); ++j) {

            if (using_script) {
                pixels.push_back(se.call_colorize((*data_row)[j]));
            } else {
                pixels.push_back(color_algo_1(params, (*data_row)[j]));
            }
        }

        output_file.write_row(pixels);
    }

}

auto read_fractal_data(std::string input_file_name) {

    std::fstream input{input_file_name, std::ios::in | std::ios::binary};

    return FractalFile::read_from_file(input_file_name);


}

int main (int argc, char*argv[]) {

    auto clopts = parse_commandline(argc, argv);

    auto data = read_fractal_data(clopts.input_file);

    color_image(clopts, *data);

    return 0;
}


