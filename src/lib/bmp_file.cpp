#include "bmp_file.hpp"

#include <iostream>
#include <cmath>


void BMPFile::initialize_file() {

    if (initialized_) return;

    out_stream_ = new std::ofstream{};
    // throw an exception for most errors
    out_stream_->exceptions(std::ofstream::failbit | std::ofstream::badbit );
    out_stream_->open(file_name_, std::ios::out|std::ofstream::trunc|std::ofstream::binary);
    if (! out_stream_->is_open()) {
        throw std::runtime_error("Not open but no exception");
    }

    _write_file_header();
    _write_image_header();


    initialized_ = true;
}

void BMPFile::write_row(std::vector<pixel> const & data) {
        initialize_file();

    if (current_row_ >= height_) {
        throw std::runtime_error("Writing more rows than allowed by the height");
    }

    if (data.size() != (unsigned)width_) {
        std::cerr << "Width_ = " << width_ << " data = " << data.size() << "\n";
        throw std::runtime_error("row size is incorrect for width");
    }

    current_row_ += 1;
    // write these out using the iterator, but perhaps we could
    // do it in one shot since vector is guaranteed to be contiguous.
    // research for later.
    for (auto const p : data) {
        auto word = p.to_int();
        out_stream_->write(reinterpret_cast<const char *>(&word), sizeof(word));
        if (! out_stream_->good()) {
            throw std::runtime_error("write failed");
        }
    }
}


struct file_header {
    std::uint16_t signature = 0x4D42;
    std::uint32_t file_size = 0;
    std::uint16_t reserve1 = 0;
    std::uint16_t reserve2 = 0;
    std::uint32_t data_offset = 0;
} __attribute__((packed));

struct image_header {
    std::uint32_t header_size = 40;
    std::int32_t width = 0;
    std::int32_t height = 0;
    std::uint16_t planes = 1;
    std::uint16_t bpp = 32;
    std::uint32_t compression_method = 0; // no compression
    std::uint32_t image_size = 0;
    std::int32_t horiz_resln = 0;
    std::int32_t vert_resln = 0;
    std::uint32_t colors = 0; // 0 = 2^bpp
    std::uint32_t important_colors = 0; // generally ignored
}__attribute__((packed));

void BMPFile::_write_file_header() {
    file_header fh;

    fh.data_offset = sizeof(file_header) + sizeof(image_header);

    fh.file_size = fh.data_offset + (height_ * width_ * 4);

    out_stream_->write(reinterpret_cast<char*>(&fh), sizeof(fh));

}


void BMPFile::_write_image_header() {
    image_header ih;

    ih.width = width_;
    ih.height = height_;

    out_stream_->write(reinterpret_cast<const char *>(&ih), sizeof(ih));
}

