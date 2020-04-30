#include "color_script_engine.hpp"

#include "compute.hpp"
#include "pixel.hpp"

#include <iostream>
#include <cassert>



void fractal_params_new(void *memory) {
  // Initialize the pre-allocated memory by calling the
  // object constructor with the placement-new operator
  new(memory) fractal_params();
}
 
void fractal_params_del(void *memory) {
  // Uninitialize the memory by calling the object destructor
  ((fractal_params*)memory)->~fractal_params();
}

struct script_result : check_results {
    int refcnt = 1;

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


bool ColorScriptEngine::call_setup(fractal_params *fp) {

    auto ctx = prepare_context("void setup(fractal_params)");
    ctx->SetArgObject(0, fp);
    int r = ctx->Execute();
    if( r != asEXECUTION_FINISHED ) {
        std::cerr << "Why didn't that work?\n";
        return false;
    }

    return true;
}

pixel ColorScriptEngine::call_colorize(check_results &result) {
    if (not color_func_) {
        color_func_ = find_function("color colorize(result@)");
        if (not color_func_) {
            throw std::runtime_error("Could not find colorize() function");
        }
    }

    auto ctx = prepare_context(color_func_);
    ctx->SetArgObject(0, &result);
    int r = ctx->Execute();
    if( r != asEXECUTION_FINISHED ) {
        throw std::runtime_error("Execution of colorize() did not finish");
    }

    return *(pixel *)ctx->GetReturnObject();
}

void ColorScriptEngine::_register_interface(asIScriptEngine * engine) {
    int r;
    
    // fractal_params
    r = engine->RegisterObjectType("fractal_params", sizeof(fractal_params), 
            asOBJ_VALUE | asGetTypeTraits<fractal_params>()); 
    assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("fractal_params", asBEHAVE_CONSTRUCT,
            "void f()", asFUNCTION(fractal_params_new), 
            asCALL_CDECL_OBJLAST); 
    assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("fractal_params", asBEHAVE_DESTRUCT, 
            "void f()", asFUNCTION(fractal_params_del), 
            asCALL_CDECL_OBJLAST); 
    assert( r >= 0 );

    r = engine->RegisterObjectProperty("fractal_params", "complex bb_tl",
            asOFFSET(fractal_params,bb_top_left));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("fractal_params", "complex bb_br",
            asOFFSET(fractal_params,bb_bottom_right));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("fractal_params", "int limit",
            asOFFSET(fractal_params,limit));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("fractal_params", "int samples_real",
            asOFFSET(fractal_params,samples_real));
    assert( r >= 0 );
    r = engine->RegisterObjectProperty("fractal_params", "int samples_img",
            asOFFSET(fractal_params,samples_img));
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
            asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<pixel>() | asOBJ_APP_CLASS_ALLINTS); 
    assert( r >= 0 );
    r = engine->RegisterGlobalFunction("color white()", 
            asFUNCTION(script_white), asCALL_CDECL); 
    assert( r >= 0 );
    r = engine->RegisterGlobalFunction("color black()", 
            asFUNCTION(script_black), asCALL_CDECL); 
    assert( r >= 0 );

}
