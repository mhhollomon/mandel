#include "color_script_engine.hpp"

#include "pixel.hpp"

#include <iostream>
#include <cassert>
#include <cstring>
#include <string_view>
#include <map>
#include <charconv>


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

struct parse {
    std::string_view key;
    std::string_view value;
    parse(std::string_view k, std::string_view v) :
        key(k), value(v) {}
};

void* ColorScriptEngine::parse_args(std::string arg_string) {

    std::map<std::string_view, std::string_view> kvp;

    std::string_view sv(arg_string);

    char current_delim = '=';
    std::string_view current_key;
    while(true) {
        std::cerr << "current delim = '" << current_delim << "'\n";
        auto pos = sv.find_first_of(";=");

        if (pos == std::string_view::npos) {
            if (sv.size() > 0) {
                if (current_delim == ';') {
                    // we saw a key, so use what is left as the value.
                    kvp.emplace(current_key, sv);
                } else {
                    throw std::runtime_error("Invalid args specified - key with no value");
                }
            }
            break;

        } else if (sv[pos] != current_delim) {
            throw std::runtime_error("Invalid args specified - looking for " + current_delim);
        }

        if (current_delim == '=') {
            current_key = sv.substr(0, pos);
            sv.remove_prefix(pos+1);
            current_delim = ';';
        } else {
            auto value = sv.substr(0, pos);
            sv.remove_prefix(pos+1);
            kvp.emplace(current_key, value);
            current_delim = '=';
        }
    }

    for (auto const& [key, value] : kvp) {
        std::cerr << key << " => " << value << "\n";
    }

    asIScriptModule *module = get_module();
    asITypeInfo *args_class_type = module->GetTypeInfoByDecl("args");

    if (not args_class_type) {
        throw std::runtime_error("Could not find class args for setup\n");
    }

    asIScriptFunction *factory = args_class_type->GetFactoryByDecl("args @args()");
 
    // Prepare the context to call the factory function
    auto *ctx = prepare_context(factory);
    

    // Execute the call
    ctx->Execute();
    
    // Get the object that was created
    asIScriptObject *args_obj = (asIScriptObject*)ctx->GetReturnObject();
    args_obj->AddRef();

    unsigned prop_count = args_class_type->GetPropertyCount();

    for (unsigned i = 0; i < prop_count; ++i) {
        char const * prop_name = args_obj->GetPropertyName(i);
        unsigned prop_type_id = args_obj->GetPropertyTypeId(i);

        std::cerr << "checking property = " << prop_name << "\n";
        auto const iter = kvp.find(prop_name);
        if (iter != kvp.end()) {
            std::cerr << "Found property = " << prop_name << " => " << iter->second << "\n";

            if (prop_type_id == asTYPEID_INT32 ) {
                std::cerr << "its an int\n";
                int *prop_addr = (int*)args_obj->GetAddressOfProperty(i);
                int new_value = 0;
                
                auto end_ptr = iter->second.data() + iter->second.size();
                auto result = std::from_chars(iter->second.data(), 
                        end_ptr, new_value);
                if (result.ec == std::errc::invalid_argument or result.ptr != end_ptr) {
                    std::cerr << "Could not convert string '" << iter->second 
                            << "' to an integer for property " << prop_name << "\n";
                    throw std::runtime_error(
                            "Integer conversion error for property argument");
                }

                std::cerr << "Old Value = " << *prop_addr << "\n";
                std::cerr << "New value = " << new_value << "\n";
                *prop_addr = new_value;
                
            } else if (prop_type_id == asTYPEID_BOOL) {
                std::cerr << "its an bool\n";
                bool *prop_addr = (bool*)args_obj->GetAddressOfProperty(i);
                if (iter->second ==  "true" or iter->second == "1") {
                    *prop_addr = true;
                } else {
                    *prop_addr = false;
                }
            } else if (prop_type_id == asTYPEID_DOUBLE ) {
                std::cerr << "its a double\n";
                double *prop_addr = (double*)args_obj->GetAddressOfProperty(i);
                double new_value;
                
                /* apparently libstdc++ doesn't have the floating point from_char
                 * so we need to fall back on stod, sigh
                 *
                auto result = std::from_chars(iter->second.data(), 
                        iter->second.data() + iter->second.size(), new_value,
                        std::chars_format::general);
                if (result.ec == std::errc::invalid_argument) {
                    std::cerr << "Could not convert value" << iter->second 
                            << " to a double for property " << prop_name << "\n";
                    throw std::runtime_error(
                            "Double conversion error for property argument");
                }
                */

                size_t end;
                std::string foo = std::string(iter->second);
                new_value = stod(foo, &end);

                if (end != foo.size()) {
                    std::cerr << "Could not convert string '" << iter->second 
                            << "' to a double for property " << prop_name << "\n";
                    throw std::runtime_error(
                            "Double conversion error for property argument");
                }

                std::cerr << "Old Value = " << *prop_addr << "\n";
                std::cerr << "New value = " << new_value << "\n";

                *prop_addr = new_value;
                
            } else {
                std::cerr << "Some other type = " << prop_type_id << "\n";
            }
        }
    }

    return args_obj;
}


bool ColorScriptEngine::call_setup(fractal_meta_data *fp, std::string arg_string) {


    bool has_args = true;
    auto *setup_func = find_function("void setup(meta_data,args@)");

    if (not setup_func) {
        setup_func = find_function("void setup(meta_data)");
        has_args = false;
    }

    if (setup_func) {
        void *args_obj = nullptr;
        if (has_args) {
            args_obj = parse_args(arg_string);
        }

        // need to do this after the call to parse_args
        // since it needs to make a function call
        auto ctx = prepare_context(setup_func);
        ctx->SetArgObject(0, fp);
        if (has_args) {
            std::cerr << "Got here ---\n";
            if (args_obj == nullptr) {
                std::cerr << "Yipes its null!\n";
            } else {
                std::cerr << "Seems kosher\n";
            }
            ctx->SetArgAddress(1, args_obj);
        }

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
