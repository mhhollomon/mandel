# mandel
My take on the fractal coloring genre

A set of tools to evaluate and color fractal sets.

## fractalator
A CLI for evaulating point on the complex number plane for fractal set
inclusion. Currently, the only fractal supported is the Mandelbrot set.

fractalator writes an output file with the results of the evaulation. This file
can then be used with the `colorator` utiltiy (see below).

### command line

`fractalator -o <output file> -j <jobs> -l <limit> <bounding box arguments>`

#### general arguments

<dl>
<dt>-o, --output-file ~~filename~~</dt>
<dd>path to file into which the output will be written. IF the file exists, it
will be truncated.</dd>
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
