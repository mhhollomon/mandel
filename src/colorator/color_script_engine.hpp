#if !defined(COLOR_SCRIPT_ENGINE_HPP_)
#define COLOR_SCRIPT_ENGINE_HPP_

#include "script_engine.hpp"
#include "compute.hpp"
#include "pixel.hpp"

class ColorScriptEngine: public ScriptEngine {
    asIScriptFunction* color_func_ = nullptr;

    virtual void _register_interface(asIScriptEngine* engine) override; 
  public:

    bool call_setup(fractal_params *fp);

    pixel call_colorize(check_results &results);
};

#endif
