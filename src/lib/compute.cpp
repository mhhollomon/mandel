#include "compute.hpp"

#include <cmath>
#include <iostream>
#include <list>

fractal_point_data mandelbrot_test(std::complex<double> test_point, int limit) {

    fractal_point_data retval;
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

void compute_slice(work_item wi) {
    for (int index = wi.start_index; index < wi.end_index; ++index) {
        double real_double = wi.base_real + (wi.real_increment * index);
        (*wi.output)[index] = mandelbrot_test({real_double, wi.base_img}, wi.limit);
    }
}

std::shared_ptr<fixed_array<result_slice>> compute_fractal(fractal_params p) {
    auto retval = std::make_shared<fixed_array<result_slice>>(p.samples_img);

    double real_increment = (p.bb_bottom_right.real() - p.bb_top_left.real()) / p.samples_real;
    double img_increment = (p.bb_top_left.imag() - p.bb_bottom_right.imag()) / p.samples_img;

    double base_real = p.bb_top_left.real();

    for (int row = 0; row < p.samples_img; ++row) {
        if (row % 100 == 0)
            std::cout << "----------------- starting row = " << row << " ---\n";
        
        // TODO - this breaks encapsualtion of the type
        auto rs = std::make_shared<fixed_array<fractal_point_data> >(p.samples_real);

        double base_img = p.bb_bottom_right.imag() + ( img_increment * row );

        compute_slice({rs, row, p.limit, 0, p.samples_real, 
                base_img, base_real, real_increment});

        (*retval)[row] = rs;
    }

    return retval;
}

/*---------------------------------------------
 * parallel land
 *---------------------------------------------*/

// Producer - ques up work items
void producer(fractal_params p, fractal_work_queue &wq, 
        std::shared_ptr<fixed_array<result_slice>> &data_array) {
    double real_increment = (p.bb_bottom_right.real() - p.bb_top_left.real()) / p.samples_real;
    double img_increment = (p.bb_top_left.imag() - p.bb_bottom_right.imag()) / p.samples_img;
    double base_real = p.bb_top_left.real();

    for (int row = 0; row < p.samples_img; ++row) {
        (*data_array)[row] = 
            std::make_shared<fixed_array<fractal_point_data> >(p.samples_real);

        double base_img = p.bb_bottom_right.imag() + ( img_increment * row );

        work_item wi = {(*data_array)[row], row, p.limit, 0, p.samples_real,
            base_img, base_real, real_increment};

        wq.add_work(wi);

        if (row % 100 == 0)
            std::cerr << "Producer queued : " << row << "\n";
    }

    wq.set_all_done();
}

// consumer - grabs work
void consumer(int id, fractal_work_queue &wq) {
    std::cerr << "Consumer " << id << " starting\n";
    while(true) {
        auto [ is_valid, wi ] = wq.get_work();
        if (not is_valid) {
            std::cerr << "Consumer " << id << " shutting down\n";
            return;
        }
        //std::cerr << "Consumer " << id << " working on " << wi.row_number << "\n";

        compute_slice(wi);
    }
}



/* --------------------------------------------
 * main routine
 * --------------------------------------------*/

std::shared_ptr<fixed_array<result_slice>> compute_fractal(fractal_params p, 
                        fractal_work_queue &wq, int jobs) {

    // Toplevel array of arrays
    auto retval = std::make_shared<fixed_array<result_slice>>(p.samples_img);

    std::thread pd(producer,p, std::ref(wq), std::ref(retval));

    std::list<std::thread> consumers;

    for (int i = 0; i < jobs; ++i) {
        consumers.emplace_back(consumer, i, std::ref(wq));
    }

    pd.join();

    for( auto &c : consumers) {
        c.join();
    }

    return retval;
}
