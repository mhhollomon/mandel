#include "pixel.hpp"

#include <cmath>


std::ostream & operator<<(std::ostream &strm, pixel const & p) {
    strm << "(" << p.red_ << "," << p.green_ << "," << p.blue_ << ")";
    return strm;
}

pixel::pixel( double hue, double saturation, double value) {
    if (hue > 1.0) throw std::runtime_error("hsv hue out of bounds");
    if (saturation > 1.0) throw std::runtime_error("hsv saturation out of bounds");
    if (value > 1.0) throw std::runtime_error("hsv value out of bounds");

    // algorithm from https://www.rapidtables.com/convert/color/hsv-to-rgb.html
    double sector = hue * 6.0f;
    double c = value * saturation;
    double x = c * ( 1 - std::fabs(fmod(sector,2.0) -1.0));
    double m = value - c;

    double rp, gp, bp;
    switch (int(sector)) {
        case 0:
            rp = c; gp = x; bp = 0;
            break;
        case 1:
            rp = x; gp = c; bp = 0;
            break;
        case 2:
            rp = 0; gp = c; bp = x;
            break;
        case 3:
            rp = 0; gp = x; bp = c;
            break;
        case 4 :
            rp = x; gp = 0; bp = c;
            break;
        case 5:
            rp = c; gp = 0; bp = x;
            break;

        default:
            throw std::runtime_error("error in sector calculation");
            break;
    }

    /*
    std::cout << "\n   sector = " << sector
        << " c = " << c
        << " x = " << x
        << " m = " << m
        << "\n   "
        << " rp = " << rp
        << " gp = " << gp
        << " bp = " << bp
        << "\n";
        */


    red_ = int((rp+m) * 255.0);
    green_ = int((gp+m) * 255.0);
    blue_ = int((bp+m) * 255.0);
}
