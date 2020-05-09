#include "fractalator.hpp"


#include "fractal_file.hpp"
#include "work_queue.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>


void write_fractal_file(fractalator_options const &clopts, 
        std::shared_ptr<point_grid> data) {
    std::cout << "Writing File\n";

    int max_iter = 0;
    int min_iter = clopts.limit;

    for (auto const & data_row : *data) {
        for (auto const & res : *data_row) {
            if (res.diverged) {
                if (res.iterations > max_iter) max_iter = res.iterations;
                if (res.iterations < min_iter) min_iter = res.iterations;
            }
        }
    }
    auto output_file = FractalFile{clopts.output_file};
    output_file.add_metadata(
            { { clopts.left_top_real, clopts.left_top_img },
                { clopts.right_bottom_real, clopts.right_bottom_img },
                clopts.escape,
                clopts.limit,
                clopts.width,
                clopts.height,
                max_iter,
                min_iter }
            );

    for (auto const & data_row : *data) {
        output_file.write_row(*data_row);
    }

    output_file.finalize();
}

void compute_fractal(fractalator_options const &clopts) {
    std::cout << "Computing fractal\n";

    std::shared_ptr<point_grid> fractal_data;

    auto fp = fractal_params{
                std::complex<double>{ clopts.left_top_real, clopts.left_top_img },
                std::complex<double>{ clopts.right_bottom_real, clopts.right_bottom_img },
                clopts.escape,
                clopts.limit,
                clopts.width,
                clopts.height
            };

    if (clopts.jobs == 0) {
        std::cerr << "Serial computation\n";
        fractal_data = compute_fractal(fp);
    } else {
        std::cerr << "Parallel with " << clopts.jobs << " jobs\n";
        fractal_work_queue wq(clopts.jobs*2);
        fractal_data = compute_fractal(fp, wq, clopts.jobs);
    } 


    write_fractal_file(clopts, fractal_data);
}
