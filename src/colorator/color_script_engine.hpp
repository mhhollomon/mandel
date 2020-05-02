#if !defined(COLOR_SCRIPT_ENGINE_HPP_)
#define COLOR_SCRIPT_ENGINE_HPP_

#include "script_engine.hpp"
#include "pixel.hpp"
#include "meta_data.hpp"
#include "compute.hpp"

class ColorScriptEngine: public ScriptEngine {
    asIScriptFunction* color_func_   = nullptr;
    asIScriptFunction* prepass_func_ = nullptr;
    bool checked_prepass = false;

    virtual void _register_interface(asIScriptEngine* engine) override; 
  public:
    ~ColorScriptEngine();

    bool call_setup(fractal_meta_data *fp);

    pixel call_colorize(check_results &results);

    bool has_prepass();

    void call_prepass(check_results &results);
};

#endif