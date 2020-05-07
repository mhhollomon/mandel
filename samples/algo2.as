/*
 * A simple coloring algorithm based on the HSV color wheel
 *
 *    Starting at blue, work around the color wheel N times
 *    Value is proportional to the count.
 */
const double N = 3.0;

int limit;
int max_iter;
int min_iter;
double escape;
double loglog_escape;

void setup(meta_data p) {
    limit = p.limit;
    min_iter = p.min_iterations;
    max_iter = p.max_iterations;
    escape   = p.escape_radius;
    // This is an optimization so that we are not
    // doing the log2 calls in the tight loop below.
    loglog_escape = log2(log2(escape));

    logger("min = "+ min_iter+ "\n");
    logger("max = "+ max_iter+ "\n");
}

color colorize(point_data@ p) {

    if (p.diverged) {
        int scaled_iters = p.iterations - min_iter + 1;
        // see algo1.as for more notes. This generalizes the equation to use any escape
        // radius
        double smoothed_count = double(scaled_iters) + 
            loglog_escape - log2(log2(p.last_modulus));
        double log_limited_count;
        if (smoothed_count < 1.0) {
           log_limited_count = 0.0;
        } else {
            log_limited_count = log(smoothed_count)/log(double(max_iter-min_iter+1));
        }
        // starting at blue, go around the circle N times
        double hue = (4.0f/6.0f) *(1.0 - N*log_limited_count);
        while (hue < 0.0) hue += 1.0;

        double saturation = 1.0;

        double value = log_limited_count;

        return hsv(hue, saturation, value);
    } else {
        return black();
    }
}

