/*
 * A simple coloring algorithm based on the HSV color wheel
 *
 * For diverged points, the iterations are smoothed into a reall
 * number. The log of that count is then truend into a fraction
 * between 0 and 1. The fraction is usedto index into the color wheel
 * backwards where 0 = pure blue and 1 = pure red.
 *
 * The color value is set to that fraction so that low counts are dark
 * and high counts are bright.
 */
int limit;

void setup(fractal_params p) {
    limit = p.limit;
}


color colorize(result@ r) {

    if (r.diverged) {
        double smoothed_count = double(r.iterations) + 1.0 + log(log(r.last_modulus));
        double log_limited_count = log(smoothed_count)/log(double(limit+1));
        double hue = (4.0f/6.0f) *(1.0 - log_limited_count);
        if (hue < 0.0) hue += 1.0;
        double saturation = 1.0;
        double value = log_limited_count;

        return hsv(hue, saturation, value);
    } else {
        return black();
    }
}

