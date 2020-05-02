#include "color_script_engine.hpp"

#include "pixel.hpp"

#include <iostream>
#include <cassert>



ColorScriptEngine::~ColorScriptEngine() {
    if (color_func_) color_func_->Release();
    if (prepass_func_) prepass_func_->Release();
}

bool ColorScriptEngine::has_prepass() {
    if (checked_prepass) {
        return prepass_func_ != nullptr;
    }

    prepass_func_ = find_function("void prepass(result@)");
    checked_prepass = true;
    if (prepass_func_)
        prepass_func_->AddRef();

    return prepass_func_ != nullptr;
}



void fractal_params_new(void *memory) {
  // Initialize the pre-allocated memory by calling the
  // object constructor with the placement-new operator
  new(memory) fractal_meta_data();
}
 
void fractal_params_del(void *memory) {
  // Uninitialize the memory by calling the object destructor
  ((fractal_meta_data*)memory)->~fractal_meta_data();
}

struct script_result : public check_results {
    int refcnt = 1;

    script_result() = default;
    script_result( check_results const & cr) : check_results(cr)
    {}

    static script_result *Create() {
        return new script_result();
    }

    void Release() {
        if (--refcnt <= 0)
            delete this;
    }

    void AddRef() {
        ++refcnt;
    }
};

pixel script_white() {
    return pixel(255,255,255);
}

pixel script_black() {
    return pixel(0,0,0);
}

pixel script_hsv(double h, double s, double v) {
    return pixel(h,s,v);
}

double script_fmod(double a, double b) {
    // shim to make sure the compiler
    // gets the correct version
    return std::fmod(a,b);
}

double script_erf(double a) {
    // shim to make sure the compiler
    // gets the correct version
    return std::erf(a);
}


bool ColorScriptEngine::call_setup(fractal_meta_data *fp) {


    auto *setup_func = find_function("void setup(fractal_params)");

    if (setup_func) {
        auto ctx = prepare_context("void setup(fractal_params)");
        ctx->SetArgObject(0, fp);

        return execute_context();
    } else {
        return true;
    }
}

pixel ColorScriptEngine::call_colorize(check_results &result) {
    if (not color_func_) {
        color_func_ = find_function("color colorize(result@)");
        if (color_func_) {
            // we are storing so be sure to take a reference
            color_func_->AddRef();
        } else {
            throw std::runtime_error("Could not find colorize() function");
        }
    }

    script_result *r = new script_result(result);

    auto ctx = prepare_context(color_func_);
    ctx->SetArgObject(0, &result);

    if (execute_context()) {
        r->Release();
        return *(pixel *)ctx->GetReturnObject();
    } else {
        throw std::runtime_error("colorize call failed");
    }
}

void ColorScriptEngine::call_prepass(check_results &results) {
    if (not has_prepass()) return;

    auto ctx = prepare_context(prepass_func_);
    ctx->SetArgObject(0, &results);
    if ( not execute_context() ) {
        throw std::runtime_error("Call to prepass() failed");
    }
}

void ColorScriptEngine::_register_interface(asIScriptEngine * engine) {
    int r;
    
    // fractal_params
    r = engine->RegisterObjectType("fractal_params", sizeof(fractal_meta_data), 
            asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<fractal_meta_data>()); 
    assert( r >= 0 );
    /*
    r = engine->RegisterObjectBehaviour("fractal_params", asBEHAVE_CONSTRUCT,
            "void f()", asFUNCTION(fractal_params_new), 
            asCALL_CDECL_OBJLAST); 
    assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("fractal_params", asBEHAVE_DESTRUCT, 
            "void f()", asFUNCTION(fractal_params_del), 
            asCALL_CDECL_OBJLAST); 
    assert( r >= 0 );
    */

    r = engine->RegisterObjectProperty("fractal_params", "complex bb_tl",
            asOFFSET(fractal_meta_data,bb_top_left));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("fractal_params", "complex bb_br",
            asOFFSET(fractal_meta_data,bb_bottom_right));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("fractal_params", "int limit",
            asOFFSET(fractal_meta_data,limit));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("fractal_params", "int samples_real",
            asOFFSET(fractal_meta_data,samples_real));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("fractal_params", "int samples_img",
            asOFFSET(fractal_meta_data,samples_img));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("fractal_params", "int max_iterations",
            asOFFSET(fractal_meta_data,max_iterations));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("fractal_params", "int min_iterations",
            asOFFSET(fractal_meta_data,min_iterations));
    assert( r >= 0 );

    // result
    r = engine->RegisterObjectType("result", 0, asOBJ_REF); 
    assert( r >= 0 );

    // TODO: Maybe don't let script create?
    r = engine->RegisterObjectBehaviour("result", asBEHAVE_FACTORY, 
            "result@ f()", asFUNCTION(script_result::Create), asCALL_CDECL); 
    assert( r >= 0 );

    r = engine->RegisterObjectBehaviour("result", asBEHAVE_ADDREF, 
            "void f()", asMETHOD(script_result,AddRef), asCALL_THISCALL); 
    assert( r >= 0 );

    r = engine->RegisterObjectBehaviour("result", asBEHAVE_RELEASE, 
            "void f()", asMETHOD(script_result,Release), asCALL_THISCALL); 
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("result", "complex last_value",
            asOFFSET(script_result,last_value));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("result", "double last_modulus",
            asOFFSET(script_result,last_modulus));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("result", "int iterations",
            asOFFSET(script_result,iterations));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("result", "bool diverged",
            asOFFSET(script_result,diverged));
    assert( r >= 0 );

    // color (pixel)
    std::cerr << "Register color (pixel)\n";
    r = engine->RegisterObjectType("color", sizeof(pixel), 
            asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<pixel>() | 
            asOBJ_APP_CLASS_ALLINTS); 
    assert( r >= 0 );
    r = engine->RegisterGlobalFunction("color white()", 
            asFUNCTION(script_white), asCALL_CDECL); 
    assert( r >= 0 );
    r = engine->RegisterGlobalFunction("color black()", 
            asFUNCTION(script_black), asCALL_CDECL); 
    assert( r >= 0 );
    r = engine->RegisterGlobalFunction("color hsv(double, double, double)", 
            asFUNCTION(script_hsv), asCALL_CDECL); 
    assert( r >= 0 );

    // other math
    r = engine->RegisterGlobalFunction("double fmod(double, double)",
            asFUNCTION(script_fmod), asCALL_CDECL);
    assert( r >= 0 );
    r = engine->RegisterGlobalFunction("double erf(double)",
            asFUNCTION(script_erf), asCALL_CDECL);
    assert( r >= 0 );

}
