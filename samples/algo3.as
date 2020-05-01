/*
 * A simple coloring algorithm based on the HSV color wheel
 *
 * Starting at blue, work around the color wheel N times
 * Avoid the "purple" sector of the color wheel. So modulo things
 * by .6666. Then subtract.
 */
const double N = 6.0;
const double color_wheel_size = 4.0f/6.0f;

int limit;

void setup(fractal_params p) {
    // get the maximum number of iterations.
    limit = p.limit;
}


color colorize(result@ r) {

    if (r.diverged) {
        double smoothed_count = double(r.iterations) + 1.0 + log(log(r.last_modulus));
        double log_limited_count = log(smoothed_count)/log(double(limit+1));

        double hue = color_wheel_size - fmod(N*log_limited_count, color_wheel_size);
        while (hue < 0.0) hue += 1.0;

        double saturation = 1.0;
        double value = log_limited_count;

        return hsv(hue, saturation, value);
    } else {
        return black();
    }
}

