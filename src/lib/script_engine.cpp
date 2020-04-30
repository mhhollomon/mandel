#include "script_engine.hpp"

#include <scriptstdstring/scriptstdstring.h>
#include <scriptbuilder/scriptbuilder.h>

#include <scriptmath/scriptmath.h>
#include <scriptmath/scriptmathcomplex.h>

#include <iostream>
#include <cassert>



void MessageCallback(const asSMessageInfo *msg, void *param) {
    std::string type = "ERR";

  if( msg->type == asMSGTYPE_WARNING ) 
    type = "WARN";
  else if( msg->type == asMSGTYPE_INFORMATION ) 
    type = "INFO";

  std::cerr << msg->section
      << "(" << msg->row << "," << msg->col << ")"
      << " : " << type
      << " : " << msg->message << "\n";
}


ScriptEngine::~ScriptEngine() {
    if (context_) {
        context_->Release();
    }
    if (engine_) {
        engine_->ShutDownAndRelease();
    }
}

asIScriptFunction *ScriptEngine::find_function(std::string const& func_decl) {
    if (!engine_)
        throw std::runtime_error("Script engine has not been initialized\n");

    return engine_->GetModule("Colorizer")->GetFunctionByDecl(func_decl.c_str());
}

asIScriptContext* ScriptEngine::prepare_context(std::string const& func_decl) {

    auto func = find_function(func_decl);

    if (func == nullptr) {
        throw std::runtime_error("No such function.");
    }

    return prepare_context(func);
}

asIScriptContext* ScriptEngine::prepare_context(asIScriptFunction *func) {
    if (!engine_)
        throw std::runtime_error("Script engine has not been initialized\n");

    if (!context_)
        context_ = engine_->CreateContext();

    context_->Prepare(func);

    return context_;
}


void script_log(std::string &s) {

    std::cerr << s;
}


void ScriptEngine::initialize(std::string const &script) {
    if (engine_) return;

    std::cout << "Initializing ScriptEngine\n";

    engine_ = asCreateScriptEngine();

    int r;
    
    r= engine_->SetMessageCallback(asFUNCTION(MessageCallback), 0, 
            asCALL_CDECL);
    assert(r >= 0);

    // std::string
    RegisterStdString(engine_);
    
    // double math
    RegisterScriptMath(engine_);

    // complex<double>
    RegisterScriptMathComplex(engine_);

    // logger
    r = engine_->RegisterGlobalFunction("void log(const string &in)", 
            asFUNCTION(script_log), asCALL_CDECL); 
    assert( r >= 0 );

    _register_interface(engine_);

    std::cerr << "interface registered\n";

    CScriptBuilder builder;
    r = builder.StartNewModule(engine_, "Colorizer"); 
    if( r < 0 ) {
        // If the code fails here it is usually because there
        // is no more memory to allocate the module
        std::cerr << "Unrecoverable error while starting a new module.\n";
        return;
    }
    r = builder.AddSectionFromFile(script.c_str());
    if( r < 0 ) {
        // The builder wasn't able to load the file. Maybe the file
        // has been removed, or the wrong name was given, or some
        // preprocessing commands are incorrectly written.
        std::cerr << "Please correct the errors in the script and try again.\n";
        return;
    }
    r = builder.BuildModule();
    if( r < 0 ) {
        // An error occurred. Instruct the script writer to fix the 
        // compilation errors that were listed in the output stream.
        std::cerr << "Please correct the errors in the script and try again.\n";
        return;
    }

    std::cerr << "initialization complete\n";
}
