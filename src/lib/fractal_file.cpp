#include "fractal_file.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/complex.hpp>

const unsigned SIGNATURE = 0x41434652;
const unsigned VERSION   = 0x00010001;


template<class Archive> void serialize(Archive & archive,
               fractal_meta_data & fmd)
{
    archive( fmd.bb_top_left, fmd.bb_bottom_right, fmd.escape_radius, fmd.limit,
            fmd.samples_real, fmd.samples_img,
            fmd.max_iterations, fmd.min_iterations);
}

template<class Archive> void serialize(Archive & archive,
               fractal_point_data & fpd)
{
    archive(fpd.last_value, fpd.last_modulus, fpd.iterations, fpd.diverged);
}

void FractalFile::add_metadata( fractal_meta_data const &fmd ) {

    if (has_meta_) {
        throw std::runtime_error("Metadata already added");
    }

    metadata_ = fmd;

    fstrm_.exceptions(std::ofstream::failbit | std::ofstream::badbit );
    fstrm_.open(file_name_, 
            std::ios::out|std::ofstream::trunc|std::ofstream::binary);

    fstrm_.write(reinterpret_cast<const char*>(&SIGNATURE), sizeof(SIGNATURE));
    fstrm_.write(reinterpret_cast<const char*>(&VERSION), sizeof(VERSION));

    cereal::BinaryOutputArchive oarchive(fstrm_);

    oarchive(fmd);

    has_meta_ = true;

}

void FractalFile::write_row(point_row const &rs) {
    if (not has_meta_)
       throw std::runtime_error("Must add_metadata() before write_row()");

    auto expected_rows = metadata_.samples_img;

    if (row_count_ >= expected_rows)
        throw std::runtime_error("Already have all the expected rows\n");

    auto expected_columns =  metadata_.samples_real;
    if (rs.size() != expected_columns)
        throw std::runtime_error("Incorrect number of results in vector\n");

    row_count_ += 1;

    cereal::BinaryOutputArchive oarchive(fstrm_);

    for (auto const &fpd : rs) {
        oarchive(fpd);
    }
}


void FractalFile::finalize() {
    if (not has_meta_)
       throw std::runtime_error("Must add_metadata() before write_row()");

    auto expected_rows = metadata_.samples_img;

    if (row_count_ != expected_rows)
        throw std::runtime_error("incorrect numberof rows.\n");


    fstrm_.close();
}

std::unique_ptr<FractalFile> FractalFile::read_from_file(std::string file_name) {

    auto retval = std::make_unique<FractalFile>(file_name);

    retval->read_data();

    return retval;
}

fractal_meta_data 
FractalFile::read_meta_data_from_file(std::string file_name) {
    FractalFile ff(file_name);

    ff.read_meta_data();

    return ff.get_meta_data();

}

void FractalFile::read_meta_data() {
    fstrm_.open(file_name_,
            std::ios::in | std::ios::binary);

    char buffer[sizeof(SIGNATURE)+sizeof(VERSION)+1];

    fstrm_.read(buffer, sizeof(SIGNATURE));
    if (fstrm_.gcount() != sizeof(SIGNATURE))
       throw std::runtime_error("Could not read file sig");
    if  ( *(reinterpret_cast<unsigned*>(buffer)) != SIGNATURE) 
        throw std::runtime_error("Sig does not match");
    fstrm_.read(buffer, sizeof(VERSION));


    cereal::BinaryInputArchive iarchive(fstrm_);
    iarchive(metadata_);
    has_meta_ = true;
}

void FractalFile::read_data() {

    read_meta_data();

    int expected_rows = metadata_.samples_img;
    int expected_cols = metadata_.samples_real;

    rows_ = std::make_shared<point_grid>(expected_rows);

    for (int i = 0; i < expected_rows; ++i) {
        auto new_row = std::make_shared<point_row>(expected_cols);
        for (int j = 0; j < expected_cols; ++j) {
            iarchive((*new_row)[j]);
        }

        (*rows_)[i] = new_row;

    }

    fstrm_.close();

}

fractal_meta_data FractalFile::get_meta_data() const {
    if (not has_meta_)
        throw std::runtime_error("No meta data is available\n");

    return metadata_;
}
