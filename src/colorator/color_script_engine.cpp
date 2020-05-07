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

    prepass_func_ = find_function("void prepass(point_data@)");
    checked_prepass = true;
    if (prepass_func_)
        prepass_func_->AddRef();

    return prepass_func_ != nullptr;
}

void ColorScriptEngine::call_precolor() {


    auto *precolor_func = find_function("void precolor()");

    if (precolor_func) {
        prepare_context(precolor_func);
        if ( not execute_context() ) {
            throw std::runtime_error("Call to precolor() failed");
        }
    }
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

struct script_point_data : public fractal_point_data {
    int refcnt = 1;

    script_point_data() = default;
    script_point_data( fractal_point_data const & cr) : fractal_point_data(cr)
    {}

    static script_point_data *Create() {
        return new script_point_data();
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

double script_log2(double a) {
    return std::log2(a);
}


bool ColorScriptEngine::call_setup(fractal_meta_data *fp) {


    auto *setup_func = find_function("void setup(meta_data)");

    if (setup_func) {
        auto ctx = prepare_context("void setup(meta_data)");
        ctx->SetArgObject(0, fp);

        return execute_context();
    } else {
        return true;
    }
}

pixel ColorScriptEngine::call_colorize(fractal_point_data &pd) {
    if (not color_func_) {
        color_func_ = find_function("color colorize(point_data@)");
        if (color_func_) {
            // we are storing so be sure to take a reference
            color_func_->AddRef();
        } else {
            throw std::runtime_error("Could not find colorize() function");
        }
    }

    script_point_data *r = new script_point_data(pd);

    auto ctx = prepare_context(color_func_);
    ctx->SetArgObject(0, r);

    if (execute_context()) {
        r->Release();
        return *(pixel *)ctx->GetReturnObject();
    } else {
        throw std::runtime_error("colorize call failed");
    }
}

void ColorScriptEngine::call_prepass(fractal_point_data &pd) {
    if (not has_prepass()) return;

    script_point_data *r = new script_point_data(pd);
    auto ctx = prepare_context(prepass_func_);
    ctx->SetArgObject(0, r);
    if ( execute_context() ) {
        r->Release();
    } else {
        throw std::runtime_error("Call to prepass() failed");
    }
}

void ColorScriptEngine::_register_interface(asIScriptEngine * engine) {
    int r;
    
    // meta_data
    r = engine->RegisterObjectType("meta_data", sizeof(fractal_meta_data), 
            asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<fractal_meta_data>()); 
    assert( r >= 0 );
    /*
    r = engine->RegisterObjectBehaviour("meta_data", asBEHAVE_CONSTRUCT,
            "void f()", asFUNCTION(meta_data_new), 
            asCALL_CDECL_OBJLAST); 
    assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("meta_data", asBEHAVE_DESTRUCT, 
            "void f()", asFUNCTION(meta_data_del), 
            asCALL_CDECL_OBJLAST); 
    assert( r >= 0 );
    */

    r = engine->RegisterObjectProperty("meta_data", "complex bb_tl",
            asOFFSET(fractal_meta_data,bb_top_left));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("meta_data", "complex bb_br",
            asOFFSET(fractal_meta_data,bb_bottom_right));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("meta_data", "double escape_radius",
            asOFFSET(fractal_meta_data,escape_radius));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("meta_data", "int limit",
            asOFFSET(fractal_meta_data,limit));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("meta_data", "int samples_real",
            asOFFSET(fractal_meta_data,samples_real));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("meta_data", "int samples_img",
            asOFFSET(fractal_meta_data,samples_img));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("meta_data", "int max_iterations",
            asOFFSET(fractal_meta_data,max_iterations));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("meta_data", "int min_iterations",
            asOFFSET(fractal_meta_data,min_iterations));
    assert( r >= 0 );

    // point_data
    r = engine->RegisterObjectType("point_data", 0, asOBJ_REF); 
    assert( r >= 0 );

    // TODO: Maybe don't let script create?
    r = engine->RegisterObjectBehaviour("point_data", asBEHAVE_FACTORY, 
            "point_data@ f()", asFUNCTION(script_point_data::Create), asCALL_CDECL); 
    assert( r >= 0 );

    r = engine->RegisterObjectBehaviour("point_data", asBEHAVE_ADDREF, 
            "void f()", asMETHOD(script_point_data,AddRef), asCALL_THISCALL); 
    assert( r >= 0 );

    r = engine->RegisterObjectBehaviour("point_data", asBEHAVE_RELEASE, 
            "void f()", asMETHOD(script_point_data,Release), asCALL_THISCALL); 
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("point_data", "complex last_value",
            asOFFSET(script_point_data,last_value));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("point_data", "double last_modulus",
            asOFFSET(script_point_data,last_modulus));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("point_data", "int iterations",
            asOFFSET(script_point_data,iterations));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("point_data", "bool diverged",
            asOFFSET(script_point_data,diverged));
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
    r = engine->RegisterGlobalFunction("double log2(double)",
            asFUNCTION(script_log2), asCALL_CDECL);
    assert( r >= 0 );

}
