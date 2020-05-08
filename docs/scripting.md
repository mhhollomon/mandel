`colorator` uses
[angelscript](http://www.angelcode.com/angelscript/sdk/docs/manual/doc_script.html)
for scripting. This document is about the provided and required interfaces.


## Provided Interfaces


### Standard Angelscript add ons

The following add ons that are a part of the angelscript distribution are
provided as-is:

- [strings](http://www.angelcode.com/angelscript/sdk/docs/manual/doc_script_stdlib_string.html)
- [arrays](http://www.angelcode.com/angelscript/sdk/docs/manual/doc_datatypes_arrays.html)
    In addition, the array syntax `int[] foo(3)` to define an array of 3
    elements is also supported.
- complex numbers.
- [standard math
    functions](http://www.angelcode.com/angelscript/sdk/docs/manual/doc_addon_math.html)


### Application Specific Classes

#### meta_data
The `meta_data` class includes information that is global to the fractal -
iteration limit, escape radius, bounding box, etc. It is defined as follows:
~~~cpp
class meta_data {
    complex bb_top_left; // top left point of the bounding box
    complex bb_bottom_right; // bottom right point of the bounding box
    double escape_radius; // test value to use to detect divergence
    int limit;            // Limit on the number of iterations to test
    int samples_real;     // Number of samples (pixel) along the real (x) axis
    int samples_img;      // Number of samples along the imaginary (y) axis
    int max_iterations;   // Highest number of iterations actually seen
    int min_iterations;   // Lowest number of iterations actually seen
}
~~~

#### point_data

This class is the data associated with a particular point on the complex plane.

~~~cpp
class point_data {
    bool    diverged = false;    // did the computation diverge or not?
                                 // The other values are not quaranteed to be
                                 // meaningful if this is false.
    complex last_value = 0.0;    // value of the function at divergence.
    double  last_modulus = 0.0;  // modulus (vector length) of last_value.
    int     iterations = 0;      // number of iterations before crossing over
                                 // escape_radius
}
~~~

### color
Represents a color. The class itself should be considered opaque. The following
constructors are provided.

- `color white()` - create a color object representing white.
- `color black()` - create a color object representing black.
- `color rgb(int red, int green, int blue)`
    create a color object with the given RGB values. The values must be between
    0 and 255.
- `color hsv(double hue, double saturation, double value)`
    create a color object with the given HSV values. All arguments must be
    between 0 and 1.

### Additional math functions

In addition to the angelscript provide math functions, the following are also
available

- `double fmod(double, double)`
- `double log2(double)`
- `double erf(double)`

### Other functions

- `logger(string)` - writes the string to standard out.

## Script Structure

`colorator` will attempt to call the following functions in the given order.
Those marked as "required" must be present in the script. The others will be
called if given, but will not cause and error if they are not.

See the samples directory for commented examples.

### setup() (optional)
The first function called and the only chance the script has to get the
`meta_data`.

There are two possible signatures for the setup function

#### `void setup(meta_data)`

#### `void setup(meta_data, args@)`

The second form is the one preferred by the system. If it is available, it will
be called. See section ["Argument Passing"](#argument-passing)


### `void prepass(point_data@)` (optional)
`prepass` will be called with the same data (in the same order) as `colorize`.
This function is useful for gathering statistics or other global calculations on
the fractal data.

### `void precolor()` (optional)
Called after prepass is complete, this function allso the user to analyze any
data collected by `prepass` in order to possibly set parameters for the
coloring pass.

### `color colorize(point_data@)` (required)
The actuall coloring algorithm. This function is expected to return a color
object for each point passed.

### Argument Passing

It is possible to pass arguments from the colorator command line into the
script. To do so, you must do the following:

#### define an "args" class

1. Define a `class args` 
   (the name of the class must be exactly "args") to hold
   the data. Initializers should be used to set default values. Unlike C++,
   Angelscript initializes all variables. 

   Currently only `int`, `dobule` and `bool` members are supported.

~~~cpp
class args {
    double color_offset;
    bool   turn_off_blending = false;
    int    histogram_buckets = 10;
}
~~~

2. Use the two parameter form of the `setup()` function.
   ~~~cpp
   void setup(meta_data m, args@ a) {
        // yada
   }
   ~~~

3. Pass the arguments in the command line
   `colorator -i in.fact -o out.bmp -s myscript --args 'color_offset=3.2;histogram_buckets=100;turn_off_blending=true`


Class members that are not mentioned on the command line will retain the
default value - either the system default, or the default initializer.

If a key in the args list is not a member of the class, it will be silently
ignored. If a member is in the rgs list but the member is not of recognized
type, awarning will be issued and the key/value pair ignored.
