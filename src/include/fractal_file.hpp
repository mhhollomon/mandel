#if !defined(FRACTAL_FILE_HPP_)
#define FRACTAL_FILE_HPP_

#include "fractal.pb.h"
#include "fractal_data.hpp"

#include "fixed_array.hpp"

#include <string>
#include <fstream>
#include <memory>


class FractalFile {
  private:
    std::string file_name_;
    std::unique_ptr<fractal::File> pb_;

  public:
    FractalFile(std::string file_name) noexcept : file_name_{file_name} {};
    ~FractalFile() = default;

    void add_metadata( std::complex<double> bb_tl,
            std::complex<double> bb_br,
            int limit,
            int samples_real,
            int samples_img,
            int max_iter,
            int min_iter);

    void write_row(fixed_array<fractal_point_data> const& rs);

    void finalize();

    static std::unique_ptr<FractalFile> read_from_file(std::string file_name);
    
    fractal_meta_data get_header_info() const;
    auto get_rows() const { return pb_->rows(); }

  private:
    void read_data();

};

#endif
