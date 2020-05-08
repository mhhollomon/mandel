#include "colorator.hpp"

#include "color_script_engine.hpp"

#include "bmp_file.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>




void color_image(colorator_options const &clopts, FractalFile const &data) {
    ColorScriptEngine se;
    
    auto params = data.get_meta_data();

    std::cerr << "params.limit = " << params.limit << "\n";
    
    auto output_file = BMPFile{clopts.output_file, params.samples_img, 
		params.samples_real};

    se.initialize(clopts.script_file);

    std::cerr << "calling setup\n";
    if (not se.call_setup(&params, clopts.script_args)) {
        std::cerr << "Why didn't that work?\n";
        return;
    }
    std::cerr << "setup call complete\n";


	auto rows = data.get_rows();

    if (se.has_prepass()) {
        std::cout << "calling prepass\n";
        for (int i = 0; i < rows->size(); ++i) {
            auto data_row = (*rows)[i];
            
            for (int j = 0; j < data_row->size(); ++j) {

                se.call_prepass((*data_row)[j]);
            }
        }
    }

    std::cout << "calling precolor\n";
    se.call_precolor();

    auto pixels = std::vector<pixel>{};
    std::cout << "colorizing\n";
	for (int i = 0; i < rows->size(); ++i) {
        auto data_row = (*rows)[i];
		
        pixels.clear();

        for (int j = 0; j < data_row->size(); ++j) {
            pixels.push_back(se.call_colorize((*data_row)[j]));
        }

        output_file.write_row(pixels);
    }

}

