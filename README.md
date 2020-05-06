# mandel
My take on the fractal coloring genre

A set of tools to evaluate and color fractal sets.

## fractalator
A CLI for evaulating point on the complex number plane for fractal set
inclusion. Currently, the only fractal supported is the Mandelbrot set.

fractalator writes an output file with the results of the evaulation. This file
can then be used with the `colorator` utiltiy (see below).

### command line

`fractalator <general arguments> <bounding box arguments> <sampling arguments>`

#### general arguments

<dl>
<dt>-o, --output-file <b>filename</b></dt>
<dd>path to file into which the output will be written. If the file exists, it
will be truncated.</dd>
<dt>-j, --jobs <b>jobcount</b></dt>
<dd>Number of parallel threads use for computing. If `0` is given or if the the
options is not on the command line, the engine will use a strict sequential
code path. If **jobcount** is `1`, the parallel path will be used, but only one
calculating thread will be used. This can actually be slower than `0`. If
**jobcount** > `1`, then that many threads will be used.</dd>
<dt>-l --limit <b>limit</b></dt>
<dd>Integer number of iterations of the fractal formula to use to decide if the
results will diverge or not.</dd>
</dl>

#### bounding box arguments
There are multiple ways to define the part of the complex plane that will be
used.

- Direct - specify the bounding box's top left and bottom right points using
    `--ltr,--lti,--rbr,--rbi`. This is the base case. Internally, all other ways
    to define the bounding box are reduced to this.

- Center - use `--cr,--ci` to specify the center of the box and `--box` to
    specify the length of the sides. Note that the box specified this way will
    necessarily be a square.

- Aspect - Use `--ltr,--lti` to specify the top right of the box, `--box` to
    specify the length of the real axis and `--aspect` to give sampling. In
    this case, the real axis will be the length given by `--box`. The imaginary
    axis will be a scaled length of the ratio of given by aspect.
    `--ltr 1.0 --lti 1.0 --box 0.5 --aspect 1920x1080` is equivalent to `--ltr
    1.0 --lti 1.0 --rbr 1.5 --rbi 0.71875 --width 1920 --height 1080`

<dl>
<dt>--ltr <b>number</b></dt><dd>real coordinate for the left top point of the
bounding box</dd>
<dt>--lti <b>number</b></dt><dd>imaginary coordinate for the left top point of
the bounding box</dd>
<dt>--rbr <b>number</b></dt><dd>real coordinate for the right bottom point of
the bounding box</dd>
<dt>--lti <b>number</b></dt><dd>imaginary coordinate for the right bottom point
of the bounding box</dd>
<dt>--cr <b>number</b></dt><dd>real coordinate for the center of the bounding box</dd>
<dt>--ci <b>number</b></dt><dd>imaginary coordinate for the center of the
bounding box</dd>
<dt>--box <b>number</b></dt><dd>When used with --cr,--ci, this is the length of
the side for bounding box. When used with --aspect, this is the length of the
real axis for the bounding box. </dd>
</dl>

#### sampling arguments

These arguments control the number of points along each axis that will be
sampled by the algorithm.

<dl>
<dd>-s, --samples <b>n</b></dd>
<dt>Number of samples to use on both axis, equivalent to `--width n --height n`</dt>
<dd>-w, --width <b>n</b></dd>
<dt>Number of samples to use on the real axis</dt>
<dd>-h, --height <b>n</b></dd>
<dt>Number of samples to use on the imaginary axis</dt>
<dd>--aspect <b>WxH</b></dd>
<dt>Number of samples to use. Equivalent to `--width W --height H` but also
</dl>
controls the actual height of the bounding box (See above).</dt>
</dl>


## colorator

TBD

## mandel

TBD

## Technologies

- [Cmake](https://cmake.org/) for build configuration.
- [cxxopts](https://github.com/jarro2783/cxxopts) for command line handling.
- [cereal](https://uscilab.github.io/cereal/index.html) for serialization.

## License

MIT &copy; 2018-2019 Mark Hollomon
