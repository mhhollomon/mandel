#include "colorator.hpp"

#include "cxxopts.hpp"


colorator_options parse_commandline(int argc, char**argv) {

    colorator_options clopts;

    try {
        cxxopts::Options options("colorator", "fractal file colorizer");

        options.add_options()
            ("h,help", "Print help message", cxxopts::value(clopts.help))
            ("o,output-file", "File in which to put the main output", cxxopts::value(clopts.output_file))
            ("i,input-file", ".fract file to read for fractal data", cxxopts::value(clopts.input_file))
            ("s,script-file", "angelscript file to read for coloring algorithm", cxxopts::value(clopts.script_file))
            ("a,args", "key value pairs separated by semi-colon to pass to script", cxxopts::value(clopts.script_args))
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
    if (clopts.script_file == "") throw std::runtime_error("No script file specified");
    
    return clopts;

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


