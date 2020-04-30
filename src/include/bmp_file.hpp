#if !defined(BMP_FILE_HPP_)
#define BMP_FILE_HPP_

#include "pixel.hpp"

#include <fstream>
#include <vector>


class BMPFile {
    std::string file_name_;
    int height_;
    int width_;
    std::ofstream *out_stream_ = nullptr;
    bool initialized_ = false;
    int current_row_ = 0;

  public:

    BMPFile(std::string file_name, int height, int width) noexcept {
        file_name_ = file_name;
        height_ = height;
        width_ = width;
    }

    ~BMPFile() {
        if (out_stream_) {
            out_stream_->close();
            delete out_stream_;
            out_stream_ = nullptr;
        }
    }

    void initialize_file();
    void write_row(std::vector<pixel> const & data);

  private:

    void _write_file_header();
    void _write_image_header();
};

#endif
