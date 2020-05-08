#if !defined(COLOR_SCRIPT_ENGINE_HPP_)
#define COLOR_SCRIPT_ENGINE_HPP_

#include "script_engine.hpp"
#include "pixel.hpp"
#include "fractal_data.hpp"

#include <string>

class ColorScriptEngine: public ScriptEngine {
    asIScriptFunction* color_func_   = nullptr;
    asIScriptFunction* prepass_func_ = nullptr;
    bool checked_prepass = false;

    virtual void _register_interface(asIScriptEngine* engine) override; 
  public:
    ~ColorScriptEngine();

    bool call_setup(fractal_meta_data *fp, std::string arg_string);

    pixel call_colorize(fractal_point_data &results);

    bool has_prepass();

    void call_prepass(fractal_point_data &results);

    void call_precolor();

  private:
    void* parse_args(std::string arg_string);
};

#endif
