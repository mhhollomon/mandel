#if !defined(FRACTAL_FILE_HPP_)
#define FRACTAL_FILE_HPP_

#include "fractal_data.hpp"

#include "fixed_array.hpp"

#include <string>
#include <fstream>


class FractalFile {
  private:
    std::string file_name_;
    fractal_meta_data metadata_;
    bool has_meta_ = false;
    std::shared_ptr<point_grid> rows_;
    std::fstream fstrm_;
    int row_count_ = 0;

  public:
    FractalFile(std::string file_name) noexcept : file_name_{file_name} {};
    ~FractalFile() = default;

    void add_metadata( fractal_meta_data const &fmd );

    void write_row(point_row const& rs);

    void finalize();

    static std::unique_ptr<FractalFile> read_from_file(std::string file_name);
    static fractal_meta_data read_meta_data_from_file(std::string file_name);
    
    fractal_meta_data get_meta_data() const;
    std::shared_ptr<point_grid>  const & get_rows() const { return rows_; }

  private:
    void read_meta_data();
    void read_data();

};

#endif
