/*
 * This more complicated example uses most of the facilities of the tool. 
 *
 * For diverged points, the iterations are smoothed into a real
 * number. The log of that count is then turned into a fraction
 * between 0 and 1. The fraction is used to index into the color wheel
 * backwards where 0 = pure blue and 1 = pure red.
 *
 * The error function (erf()) is used to shape the response.
 *
 * A piecewise linear function is used to compute the color value.
 *
 * Sample invocation:
 *
 *  build/fractalator -o sample.fract --aspect 3840x2160 --cr 0.25005 --ci 0.00000056 --box 0.0000002 -l 6000
 *  build/colorator -i sample.fract -o sample.bmp -s complicated.as --args 'show_mid_point=false;'
 */

// ---------------------------------------------------------
// Fixed globals

// Number of buckets in the histogram
//
const int BUCKET_COUNT = 100;

// Base scale factor for the error function scale.
//
const double ERF_SCALE_SCALE = 1.0;

// ---------------------------------------------------------
// Arguments settable by the users
// as strings to --args for colorator.
// Currently only bool, int, and double
// are supported by the API.
// 
class args {
    // If true, create a white cross at
    // the center of the image.
    // 
    bool show_mid_point = true;
};

// The actual values we'll use as
// overriden by any passed in values.
args config;

// ---------------------------------------------------------
// Histogram buckets
// 
// limit = the highest iteration that is a part of this bucket
// count = how many diverged points is a part of this bucket
//
class bucket_desc {
    double limit;
    int count;
};

// The actual buckets
//
bucket_desc[] buckets(BUCKET_COUNT);

// The spread of iterations in a single bucket.
//
double bucket_size;


// ---------------------------------------------------------
// Metadata globals.
//
// These will come from the stuff passed in.
//
int limit;
int max_iter;
int min_iter;
int total_rows;
int total_columns;
double escape_radius;

// Calculated values based on the metadata
//
int mid_row_count;
int mid_column_count;
double loglog_escape;
int shifted_max_iter;


void setup(meta_data p, args@ a) {
    config = a;
    logger("config.show_mid_point = " + config.show_mid_point + "\n");

    // copy things out of the meta_data
    //
    limit = p.limit;
    min_iter = p.min_iterations;
    max_iter = p.max_iterations;
    total_rows = p.samples_img;
    total_columns = p.samples_real;
    escape_radius = p.escape_radius;

    mid_row_count = p.samples_img / 2;
    mid_column_count = p.samples_real / 2;
    bucket_size = double(max_iter-min_iter)/double(BUCKET_COUNT);
    loglog_escape = log2(log2(escape_radius));
    
    // c.f. compute_smoothed_count
    //
    shifted_max_iter = max_iter-min_iter+1;

    // initialize the histogram buckets
    //
    for (int i = 0; i < BUCKET_COUNT; ++i) {
        buckets[i].limit = bucket_size*(i+1);
        buckets[i].count = 0;
    }

    // Just to give warm fuzzies that things worked
    //
    logger("min_iter = " + min_iter + "\n");
    logger("max_iter = " + max_iter + "\n");
    logger("mid_row_count    = " + mid_row_count + "\n");
    logger("mid_column_count = " + mid_column_count + "\n");
    logger("bucket_size      = " + bucket_size + "\n");
    logger("escape_radius    = " + escape_radius + "\n");
    logger("shifted_max      = " + shifted_max_iter + "\n");

}


// Compute a histogram. Not sure what we'll do with it but lets just see
// it. 
//
//
int total_diverged;

void prepass(point_data@ r) {
    if (r.diverged) {
        total_diverged += 1;
        int scaled = r.iterations-min_iter;
        int bucket_index = scaled / int(bucket_size+1);
        for (int i = bucket_index; i < BUCKET_COUNT; ++i) {
            if (buckets[i].limit > scaled) {
                buckets[i].count += 1;
                break;
            }
        }
    }

}

// ---------------------------------------------------------
// Analysis of the histogram.
//
// Computed scale factor for the erf() call.
//
double erf_scale;


void precolor() {
    for (int i = 0; i < BUCKET_COUNT; ++i) {
        logger("Bucket " + i + " = " + buckets[i].count + "\n");
    }
    logger("total diverged = " + total_diverged + "\n");

    // Find the bucket that contains the mean.
    // Use the limit associated with that bucket to compute
    // a scaling factor used in the erf() call. This will mean
    // blue to greenish will be used for the first half.
    //
    int bucket_total = 0;
    for (int i = 0; i < BUCKET_COUNT; ++i) {
        bucket_total += buckets[i].count;
        if (bucket_total >= (total_diverged/2)) {
            double limit = int(buckets[i].limit);
            /*
            // Attempt to deal with extremely bottom-heavy distribution.
            // Not entirely successful. Experiment.
            if (i == 0) {
                double percent_of_total = double(bucket_total)/double(total_diverged);
                limit *= 1.0 - percent_of_total;
            }
            */
            logger("Computed limit = " + limit + "\n");

            double scaled = compute_scaled_count(int(limit) + min_iter, escape_radius);

            logger("scaled = " + scaled + "\n");
            erf_scale = ERF_SCALE_SCALE/(2.0 * scaled);
            logger("erf_scale = " + erf_scale + "\n");
            break;
        }
    }
}

//----------------------------------------------------------
// escape parameter
// ---------------------------------------------------------
double compute_scaled_count(int iterations, double l_mod) {
    double smoothed_count = compute_smoothed_count(iterations, l_mod);
    // see below for discussion
    return smoothed_count/double(shifted_max_iter);
}

double compute_smoothed_count(int iterations, double l_mod) {
    //
    // iterations are on the half-open interval [0, limit)
    // The algorithm guarantees that l_mod is between [escape, escape^2)
    // so the log term will go between log2(log2(escape)) and log2(log2(escape))+1
    // The smoothing factor will be from (-1, 0]
    // Final results, then, are on (min_itr-1, max_iter]
    // 
    // A +1 term is added to shift this to (0, max_iter-min_iter+1]
    int scaled_iters = iterations - min_iter + 1;
    return double(scaled_iters) + loglog_escape - log2(log2(l_mod));
}


//----------------------------------------------------------
// HSV value parameter
//----------------------------------------------------------
//
// A piece-wise linear function.
// The input is a number [0, 1).
//
// The outut will rise from 0 at 0 to 1 at PEAK.
// The output will then fall back to LOW at 1.
//
// However, the rise is in two pieces. For the interval [0, LEFT_KNEE)
// it will rise at the rate of KNEE_SLOPE. Between [LEFT_KNEE, PEAK]
// it rises at a rate sufficent to bring it 1 at PEAK.
//

// The value of the input that will result in 1.
const double PEAK = 0.7;

// The value of the input where the first rising segment ends.
const double LEFT_KNEE = 0.5;

// The slope for the first segment.
const double KNEE_SLOPE = 0.5;

// The final value at input = 1
const double LOW = 0.2;

const double KNEE = LEFT_KNEE * KNEE_SLOPE;
const double POSTKNEE_SLOPE = (1.0-KNEE)/(PEAK-LEFT_KNEE);
double left_half(double x) {
    if (x < LEFT_KNEE) {
        return (x * KNEE_SLOPE);
    } else {
        return KNEE + (x-LEFT_KNEE) * POSTKNEE_SLOPE;
    }
}

const double RIGHT_SLOPE = ((1.0-LOW)/(1.0-PEAK));
double right_half(double x) {
    return 1 + (PEAK-x) * RIGHT_SLOPE;
}

double compute_value(double x) {
    if (x > PEAK) {
        return right_half(x);
    } else {
        return left_half(x);
    }
}

// ---------------------------------------------------------
// Lets color things!
// ---------------------------------------------------------
int row = 0;
int column = -1;
color colorize(point_data@ r) {

    column += 1;
    if ( column >= total_columns) {
        column = 0;
        row += 1;
    }

    if ( config.show_mid_point && 
            row < mid_row_count+1 && row > mid_row_count -1 && 
            column < mid_column_count+7 && column > mid_column_count-7 ) {

        // draw the horizontal arms
        return white();

    } else if ( config.show_mid_point && 
            column < mid_column_count+1 && column > mid_column_count-1 && 
            row < mid_row_count+7 && row >mid_row_count-7) {

        // draw the vertical arms
        return white();

    } else if (r.diverged) {

        double scaled_count = compute_scaled_count(r.iterations, r.last_modulus);

        // single sided error function ( grows fast at the start)
        double log_limited_count = erf(scaled_count*erf_scale) ;

        double hue = (4.0f/6.0f) *(1.0 - log_limited_count);
        if (hue < 0.0) hue += 1.0;

        double saturation = 1.0;

        double value = compute_value(log_limited_count);

        return hsv(hue, saturation, value);

    } else {
        return black();
    }
}
