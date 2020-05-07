/*
 * A simple coloring algorithm based on the HSV color wheel
 *
 * For diverged points, the iterations are smoothed into a real
 * number. The log of that count is then truend into a fraction
 * between 0 and 1. The fraction is used to index into the color wheel
 * backwards so that 0 = pure blue and 1 = pure red.
 *
 * The color value is set to that fraction so that low counts are dark
 * and high counts are bright.
 */
int limit;
int max_iter;
int min_iter;

void setup(meta_data p) {
    limit = p.limit;
    min_iter = p.min_iterations;
    max_iter = p.max_iterations;

    logger("min = "+ min_iter+ "\n");
    logger("max = "+ max_iter+ "\n");
}

// This algorithm doesn't need a prepass.
// Just as an example, compute the average iterations
double iter_sum;
double point_count;
double iter_avg;
void prepass(point_data@ p) {
    // most of the other data isn't valid unless
    // diverged is true
    if (p.diverged) {
        iter_sum += p.iterations;
        point_count += 1;
    }
}

// Again, the algorithm doesn't need this
// but show how to use precolor to finish the 
// statistics from above
void precolor() {
    iter_avg = iter_sum/point_count;

    // use the logger
    logger("Average iterations = " + iter_avg + "\n");
}



// This is the function that actually does the coloring.
color colorize(point_data@ p) {

    if (p.diverged) {
        int scaled_iters = p.iterations - min_iter + 1;

        // extend the integer iteration count to a real.
        // c.f. https://www.iquilezles.org/www/articles/mset_smooth/mset_smooth.htm
        // for a decent writeup on this technique.
        // Assumes the escape radius is the default 256.
        double smoothed_count = double(scaled_iters) + 3.0 - log2(log2(p.last_modulus));
        double log_limited_count;
        if (smoothed_count < 1.0) {
           log_limited_count = 0.0;
        } else {
            log_limited_count = log(smoothed_count)/log(double(max_iter-min_iter+1));
        }

        // calculate the hue. Blue is at 240 degrees (4/6 of the way around the wheel).
        // Red is at 0.
        double hue = (4.0f/6.0f) *(1.0 - log_limited_count);
        // shouldn't be needed - but be safe.
        if (hue < 0.0) hue += 1.0;

        // hard code the saturation. Certainly something to play with
        // to make this a function of the count as well.
        double saturation = 1.0;

        // Calculate the value
        double value = log_limited_count;

        // Return the chosen color using the hsv() constructor
        return hsv(hue, saturation, value);
    } else {
        // If it didn;t diverge (and thus is a part of the set)
        // paint it black.
        return black();
    }
}

