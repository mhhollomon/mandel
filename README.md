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
<dt>-h, --help</dt>
<dd>Print out short command line help text<dd>
<dt>-o, --output-file &lt;filename&gt;</dt>
<dd>path to file into which the output will be written. If the file exists, it
will be truncated.</dd>
<dt>-j, --jobs &lt;jobcount&gt;</dt>
<dd>Number of parallel threads use for computing. If <code>0</code> is given or
if the the options is not on the command line, the engine will use a strict
sequential code path. If <b>&lt;jobcount&gt;</b> is <code>1</code>, the parallel
path will be used, but only one calculating thread will be used. This can
actually be slower than <code>0</code>. If <b>&lt;jobcount&gt;</b> is greater
than <code>1</code>, then that many threads will be used.</dd> 
<dt>-l --limit &lt;limit&gt;</dt>
<dd>Integer number of iterations of the fractal formula to use to decide if the
results will diverge or not.</dd>
<dt>-e, --escape &lt;radius&gt;</dt>
<dd>Value to use to test if the point has diverged. Default is 256.0</dd>
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
<dt>--ltr &lt;number&gt;</dt><dd>real coordinate for the left top point of the
bounding box</dd>
<dt>--lti &lt;number&gt;</dt><dd>imaginary coordinate for the left top point of
the bounding box</dd>
<dt>--rbr &lt;number&gt;</dt><dd>real coordinate for the right bottom point of
the bounding box</dd>
<dt>--lti &lt;number&gt;</dt><dd>imaginary coordinate for the right bottom point
of the bounding box</dd>
<dt>--cr &lt;number&gt;</dt><dd>real coordinate for the center of the bounding box</dd>
<dt>--ci &lt;number&gt;</dt><dd>imaginary coordinate for the center of the
bounding box</dd>
<dt>--box &lt;number&gt;</dt><dd>When used with <code>--cr,--ci</code>, this is the length of
the side for bounding box. When used with <code>--aspect</code>, this is the length of the
real axis for the bounding box. </dd>
</dl>

#### sampling arguments

These arguments control the number of points along each axis that will be
sampled by the algorithm.

<dl>
<dt>-s, --samples &lt;n&gt;</dt>
<dd>Number of samples to use on both axis, equivalent to <code>--width n --height n</code></dd>
<dt>--width &lt;n&gt;</dt>
<dd>Number of samples to use on the real axis</dd>
<dt>--height &lt;n&gt;</dt>
<dd>Number of samples to use on the imaginary axis</dd>
<dt>--aspect &lt;WxH&gt;</dt>
<dd>Number of samples to use. Equivalent to <code>--width W --height H</code> but also
controls the actual height of the bounding box (See above).</dd>
</dl>


## colorator

Applies a user supplied coloring algorithm to the input fractal data to create a
.bmp mage file.

The coloring algorithm is written in
[angelscript](http://www.angelcode.com/angelscript/sdk/docs/manual/doc_script.html).

See the docs directory for further details on the interface. See the samples
directory for examples.

### command line

`colorator [-h] -o <output> -i <input> -s <script>`

<dl>
<dt>-h, --help </dt>
<dd>Print out short command line help text<dd>
<dt>-o, --output-file &lt;filename&gt;</dt>
<dd>path to file into which the output will be written. If the file exists, it
will be truncated.</dd>
<dt>-i, --input-file &lt;filename&gt;</dt>
<dd>Path to .fract file on to which to apply coloring. It assumes the file
format is that put out by fractalator.</dd>
<dt>-s, --script-file &lt;filename&gt;</dt>
<dd>Path to the angelscript file containing coloring algorithm.</dd>
</dl>

## mandel

Future intelligent combination of fractalator and colorator.

## gmandel

Hypothetical future version of mandel but with a gui interface.

## Technologies

- [Cmake](https://cmake.org/) for build configuration.
- [cxxopts](https://github.com/jarro2783/cxxopts) for command line handling.
- [cereal](https://uscilab.github.io/cereal/index.html) for serialization.
- [angelscript](http://www.angelcode.com/angelscript/sdk/docs/manual/doc_script.html) for the user scripting support.

## License

MIT &copy; 2020 Mark Hollomon
