/*
 * A simple coloring algorithm based on the HSV color wheel
 *
 *    Starting at blue, work around the color wheel N times
 *    Value is proportional to the count.
 */
const double N = 3.0;

int limit;

void setup(fractal_params p) {
    // get the maximum number of iterations.
    limit = p.limit;
}


color colorize(result@ r) {

    if (r.diverged) {
        double smoothed_count = double(r.iterations) + 1.0 + log(log(r.last_modulus));
        double log_limited_count = log(smoothed_count)/log(double(limit+1));
        double hue = (4.0f/6.0f) *(1.0 - N*log_limited_count);
        while (hue < 0.0) hue += 1.0;
        double saturation = 1.0;
        double value = log_limited_count;

        return hsv(hue, saturation, value);
    } else {
        return black();
    }
}

