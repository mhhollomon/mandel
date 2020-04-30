#include "cxxopts.hpp"

#include "fractal_file.hpp"

#include "color_script_engine.hpp"

// including this for the type coord, point, fractal_params
#include "compute.hpp"

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

fractal_params get_params_from_file(FractalFile const &data) {
	
	auto header = data.get_header();
	
	fractal_params p;
	
	p.bb_top_left = point{header.bb_tl().real(), header.bb_tl().img()};
	p.bb_bottom_right = point{header.bb_br().real(), header.bb_br().img()};
	p.limit = header.limit();
	p.samples_real = header.real_samples();
	p.samples_img = header.img_samples();
	
	return p;
}

check_results & get_result_from_file(fractal::Results const &data) {
    check_results *retval = new check_results();

    retval->last_value = point(data.last_value().real(), data.last_value().img());
    retval->last_modulus = data.last_modulus();
    retval->iterations = data.iterations();
    retval->diverged = data.diverged();

    return *retval;
}


pixel color_algo_1(fractal_params fp, check_results const &results) {
    //
    // Starting at blue, work around the coor wheel so that we are at red
    // Just before diverging.
    // Value is proportional to the count.
    //
    if (results.diverged) {
        coord smoothed_count = coord(results.iterations) + 1.0 + std::log(std::log2(results.last_modulus));
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

    
    auto params = get_params_from_file(data);
    
    auto output_file = BMPFile{clopts.output_file, params.samples_img, 
		params.samples_real};

    if (clopts.script_file != "") {
        using_script = true;
        se.initialize(clopts.script_file);
    }
		
    auto pixels = std::vector<pixel>{};

	auto rows = data.get_rows();

    if (using_script) {
        std::cerr << "calling setup\n";
        if (not se.call_setup(&params)) {
            std::cerr << "Why didn't that work?\n";
            return;
        }
        std::cerr << "setup call complete\n";

    }
	
    std::cout << "colorizing\n";
	for (int i = 0; i < rows.size(); ++i) {
		auto data_row = rows[i];
		
        pixels.clear();

		for (int j = 0; j < data_row.results_size(); ++j) {
			auto &result = get_result_from_file(data_row.results()[j]);

            if (using_script) {
                pixels.push_back(se.call_colorize(result));

            } else {
                pixels.push_back(color_algo_1(params, result));
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


