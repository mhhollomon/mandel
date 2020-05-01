#if !defined(_MANDEL_SCRIPT_ENGINE_HPP_)
#define _MANDEL_SCRIPT_ENGINE_HPP_

// tell angelscript to use doubles
#define AS_USE_FLOAT 0
#include <angelscript.h>

#include <string>


class ScriptEngine {
    asIScriptEngine *engine_ = nullptr;
    asIScriptContext *context_ = nullptr;

  public:
    ScriptEngine() noexcept = default;

    ~ScriptEngine();
    void initialize(std::string const &script);

    asIScriptFunction *find_function(std::string const& func_decl);

    asIScriptContext* prepare_context(std::string const& func_decl);
    asIScriptContext* prepare_context(asIScriptFunction *func);
    void print_exception_info();

  protected:
    bool execute_context();

  private:
    virtual void _register_interface(asIScriptEngine* engine) {} 



};

#endif
