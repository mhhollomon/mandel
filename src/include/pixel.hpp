#if !defined(MANDEL_PIXEL_HPP_)
#define MANDEL_PIXEL_HPP_

#include <ostream>

struct pixel {
    std::uint32_t red_ = 0;
    std::uint32_t blue_ = 0;
    std::uint32_t green_ = 0;

    pixel() = default;
    pixel(pixel const & p) = default;
    pixel (std::uint8_t red, std::uint8_t blue, std::uint8_t green) noexcept {
        red_   = red;
        blue_  = blue;
        green_ = green;
    }

    pixel (int red, int blue, int green) noexcept {
        red_   = red & 0xff;
        blue_  = blue & 0xff;
        green_ = green & 0xff;
    }

    pixel( double hue, double saturation, double value);

    std::uint32_t to_int() const {

        return (blue_  | green_ << 8 | red_ << 16 );
    }

    friend std::ostream & operator<<(std::ostream &strm, pixel const & p);
        
};


#endif
