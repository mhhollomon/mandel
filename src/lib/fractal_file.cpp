#include "fractal_file.hpp"

void FractalFile::add_metadata( std::complex<double> bb_tl,
            std::complex<double> bb_br,
            int limit,
            int samples_real,
            int samples_img,
            int max_iterations,
            int min_iterations) {

    if (pb_) {
        throw std::runtime_error("Metadata already added");
    }

    pb_ = std::make_unique<fractal::File>();

    auto *hdr = pb_->mutable_header();
    hdr->set_signature(0x46520100);

    auto *bb = hdr->mutable_bb_tl();
    bb->set_real(bb_tl.real());
    bb->set_img(bb_tl.imag());

    bb = hdr->mutable_bb_br();
    bb->set_real(bb_br.real());
    bb->set_img(bb_br.imag());

    hdr->set_limit(limit);
    hdr->set_real_samples(samples_real);
    hdr->set_img_samples(samples_img);
    hdr->set_max_iterations(max_iterations);
    hdr->set_min_iterations(min_iterations);
}

void FractalFile::write_row(fixed_array<fractal_point_data> const &rs) {
    if (not pb_)
       throw std::runtime_error("Must add_metadata() before write_row()");

    auto expected_rows = pb_->header().img_samples();
    auto current_count = pb_->rows_size();

    if (current_count >= expected_rows)
        throw std::runtime_error("Already have all the expected rows\n");

    auto expected_columns =  pb_->header().real_samples();
    if (rs.size() != expected_columns)
        throw std::runtime_error("Incorrect number of results in vector\n");

    auto row = pb_->add_rows();
    for (auto const & res : rs) {
        auto *msg = row->add_results();
        auto *lv = msg->mutable_last_value();
        lv->set_real(res.last_value.real());
        lv->set_img(res.last_value.imag());
        msg->set_last_modulus(res.last_modulus);
        msg->set_iterations(res.iterations);
        msg->set_diverged(res.diverged);
    }

}


void FractalFile::finalize() {
    if (not pb_)
       throw std::runtime_error("Must add_metadata() before write_row()");

    auto expected_rows = pb_->header().img_samples();
    auto current_count = pb_->rows_size();

    if (current_count != expected_rows)
        throw std::runtime_error("incorrect numberof rows - nothing written\n");

    std::fstream output(file_name_, 
            std::ios::out | std::ios::trunc | std::ios::binary);

    pb_->SerializeToOstream(&output);
}

std::unique_ptr<FractalFile> FractalFile::read_from_file(std::string file_name) {

    auto retval = std::make_unique<FractalFile>(file_name);

    retval->read_data();

    return retval;
}

void FractalFile::read_data() {
    std::fstream input (file_name_,
            std::ios::in | std::ios::binary);

    pb_ = std::make_unique<fractal::File>();
    pb_->ParseFromIstream(&input);
}

fractal_meta_data FractalFile::get_header_info() const {
    auto hdr = pb_->header();

    fractal_meta_data retval;

    retval.bb_top_left = { hdr.bb_tl().real(), hdr.bb_tl().img() };
    retval.bb_bottom_right =  { hdr.bb_br().real(), hdr.bb_br().img() };
    retval.limit =  hdr.limit();
    retval.samples_real = hdr.real_samples();
    retval.samples_img  = hdr.img_samples();
    retval.max_iterations = hdr.max_iterations();
    retval.min_iterations = hdr.min_iterations();

    return retval;
}
