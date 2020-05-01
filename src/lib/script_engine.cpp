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

void ScriptEngine::print_exception_info() {
    // Determine the exception that occurred
    std::cerr << "Script Exception:\n";
    std::cerr << "desc : " << context_->GetExceptionString() << "\n";

    // Determine the function where the exception occurred
    auto const *function = context_->GetExceptionFunction();
    std::cerr << "func : " << function->GetDeclaration() << "\n";
    std::cerr << "modl : " << function->GetModuleName() << "\n";
    std::cerr << "sect : " << function->GetScriptSectionName() << "\n";

    std::cerr << "line : " << context_->GetExceptionLineNumber() << "\n";
}

bool ScriptEngine::execute_context() {
    if (not context_)
        throw std::runtime_error("Context has not been created");

    int r = context_->Execute();
    std::string msg;
    switch (r) {
        case asEXECUTION_FINISHED:
            return true;
            break;
        case asEXECUTION_SUSPENDED:
            msg = "Execution Suspended";
            break;
        case asEXECUTION_ABORTED:
            msg = "Execution Aborted";
            break;
        case asEXECUTION_EXCEPTION :
            msg = "Execution was terminated with exception";
            print_exception_info();
            break;
        case asEXECUTION_PREPARED :
            msg = "Context ready for new execution";
            break;
        case asEXECUTION_UNINITIALIZED :
            msg = "Context is not initialized";
            break;
        case asEXECUTION_ACTIVE :
            msg = "Context is currently executing a function call";
            break;
        case asEXECUTION_ERROR :
            msg = "Context is in error state";
            break;
        default :
            msg = "Unknown Error";
            break;
    }

    std::cerr << "Execution failed : " << msg << "\n";

    return false;
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
    r = engine_->RegisterGlobalFunction("void logger(const string &in)", 
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
